#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Keep persisted style IDs 0/1 compatible with earlier firmware. */
enum {XZ_STYLE_CUTE,XZ_STYLE_ABSTRACT,XZ_STYLE_FEMALE,XZ_STYLE_ROUND,XZ_STYLE_COUNT};
/* Init/set from navigation task outside LVGL lock; get is cached and thread-safe. */
void xiaozhi_style_init(void);
unsigned xiaozhi_style_get(void);
bool xiaozhi_style_set(unsigned style);
#ifdef __cplusplus
}
#endif
