#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "yao_core.h"
#include "yao_time.h"
#include "cJSON.h"
#define YAO_CACHE_COUNT 4
typedef struct {uint32_t id; uint8_t lines[6]; char request[49], question[385]; int64_t cast_utc; yao_location_t location;} yao_record_t;
/* Serialized callers: navigation task while mini-app is active, voice worker
 * while AI is active. No UI timer or ISR accesses this store. RAM only. */
bool yao_latest(yao_record_t *out);
bool yao_lookup(uint32_t id,yao_record_t *out);
bool yao_save_manual(const uint8_t lines[6],yao_record_t *out);
bool yao_refresh_location(yao_record_t *record);
bool yao_random_bits(uint32_t *bits);
/* Owned JSON: NULL only for allocation failure. error is a static message. */
cJSON *yao_tool(const char *name,const cJSON *args,const char **error);
cJSON *yao_record_json(const yao_record_t *record);
cJSON *yao_time_json(int64_t utc,const yao_location_t *location);
cJSON *yao_query_time_json(void);
/* Caller frees returned prompt with cJSON_free(). */
char *yao_interpretation_prompt(const yao_record_t *record);
#ifdef __cplusplus
}
#endif
