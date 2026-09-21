#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define PROFILE_BADGE_COUNT 5u
#define PROFILE_MAGIC 0x32504449u
#define PROFILE_SLOT_SIZE 0x18000u
#define PROFILE_DATA_OFFSET 0x1000u
#define PROFILE_CARD_W 208u
#define PROFILE_CARD_H 148u
#define PROFILE_AVATAR_W 72u
#define PROFILE_AVATAR_H 88u
#define PROFILE_CARD_BYTES (PROFILE_CARD_W*PROFILE_CARD_H*2u)
#define PROFILE_AVATAR_BYTES (PROFILE_AVATAR_W*PROFILE_AVATAR_H*2u)
#define PROFILE_PIXELS_BYTES (PROFILE_CARD_BYTES+PROFILE_AVATAR_BYTES)
#define PROFILE_BRAND_W 148u
#define PROFILE_BRAND_H 44u
#define PROFILE_LOGO_W 32u
#define PROFILE_QR_W 192u
#define PROFILE_BRAND_BYTES (PROFILE_BRAND_W*PROFILE_BRAND_H*2u)
#define PROFILE_LOGO_BYTES (PROFILE_LOGO_W*PROFILE_LOGO_W*2u)
#define PROFILE_QR_BYTES (PROFILE_QR_W*PROFILE_QR_W/8u)
#define PROFILE_V2_BYTES (PROFILE_PIXELS_BYTES+PROFILE_BRAND_BYTES+PROFILE_LOGO_BYTES+PROFILE_QR_BYTES)
/* Portrait layout: smaller information strip, native-resolution large photo.
 * V1/V2 byte layouts remain immutable for existing devices and backups. */
#define PROFILE_V3_CARD_H 76u
#define PROFILE_V3_AVATAR_W 112u
#define PROFILE_V3_AVATAR_H 136u
#define PROFILE_V3_BRAND_H 32u
#define PROFILE_V3_CARD_BYTES (PROFILE_CARD_W*PROFILE_V3_CARD_H*2u)
#define PROFILE_V3_AVATAR_BYTES (PROFILE_V3_AVATAR_W*PROFILE_V3_AVATAR_H*2u)
#define PROFILE_V3_BRAND_BYTES (PROFILE_BRAND_W*PROFILE_V3_BRAND_H*2u)
#define PROFILE_V3_BYTES (PROFILE_V3_CARD_BYTES+PROFILE_V3_AVATAR_BYTES+PROFILE_V3_BRAND_BYTES+PROFILE_LOGO_BYTES+PROFILE_QR_BYTES)
_Static_assert(PROFILE_DATA_OFFSET+PROFILE_V3_BYTES<=PROFILE_SLOT_SIZE,"Portrait bank overflow");
/* V4 atlas: 84x148 side panel followed by 208x40 footer; remaining
 * bytes are padding. All earlier formats keep their original sizes. */
#define PROFILE_V4_CARD_H 104u
#define PROFILE_V4_PANEL_W 84u
#define PROFILE_V4_PANEL_H 148u
#define PROFILE_V4_FOOTER_H 40u
#define PROFILE_V4_PANEL_BYTES (PROFILE_V4_PANEL_W*PROFILE_V4_PANEL_H*2u)
#define PROFILE_V4_CARD_BYTES (PROFILE_CARD_W*PROFILE_V4_CARD_H*2u)
#define PROFILE_V4_AVATAR_H 144u
#define PROFILE_V4_AVATAR_BYTES (PROFILE_V3_AVATAR_W*PROFILE_V4_AVATAR_H*2u)
#define PROFILE_V4_BYTES (PROFILE_V4_CARD_BYTES+PROFILE_V4_AVATAR_BYTES+PROFILE_V3_BRAND_BYTES+PROFILE_LOGO_BYTES+PROFILE_QR_BYTES)
_Static_assert(PROFILE_V4_PANEL_BYTES+PROFILE_CARD_W*PROFILE_V4_FOOTER_H*2u<=PROFILE_V4_CARD_BYTES,"Tactical atlas overflow");
_Static_assert(PROFILE_DATA_OFFSET+PROFILE_V4_BYTES<=PROFILE_SLOT_SIZE,"Tactical bank overflow");
size_t profile_asset_size(unsigned version,unsigned asset);
#define PROFILE_JSON_MAX 2048u
_Static_assert(PROFILE_DATA_OFFSET+PROFILE_V2_BYTES<=PROFILE_SLOT_SIZE,"Profile bank overflow");
typedef enum {PROFILE_ASSET_CARD,PROFILE_ASSET_AVATAR,PROFILE_ASSET_BRAND,PROFILE_ASSET_LOGO,PROFILE_ASSET_QR} profile_asset_t;

typedef struct {
    uint32_t magic,version,sequence,json_length,pixel_length,payload_crc,flags,header_crc;
} profile_header_t;
uint32_t profile_crc32(uint32_t state,const void *data,size_t size);
bool profile_header_valid(const profile_header_t *h);
bool profile_sequence_newer(uint32_t a,uint32_t b);
