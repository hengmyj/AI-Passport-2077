#include "profile_store.h"
#include "esp_partition.h"
#include <string.h>
#include <stdatomic.h>

static const esp_partition_t *partition,*extra_partition;
static const uint8_t *mapped,*extra_mapped;
static esp_partition_mmap_handle_t mapping,extra_mapping;
static atomic_uint active_badge;
static unsigned badge_count=1;
static int banks[PROFILE_BADGE_COUNT];
static atomic_int current_slot=-1;
static atomic_uint revision,displayed;
static int pending=-1;
static unsigned pending_badge;
static profile_header_t next;
static size_t received;
static uint32_t crc;
static const uint8_t *bank_data(int bank) {return bank<2?mapped+bank*PROFILE_SLOT_SIZE:extra_mapped+(bank-2)*PROFILE_SLOT_SIZE;}
static const esp_partition_t *bank_partition(int bank) {return bank<2?partition:extra_partition;}
static size_t bank_offset(int bank) {return (bank<2?bank:bank-2)*PROFILE_SLOT_SIZE;}
static const profile_header_t *header(int slot) {return (const profile_header_t *)bank_data(slot);}
static bool bank_valid(int slot) {
    const profile_header_t *h=header(slot);
    if(!profile_header_valid(h)) return false;
    uint32_t c=profile_crc32(UINT32_MAX,(const uint8_t *)h+sizeof(*h),h->json_length);
    c=profile_crc32(c,bank_data(slot)+PROFILE_DATA_OFFSET,h->pixel_length);
    return (c^UINT32_MAX)==h->payload_crc;
}
esp_err_t profile_store_init(void) {
    /* A different firmware's partition table or an mmap failure must never
     * leave zero-initialized bank indices pointing at a null flash mapping. */
    for(unsigned i=0;i<PROFILE_BADGE_COUNT;i++)banks[i]=-1;
    mapped=extra_mapped=NULL;badge_count=1;pending=-1;
    atomic_store(&active_badge,0);atomic_store(&current_slot,-1);
    atomic_store(&revision,0);atomic_store(&displayed,0);
    partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY,"badge_user");
    if(!partition || partition->size!=2*PROFILE_SLOT_SIZE) return ESP_ERR_NOT_FOUND;
    esp_err_t e=esp_partition_mmap(partition,0,partition->size,ESP_PARTITION_MMAP_DATA,(const void **)&mapped,&mapping);
    if(e!=ESP_OK) return e;
    badge_count=1;extra_mapped=NULL;pending=-1;
    extra_partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_ANY,"badge_slots");
    if(extra_partition && extra_partition->size==(PROFILE_BADGE_COUNT-1)*2*PROFILE_SLOT_SIZE &&
       esp_partition_mmap(extra_partition,0,extra_partition->size,ESP_PARTITION_MMAP_DATA,(const void **)&extra_mapped,&extra_mapping)==ESP_OK)badge_count=PROFILE_BADGE_COUNT;
    unsigned latest=0;
    for(unsigned i=0;i<PROFILE_BADGE_COUNT;i++) {
        banks[i]=-1;if(i>=badge_count)continue;
        int a=i*2,b=a+1;bool va=bank_valid(a),vb=bank_valid(b);
        banks[i]=vb&&(!va||profile_sequence_newer(header(b)->sequence,header(a)->sequence))?b:va?a:-1;
        if(banks[i]>=0&&profile_sequence_newer(header(banks[i])->sequence,latest))latest=header(banks[i])->sequence;
    }
    atomic_store(&active_badge,0);atomic_store(&current_slot,banks[0]);
    atomic_store(&revision,latest);atomic_store(&displayed,latest);
    return ESP_OK;
}
unsigned profile_store_revision(void) {return atomic_load(&revision);}
void profile_store_displayed(unsigned r) {atomic_store(&displayed,r);}
unsigned profile_store_revision_for(unsigned id) {return id<badge_count&&banks[id]>=0?header(banks[id])->sequence:0;}
unsigned profile_store_count(void) {return badge_count;}
unsigned profile_store_badge(void) {return atomic_load(&active_badge);}
bool profile_store_ready(void) {return pending<0&&atomic_load(&revision)==atomic_load(&displayed);}
bool profile_store_refresh_pending(void) {return atomic_load(&revision)!=atomic_load(&displayed);}
esp_err_t profile_store_select(unsigned id) {
    if(id>=badge_count)return ESP_ERR_INVALID_ARG;
    if(!profile_store_ready())return ESP_ERR_INVALID_STATE;
    if(id==profile_store_badge())return ESP_OK;
    atomic_store(&active_badge,id);atomic_store(&current_slot,banks[id]);atomic_fetch_add(&revision,1);return ESP_OK;
}
const uint8_t *profile_store_card_for(unsigned id) {
    int slot=id<badge_count?banks[id]:-1;
    return slot>=0&&header(slot)->flags?bank_data(slot)+PROFILE_DATA_OFFSET:NULL;
}
const char *profile_store_json_for(unsigned id,size_t *size) {
    int slot=id<badge_count?banks[id]:-1;
    if(slot<0) {*size=2;return "{}";}
    *size=header(slot)->json_length;return (const char *)header(slot)+sizeof(profile_header_t);
}
unsigned profile_store_version_for(unsigned id) {
    int slot=id<badge_count?banks[id]:-1;return slot<0?0:header(slot)->version;
}
const uint8_t *profile_store_asset_for(unsigned id,profile_asset_t asset) {
    int slot=id<badge_count?banks[id]:-1;
    const uint8_t *p=profile_store_card_for(id);
    if(!p || (unsigned)asset>PROFILE_ASSET_QR) return NULL;
    if(asset>=PROFILE_ASSET_BRAND && header(slot)->version<2) return NULL;
    if(asset==PROFILE_ASSET_QR && !(header(slot)->flags&2)) return NULL;
    size_t offset=0;
    for(unsigned a=0;a<(unsigned)asset;a++)offset+=profile_asset_size(header(slot)->version,a);
    return p+offset;
}
esp_err_t profile_store_read_for(unsigned id,profile_asset_t asset,size_t offset,void *data,size_t size) {
    if((unsigned)asset>PROFILE_ASSET_QR) return ESP_ERR_INVALID_ARG;
    const uint8_t *p=profile_store_asset_for(id,asset);size_t bound=profile_asset_size(profile_store_version_for(id),asset);
    if(!p || offset>bound || size>bound-offset) return ESP_ERR_INVALID_ARG;
    memcpy(data,p+offset,size);return ESP_OK;
}
static esp_err_t begin_for(unsigned id,const char *json,size_t size) {
    if(id>=badge_count || !mapped || size<2 || size>PROFILE_JSON_MAX) return ESP_ERR_INVALID_ARG;
    if(atomic_load(&revision)!=atomic_load(&displayed)) return ESP_ERR_INVALID_STATE;
    pending=-1;
    int active=banks[id];
    int target=active==(int)(id*2)?id*2+1:id*2;
    pending_badge=id;
    esp_err_t e=esp_partition_erase_range(bank_partition(target),bank_offset(target),PROFILE_SLOT_SIZE);
    if(e==ESP_OK) e=esp_partition_write(bank_partition(target),bank_offset(target)+sizeof(profile_header_t),json,size);
    if(e!=ESP_OK) return e;
    next=(profile_header_t){.magic=PROFILE_MAGIC,.version=1,.sequence=active>=0?header(active)->sequence+1:1,
        .json_length=size,.pixel_length=PROFILE_PIXELS_BYTES,.flags=1};
    crc=profile_crc32(UINT32_MAX,json,size);received=0;pending=target;return ESP_OK;
}
esp_err_t profile_store_append(size_t offset,const void *data,size_t size) {
    if(pending<0 || offset!=received || !size || received>next.pixel_length || size>next.pixel_length-received) return ESP_ERR_INVALID_ARG;
    esp_err_t e=esp_partition_write(bank_partition(pending),bank_offset(pending)+PROFILE_DATA_OFFSET+received,data,size);
    if(e==ESP_OK) {crc=profile_crc32(crc,data,size);received+=size;}
    else pending=-1;
    return e;
}
esp_err_t profile_store_commit(void) {
    if(pending<0 || received!=next.pixel_length) return ESP_ERR_INVALID_SIZE;
    next.payload_crc=crc^UINT32_MAX;
    next.header_crc=profile_crc32(UINT32_MAX,&next,offsetof(profile_header_t,header_crc))^UINT32_MAX;
    /* Header is the commit marker: interrupted writes preserve the prior bank. */
    esp_err_t e=esp_partition_write(bank_partition(pending),bank_offset(pending),&next,sizeof(next));
    if(e==ESP_OK && !bank_valid(pending)) e=ESP_ERR_INVALID_CRC;
    if(e==ESP_OK) {banks[pending_badge]=pending;if(pending_badge==profile_store_badge())atomic_store(&current_slot,pending);atomic_fetch_add(&revision,1);}
    pending=-1;return e;
}
esp_err_t profile_store_clear_for(unsigned id) {
    esp_err_t e=begin_for(id,"{}",2);
    if(e!=ESP_OK) return e;
    next.flags=0;next.pixel_length=0;return profile_store_commit();
}
void profile_store_abort(void) {pending=-1;}

