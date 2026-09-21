#pragma once
#include "cJSON.h"
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
/* id=5 means current badge; other IDs are zero-based. Queries copy data. */
cJSON *profile_edit_get(unsigned id);
/* Caller serializes with the protocol lock. revision rejects stale edits. */
esp_err_t profile_edit_locked(unsigned id,unsigned revision,unsigned field,const char *text);
#ifdef __cplusplus
}
#endif
