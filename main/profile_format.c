#include "profile_format.h"
uint32_t profile_crc32(uint32_t state,const void *data,size_t size) {
    const unsigned char *p=data;
    for(size_t i=0;i<size;i++) {
        state^=p[i];
        for(int j=0;j<8;j++) state=(state>>1)^((0u-(state&1u))&0xEDB88320u);
    }
    return state;
}
bool profile_header_valid(const profile_header_t *h) {
    return h->magic==PROFILE_MAGIC && (h->version>=1 && h->version<=4) && h->json_length>=2 &&
        h->json_length<=PROFILE_JSON_MAX && h->flags<=(h->version==1?1u:3u) && (h->flags!=2) &&
        h->pixel_length==(h->flags?(h->version==1?PROFILE_PIXELS_BYTES:h->version==2?PROFILE_V2_BYTES:h->version==3?PROFILE_V3_BYTES:PROFILE_V4_BYTES):0) &&
        (profile_crc32(UINT32_MAX,h,offsetof(profile_header_t,header_crc))^UINT32_MAX)==h->header_crc;
}
bool profile_sequence_newer(uint32_t a,uint32_t b) {return (int32_t)(a-b)>0;}

size_t profile_asset_size(unsigned version,unsigned asset) {
    const size_t legacy[]={PROFILE_CARD_BYTES,PROFILE_AVATAR_BYTES,PROFILE_BRAND_BYTES,PROFILE_LOGO_BYTES,PROFILE_QR_BYTES};
    const size_t portrait[]={PROFILE_V3_CARD_BYTES,PROFILE_V3_AVATAR_BYTES,PROFILE_V3_BRAND_BYTES,PROFILE_LOGO_BYTES,PROFILE_QR_BYTES};
    const size_t tactical[]={PROFILE_V4_CARD_BYTES,PROFILE_V4_AVATAR_BYTES,PROFILE_V3_BRAND_BYTES,PROFILE_LOGO_BYTES,PROFILE_QR_BYTES};
    return asset<=PROFILE_ASSET_QR?(version>=4?tactical[asset]:version>=3?portrait[asset]:legacy[asset]):0;
}