esp_err_t profile_store_begin_for(unsigned id,const char *json,size_t size,bool v2,bool qr) {
    return profile_store_begin_format(id,json,size,v2?2:1,qr);
}
esp_err_t profile_store_begin_format(unsigned id,const char *json,size_t size,unsigned format,bool qr) {
    if(format<1||format>4)return ESP_ERR_INVALID_ARG;
    esp_err_t e=begin_for(id,json,size);
    if(e==ESP_OK&&format>=2) {next.version=format;next.flags=qr?3:1;next.pixel_length=format==4?PROFILE_V4_BYTES:format==3?PROFILE_V3_BYTES:PROFILE_V2_BYTES;}
    return e;
}
const uint8_t *profile_store_card(void) {return profile_store_card_for(profile_store_badge());}
const char *profile_store_json(size_t *size) {return profile_store_json_for(profile_store_badge(),size);}
unsigned profile_store_version(void) {return profile_store_version_for(profile_store_badge());}
const uint8_t *profile_store_asset(profile_asset_t asset) {return profile_store_asset_for(profile_store_badge(),asset);}
esp_err_t profile_store_read(profile_asset_t asset,size_t offset,void *data,size_t size) {return profile_store_read_for(profile_store_badge(),asset,offset,data,size);}
esp_err_t profile_store_begin(const char *json,size_t size) {return begin_for(profile_store_badge(),json,size);}
esp_err_t profile_store_begin_v2(const char *json,size_t size,bool qr) {return profile_store_begin_for(profile_store_badge(),json,size,true,qr);}
esp_err_t profile_store_clear(void) {return profile_store_clear_for(profile_store_badge());}
esp_err_t profile_store_patch(unsigned id,const char *json,size_t size,void (*patch)(size_t,uint8_t *,size_t,void *),void *context){
    if(id>=badge_count||!profile_store_ready()||!profile_store_card_for(id))return ESP_ERR_INVALID_STATE;
    int source=banks[id];const uint8_t *pixels=bank_data(source)+PROFILE_DATA_OFFSET;
    unsigned version=header(source)->version;size_t total=header(source)->pixel_length;
    esp_err_t e=profile_store_begin_format(id,json,size,version,(header(source)->flags&2)!=0);
    uint8_t chunk[512];
    for(size_t offset=0;e==ESP_OK&&offset<total;){size_t n=total-offset;if(n>sizeof(chunk))n=sizeof(chunk);memcpy(chunk,pixels+offset,n);
        if(patch)patch(offset,chunk,n,context);
        e=profile_store_append(offset,chunk,n);offset+=n;
    }
    if(e==ESP_OK)e=profile_store_commit();else profile_store_abort();return e;
}
