#pragma once
#include "cJSON.h"
#include "yao_service.h"
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Caller owns result. Navigation actions acknowledge before commit; in-place
 * actions commit and await completion before replacing the queued response. */
cJSON *xz_tools_handle(const cJSON *request,unsigned volume,bool connected,uint32_t *ticket);
/* Initial interpretation turn is read-only with respect to drawing new lots. */
cJSON *xz_tools_handle_reading(const cJSON *request,unsigned volume,bool connected,uint32_t *ticket,const yao_record_t *snapshot);
void xz_tools_complete(cJSON *reply,int completion);
#ifdef __cplusplus
}
#endif
