#pragma once
#include <stdbool.h>
#include <stdint.h>
#define BADGE_ALARM_COUNT 8
#define BADGE_CLOCK_MIN INT64_C(1704067200)
#define BADGE_CLOCK_MAX INT64_C(4102444800)
typedef struct {
    uint32_t id;
    uint16_t minute;
    uint8_t weekdays,pending;
    int64_t due,last;
    char text[97];
} badge_alarm_t;
typedef struct {uint32_t magic,next_id;badge_alarm_t items[BADGE_ALARM_COUNT];} badge_alarms_t;
void badge_alarms_init(badge_alarms_t *b);
bool badge_alarms_valid(const badge_alarms_t *b);
/* UTC storage, fixed UTC+8 civil schedules. Monday is mask bit zero. */
int64_t badge_alarm_next(int64_t now,unsigned minute,unsigned weekdays);
bool badge_alarm_time(const char *hhmm,unsigned *minute);
bool badge_alarm_date(const char *date,int64_t *local_day);
bool badge_alarm_schedule(int64_t now,unsigned seconds,const char *time,const char *date,unsigned weekdays,int64_t *due,unsigned *minute);
bool badge_alarms_add(badge_alarms_t *b,int64_t now,int64_t due,unsigned minute,unsigned weekdays,const char *text,uint32_t *id);
bool badge_alarms_cancel(badge_alarms_t *b,uint32_t id);
/* Pending alerts survive reboot; acknowledge removes one-shot entries. */
bool badge_alarms_ack(badge_alarms_t *b,uint32_t id);
bool badge_alarms_poll(badge_alarms_t *b,int64_t now,badge_alarm_t *out);
void badge_alarm_format(int64_t utc,char out[20]);
