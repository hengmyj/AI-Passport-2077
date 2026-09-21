#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Strict UTF-8, no incomplete code points or partial fallback tokens. */
size_t xz_utf8_decode(const char *s, uint32_t *codepoint);
size_t xz_caption_normalize(char *out, size_t capacity, const char *text,
                            bool (*supported)(uint32_t));
bool xiaozhi_font_has_glyph(uint32_t codepoint);
#ifdef __cplusplus
}
#endif
