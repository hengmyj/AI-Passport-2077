#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Pure three-coin calculation. Lines are ordered bottom to top. */
typedef struct {
    uint8_t number;
    const char *name, *text, *lines[6], *special;
} yao_hexagram_t;
typedef struct {
    const yao_hexagram_t *hexagram;
    uint8_t line; /* 0: guaci; 1..6: yaoci; 7: use-nine/use-six. */
    bool primary;
    const char *text;
} yao_reading_t;
typedef struct {
    const yao_hexagram_t *original, *changed;
    uint8_t lines[6], moving_mask, moving_count, reading_count;
    yao_reading_t readings[2];
} yao_result_t;
extern const yao_hexagram_t YAO_HEXAGRAMS[64];
bool yao_full_name(const yao_hexagram_t *hexagram,char *buffer,size_t capacity);
uint8_t yao_coin_line(uint8_t bits);
bool yao_calculate(const uint8_t lines[6], yao_result_t *result);
const char *yao_reading_rule(unsigned moving_count);
/* Complete reading, excluding duplicate unchanged hexagram. Returns required
 * bytes excluding NUL; NULL/0 measures. A short buffer is always terminated. */
size_t yao_format_full_reading(const yao_result_t *result,char *buffer,size_t capacity);
