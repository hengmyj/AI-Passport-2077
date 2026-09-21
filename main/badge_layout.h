#pragma once
/* Source regions in the existing 208x148 saved card, followed by screen
 * position and LVGL scale (256 = 1x). The stored format is unchanged. */
typedef struct { unsigned sx,sy,w,h,x,y,scale; } badge_region_t;
static const badge_region_t BADGE_REGIONS[] = {
    {8,8,72,88,66,74,384},
    {88,4,112,32,14,211,256},
    {88,35,112,20,14,246,230},
    {88,60,112,20,122,246,230},
    {8,105,192,18,14,263,230},
    {0,126,208,20,14,279,128}
};
#define BADGE_REGION_COUNT (sizeof(BADGE_REGIONS)/sizeof(BADGE_REGIONS[0]))
