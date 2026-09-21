#include "badge_theme.h"
#include <string.h>
static const uint32_t defaults[BADGE_COLOR_COUNT]={0x08090B,0x181216,0xF03543,0xE8E6DF,0xA69C9F};
static uint32_t colors[BADGE_COLOR_COUNT]={0x08090B,0x181216,0xF03543,0xE8E6DF,0xA69C9F};
static uint32_t revision=1;
void badge_theme_set(const uint32_t next[BADGE_COLOR_COUNT]) {
    if(!next)next=defaults;
    if(memcmp(colors,next,sizeof(colors))){memcpy(colors,next,sizeof(colors));revision++;}
}
const uint32_t *badge_theme_colors(void) {return colors;}
uint32_t badge_theme_revision(void) {return revision;}
