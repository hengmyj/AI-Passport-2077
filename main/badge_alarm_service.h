#pragma once
#include "badge_alarms.h"
#include "cJSON.h"
#ifdef __cplusplus
extern "C" {
#endif
bool badge_alarm_service_init(void);
int64_t badge_alarm_service_now(void);
bool badge_alarm_service_add(int64_t due,unsigned minute,unsigned days,const char *text,uint32_t *id);
bool badge_alarm_service_cancel(uint32_t id);
bool badge_alarm_service_ack(uint32_t id);
bool badge_alarm_service_poll(badge_alarm_t *out);
cJSON *badge_alarm_service_json(void);
#ifdef BADGE_CONTROL_HOST_TEST
extern int64_t badge_alarm_test_now;
extern bool badge_alarm_test_write_fail;
#endif
#ifdef __cplusplus
}
#endif
