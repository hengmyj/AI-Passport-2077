#include "profile_store.h"
#include "esp_partition.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static unsigned char flash[2*PROFILE_SLOT_SIZE];
static esp_partition_t part={sizeof(flash)};
static bool fail_header,extras_available,missing_partition,fail_mapping;
static unsigned char extra_flash[(PROFILE_BADGE_COUNT-1)*2*PROFILE_SLOT_SIZE];
static esp_partition_t extra_part={sizeof(extra_flash)};
static unsigned char *storage(const esp_partition_t *p){return p==&part?flash:extra_flash;}
const esp_partition_t *esp_partition_find_first(int t,int st,const char *name) {(void)t;(void)st;if(missing_partition)return NULL;return !strcmp(name,"badge_user")?&part:extras_available&&!strcmp(name,"badge_slots")?&extra_part:NULL;}
esp_err_t esp_partition_mmap(const esp_partition_t *p,size_t off,size_t size,int type,const void **out,esp_partition_mmap_handle_t *handle){(void)type;if(fail_mapping)return ESP_FAIL;assert(off+size<=p->size);*out=storage(p)+off;*handle=1;return ESP_OK;}
esp_err_t esp_partition_erase_range(const esp_partition_t *p,size_t off,size_t size){assert(off+size<=p->size);assert(!(off%4096)&&!(size%4096));memset(storage(p)+off,255,size);return ESP_OK;}
esp_err_t esp_partition_write(const esp_partition_t *p,size_t off,const void *data,size_t size){unsigned char *flash=storage(p);assert(off+size<=p->size);size_t actual=fail_header&&size==sizeof(profile_header_t)?12:size;const unsigned char *src=data;for(size_t i=0;i<actual;i++){assert((flash[off+i]&src[i])==src[i]);flash[off+i]&=src[i];}return actual==size?ESP_OK:ESP_FAIL;}
static void upload_bytes(unsigned char value,size_t total){unsigned char data[768];memset(data,value,sizeof(data));for(size_t pos=0;pos<total;){size_t count=total-pos;if(count>sizeof(data))count=sizeof(data);assert(profile_store_append(pos,data,count)==ESP_OK);pos+=count;}}
static void patch_first(size_t offset,uint8_t *p,size_t n,void *ctx){(void)n;(void)ctx;if(offset==0)p[0]=0xab;}
int main(void){
    missing_partition=true;assert(profile_store_init()==ESP_ERR_NOT_FOUND);assert(!profile_store_card_for(0));size_t empty;assert(!strcmp(profile_store_json_for(0,&empty),"{}")&&empty==2);assert(profile_store_version_for(0)==0);assert(profile_store_revision_for(0)==0);
    missing_partition=false;fail_mapping=true;assert(profile_store_init()==ESP_FAIL);assert(!profile_store_card());fail_mapping=false;
    assert((profile_crc32(UINT32_MAX,"123456789",9)^UINT32_MAX)==0xcbf43926u);
    memset(flash,255,sizeof(flash));assert(profile_store_init()==ESP_OK);assert(!profile_store_card());
    assert(profile_store_begin("{}",2)==ESP_OK);assert(profile_store_commit()==ESP_ERR_INVALID_SIZE);
    upload_bytes(0x12,PROFILE_PIXELS_BYTES);assert(profile_store_commit()==ESP_OK);assert(profile_store_card()[0]==0x12);
    assert(profile_store_refresh_pending()&&!profile_store_ready());
    assert(profile_store_begin("{}",2)==ESP_ERR_INVALID_STATE);profile_store_displayed(profile_store_revision());
    assert(!profile_store_refresh_pending()&&profile_store_ready());
    assert(profile_store_begin("{}",2)==ESP_OK);unsigned char b=7;assert(profile_store_append(1,&b,1)==ESP_ERR_INVALID_ARG);
    assert(profile_store_append(0,&b,1)==ESP_OK);profile_store_abort();assert(profile_store_init()==ESP_OK);assert(profile_store_card()[0]==0x12);
    assert(profile_store_begin("{}",2)==ESP_OK);upload_bytes(0x34,PROFILE_PIXELS_BYTES);fail_header=true;assert(profile_store_commit()==ESP_FAIL);fail_header=false;
    assert(profile_store_init()==ESP_OK);assert(profile_store_card()[0]==0x12);
    assert(profile_store_begin("{}",2)==ESP_OK);upload_bytes(0x56,PROFILE_PIXELS_BYTES);assert(profile_store_commit()==ESP_OK);
    assert(profile_store_init()==ESP_OK);assert(profile_store_card()[PROFILE_CARD_BYTES-1]==0x56);
    unsigned char read[2];assert(profile_store_read(true,PROFILE_AVATAR_BYTES-1,read,2)==ESP_ERR_INVALID_ARG);
    assert(profile_store_read(true,0,read,2)==ESP_OK&&read[0]==0x56);
    assert(profile_store_version()==1);assert(!profile_store_asset(PROFILE_ASSET_QR));
    assert(profile_store_begin_v2("{}",2,true)==ESP_OK);
    upload_bytes(0x99,PROFILE_V2_BYTES-1);assert(profile_store_commit()==ESP_ERR_INVALID_SIZE);
    assert(profile_store_init()==ESP_OK);assert(profile_store_version()==1);
    assert(profile_store_card()[0]==0x56);
    assert(profile_store_begin_v2("{}",2,true)==ESP_OK);upload_bytes(0x77,PROFILE_V2_BYTES);
    assert(profile_store_append(PROFILE_V2_BYTES,&b,1)==ESP_ERR_INVALID_ARG);
    assert(profile_store_commit()==ESP_OK);assert(profile_store_init()==ESP_OK);
    assert(profile_store_version()==2&&profile_store_asset(PROFILE_ASSET_BRAND)[0]==0x77);
    assert(profile_store_asset(PROFILE_ASSET_LOGO)[PROFILE_LOGO_BYTES-1]==0x77);
    assert(profile_store_read(PROFILE_ASSET_QR,PROFILE_QR_BYTES-2,read,2)==ESP_OK&&read[1]==0x77);
    assert(profile_store_read(PROFILE_ASSET_QR,PROFILE_QR_BYTES-1,read,2)==ESP_ERR_INVALID_ARG);
    assert(profile_store_begin_v2("{}",2,false)==ESP_OK);upload_bytes(0x88,PROFILE_V2_BYTES);
    assert(profile_store_commit()==ESP_OK);assert(profile_store_init()==ESP_OK);
    assert(!profile_store_asset(PROFILE_ASSET_QR));
    assert(profile_store_clear()==ESP_OK);assert(profile_store_init()==ESP_OK);assert(!profile_store_card());
    assert(profile_sequence_newer(1,UINT32_MAX));assert(!profile_sequence_newer(UINT32_MAX,1));
    extras_available=true;memset(extra_flash,255,sizeof(extra_flash));
    assert(profile_store_init()==ESP_OK&&profile_store_count()==5);
    for(unsigned id=0;id<5;id++) {
        profile_store_displayed(profile_store_revision());
        assert(profile_store_begin_for(id,"{}",2,true,id%2)==ESP_OK);
        upload_bytes(0x20+id,PROFILE_V2_BYTES);assert(profile_store_commit()==ESP_OK);
    }
    assert(profile_store_card()[0]==0x20);
    profile_store_displayed(profile_store_revision());assert(profile_store_select(4)==ESP_OK);
    assert(profile_store_card()[0]==0x24);assert(profile_store_select(2)==ESP_ERR_INVALID_STATE);
    profile_store_displayed(profile_store_revision());
    assert(profile_store_begin_for(1,"{}",2,true,true)==ESP_OK);
    assert(profile_store_select(0)==ESP_ERR_INVALID_STATE);profile_store_abort();
    assert(profile_store_select(0)==ESP_OK);profile_store_displayed(profile_store_revision());
    assert(profile_store_begin_for(2,"{}",2,true,false)==ESP_OK);upload_bytes(0xaa,PROFILE_V2_BYTES);
    fail_header=true;assert(profile_store_commit()==ESP_FAIL);fail_header=false;
    assert(profile_store_init()==ESP_OK);
    for(unsigned id=0;id<5;id++) {
        assert(profile_store_card_for(id)[0]==0x20+id);
        assert(profile_store_asset_for(id,PROFILE_ASSET_LOGO)[PROFILE_LOGO_BYTES-1]==0x20+id);
        assert((profile_store_asset_for(id,PROFILE_ASSET_QR)!=NULL)==(id%2!=0));
    }
    assert(profile_store_clear_for(2)==ESP_OK);profile_store_displayed(profile_store_revision());
    assert(!profile_store_card_for(2)&&profile_store_card_for(1)[0]==0x21);
    assert(profile_store_select(5)==ESP_ERR_INVALID_ARG);
    /* Upgrade one existing V2 card; the other slots and interrupted writes
     * must remain valid. Exercise each new asset's final byte and bounds. */
    profile_store_displayed(profile_store_revision());
    assert(profile_store_begin_format(0,"{}",2,5,true)==ESP_ERR_INVALID_ARG);
    assert(profile_store_begin_format(0,"{}",2,3,true)==ESP_OK);
    upload_bytes(0x35,PROFILE_V3_BYTES-1);assert(profile_store_commit()==ESP_ERR_INVALID_SIZE);
    assert(profile_store_init()==ESP_OK&&profile_store_version_for(0)==2);
    assert(profile_store_begin_format(0,"{}",2,3,true)==ESP_OK);
    upload_bytes(0x35,PROFILE_V3_BYTES);assert(profile_store_commit()==ESP_OK);
    assert(profile_store_init()==ESP_OK&&profile_store_version_for(0)==3);
    size_t offset=0;
    for(unsigned asset=0;asset<=PROFILE_ASSET_QR;asset++) {
        size_t size=profile_asset_size(3,asset);
        assert(profile_store_asset_for(0,asset)==profile_store_card_for(0)+offset);
        assert(profile_store_read_for(0,asset,size-2,read,2)==ESP_OK&&read[0]==0x35);
        assert(profile_store_read_for(0,asset,size-1,read,2)==ESP_ERR_INVALID_ARG);
        offset+=size;
    }
    assert(offset==PROFILE_V3_BYTES&&offset+PROFILE_DATA_OFFSET<=PROFILE_SLOT_SIZE);
    assert(profile_store_version_for(1)==2&&profile_store_card_for(1)[0]==0x21);
    assert(profile_store_begin_format(0,"{}",2,3,false)==ESP_OK);
    upload_bytes(0x36,PROFILE_V3_BYTES);fail_header=true;assert(profile_store_commit()==ESP_FAIL);fail_header=false;
    assert(profile_store_init()==ESP_OK&&profile_store_card_for(0)[0]==0x35);
    puts("Portrait V3: mixed versions, upgrade, asset offsets/bounds, interrupted write/commit PASS");
    assert(profile_store_begin_format(4,"{}",2,4,true)==ESP_OK);
    upload_bytes(0x44,PROFILE_V4_BYTES-1);assert(profile_store_commit()==ESP_ERR_INVALID_SIZE);
    assert(profile_store_init()==ESP_OK&&profile_store_version_for(4)==2);
    assert(profile_store_begin_format(4,"{}",2,4,true)==ESP_OK);
    upload_bytes(0x44,PROFILE_V4_BYTES);assert(profile_store_commit()==ESP_OK);
    assert(profile_store_init()==ESP_OK&&profile_store_version_for(4)==4);
    offset=0;
    for(unsigned asset=0;asset<=PROFILE_ASSET_QR;asset++) {
        size_t size=profile_asset_size(4,asset);
        assert(profile_store_asset_for(4,asset)==profile_store_card_for(4)+offset);
        assert(profile_store_read_for(4,asset,size-2,read,2)==ESP_OK&&read[0]==0x44);
        assert(profile_store_read_for(4,asset,size-1,read,2)==ESP_ERR_INVALID_ARG);
        offset+=size;
    }
    assert(offset==PROFILE_V4_BYTES&&offset+PROFILE_DATA_OFFSET<=PROFILE_SLOT_SIZE);
    assert(profile_store_version_for(0)==3&&profile_store_card_for(0)[0]==0x35);
    assert(profile_store_version_for(1)==2&&profile_store_card_for(1)[0]==0x21);
    assert(profile_store_begin_format(4,"{}",2,4,false)==ESP_OK);
    upload_bytes(0x45,PROFILE_V4_BYTES);fail_header=true;assert(profile_store_commit()==ESP_FAIL);fail_header=false;
    assert(profile_store_init()==ESP_OK&&profile_store_card_for(4)[0]==0x44);
    profile_store_displayed(profile_store_revision());
    assert(profile_store_patch(4,"{\"title\":\"test\"}",16,patch_first,NULL)==ESP_OK);
    assert(profile_store_card_for(4)[0]==0xab&&profile_store_card_for(4)[1]==0x44);
    assert(profile_store_asset_for(4,PROFILE_ASSET_AVATAR)[0]==0x44&&profile_store_asset_for(4,PROFILE_ASSET_QR)[0]==0x44);
    assert(profile_store_init()==ESP_OK&&profile_store_card_for(4)[0]==0xab);
    profile_store_displayed(profile_store_revision());fail_header=true;
    assert(profile_store_patch(4,"{}",2,NULL,NULL)!=ESP_OK);fail_header=false;
    assert(profile_store_init()==ESP_OK&&profile_store_card_for(4)[0]==0xab);
    assert(profile_store_card_for(1)[0]==0x21);
    puts("Tactical V4: mixed V2/V3/V4, atlas bounds and interrupted writes PASS");
    puts("Five profiles: isolation, switching barrier, interrupted inactive write, clear and reload PASS");
    puts("Profile persistence: PASS (CRC, bounds, interrupted upload/commit, reload, reset)");
    return 0;
}
