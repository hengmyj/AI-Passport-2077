#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {const uint8_t *bits;unsigned width,height,advance;int x,y;} profile_glyph_t;
typedef bool (*profile_glyph_lookup_t)(uint32_t cp,profile_glyph_t *glyph);
typedef struct {profile_glyph_t glyphs[32];unsigned count,top,height;uint16_t bg,fg;} profile_text_patch_t;
bool profile_text_prepare(profile_text_patch_t *p,unsigned field,const char *text,uint16_t bg,uint16_t fg,profile_glyph_lookup_t lookup);
/* Patches only one of the three V4 text rectangles, in a streamed RGB565 atlas. */
void profile_text_apply(size_t offset,uint8_t *bytes,size_t size,void *context);
