#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "xiaozhi_logic.h"
#include "xiaozhi_face_variants.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {xz_state_t state;char detail[256],text[512],heard[256],code[32];unsigned volume,level,face_variant;uint32_t played_ms,caption_start_ms,level_until_ms;xz_face_t emotion;} xz_snapshot_t;
void xiaozhi_ui_create(void);
void xiaozhi_ui_render(const xz_snapshot_t *state);
/* Nonblocking worker handoff; actual layout runs on the LVGL timer task. */
bool xiaozhi_ui_post(const xz_snapshot_t *state);
void xiaozhi_ui_destroy(void);
#ifdef __cplusplus
}
#endif
