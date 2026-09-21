#include "badge_alarms.h"
#ifdef BADGE_ALARM_SERVICE_TEST
#include "badge_alarm_service.h"
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    int64_t day,due;unsigned minute;char text[20];
    assert(badge_alarm_date("2026-09-17",&day));int64_t now=day*86400+14*3600-28800;
    assert(!badge_alarm_date("2026-02-29",&day));assert(badge_alarm_date("2028-02-29",&day));
    assert(!badge_alarm_date("2026-13-01",&day));assert(!badge_alarm_time("24:00",&minute));
    assert(!badge_alarm_schedule(0,10,"","",0,&due,&minute));
    assert(badge_alarm_schedule(now,0,"15:00","",127,&due,&minute)&&due==now+3600&&minute==900);
    badge_alarm_format(due,text);assert(!strcmp(text,"2026-09-17 15:00"));
    assert(badge_alarm_next(due,900,127)==due+86400);
    assert(badge_alarm_next(now,900,1)==due+4*86400); /* Monday */
    assert(!badge_alarm_schedule(now,5,"15:00","",0,&due,&minute));
    assert(!badge_alarm_schedule(now,0,"15:00","2026-09-16",0,&due,&minute));
    assert(!badge_alarm_schedule(now,0,"15:00","2026-09-18",127,&due,&minute));
    badge_alarms_t b;badge_alarms_init(&b);badge_alarm_t a;uint32_t id,other;
    assert(badge_alarms_add(&b,now,now+3600,900,127,"daily",&id));
    assert(badge_alarms_add(&b,now,now+3600,900,127,"daily",&other)&&id==other);
    assert(!badge_alarms_poll(&b,now,&a));assert(badge_alarms_poll(&b,now+3600,&a)&&a.id==id&&a.pending&&a.due==now+90000);
    badge_alarms_t reboot=b;assert(badge_alarms_valid(&reboot));assert(badge_alarms_poll(&reboot,0,&a)&&a.id==id);
    assert(badge_alarms_ack(&b,id));assert(!badge_alarms_poll(&b,now+1,&a));
    assert(!badge_alarms_poll(&b,now+10*86400+7200,&a)); /* Skip missed repeat. */
    assert(badge_alarms_add(&b,now,now+10,0,0,"one",&other));
    assert(badge_alarms_poll(&b,now+1000000,&a)&&a.id==other);assert(badge_alarms_ack(&b,other));
    assert(!badge_alarms_cancel(&b,other));assert(!badge_alarms_ack(&b,id));
    assert(badge_alarms_cancel(&b,id));
    for(unsigned i=0;i<8;i++){char label[8];snprintf(label,sizeof(label),"%u",i);assert(badge_alarms_add(&b,now,now+10,0,0,label,&id));}
    assert(!badge_alarms_add(&b,now,now+20,0,0,"ninth",&id));
    for(unsigned i=0;i<8;i++){assert(badge_alarms_poll(&b,now+10,&a));assert(badge_alarms_ack(&b,a.id));}
    assert(!badge_alarms_poll(&b,now+10,&a));
#ifdef BADGE_ALARM_SERVICE_TEST
    badge_alarm_test_now=now;assert(badge_alarm_service_init());
    assert(badge_alarm_service_add(now+5,0,0,"persistent",&id));
    assert(badge_alarm_service_init());badge_alarm_test_now=now+5;badge_alarm_test_write_fail=true;
    assert(!badge_alarm_service_poll(&a));badge_alarm_test_write_fail=false;
    assert(badge_alarm_service_poll(&a)&&a.id==id);assert(badge_alarm_service_init());assert(badge_alarm_service_poll(&a));
    badge_alarm_test_write_fail=true;assert(!badge_alarm_service_ack(id));assert(!badge_alarm_service_cancel(id));
    badge_alarm_test_write_fail=false;assert(badge_alarm_service_ack(id));assert(badge_alarm_service_init());assert(!badge_alarm_service_poll(&a));
    cJSON *o=badge_alarm_service_json();assert(cJSON_GetObjectItemCaseSensitive(o,"count")->valueint==0);cJSON_Delete(o);
#endif
    puts("Alarms PASS: clock, leap dates, weekdays, eight entries, dedupe, missed repeats, pending reboot, atomic failures");
}
