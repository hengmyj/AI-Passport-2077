#include "badge_alarm_service.h"
#include <string.h>
#include <time.h>
static badge_alarms_t book;
static bool ready;
#ifdef BADGE_CONTROL_HOST_TEST
int64_t badge_alarm_test_now=1800000000;
bool badge_alarm_test_write_fail;
static badge_alarms_t persisted;
static bool exists;
#define LOCK() ((void)0)
#define UNLOCK() ((void)0)
static bool save(const badge_alarms_t *b){if(badge_alarm_test_write_fail)return false;persisted=*b;exists=true;return true;}
int64_t badge_alarm_service_now(void){return badge_alarm_test_now;}
bool badge_alarm_service_init(void){if(exists)book=persisted;else badge_alarms_init(&book);return ready=badge_alarms_valid(&book);}
#else
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t mutex;
#define LOCK() xSemaphoreTake(mutex,portMAX_DELAY)
#define UNLOCK() xSemaphoreGive(mutex)
static bool save(const badge_alarms_t *b){
    nvs_handle_t h;esp_err_t e=nvs_open("badge_alarms",NVS_READWRITE,&h);if(e!=ESP_OK)return false;
    e=nvs_set_blob(h,"v1",b,sizeof(*b));if(e==ESP_OK)e=nvs_commit(h);nvs_close(h);return e==ESP_OK;
}
int64_t badge_alarm_service_now(void){return (int64_t)time(NULL);}
bool badge_alarm_service_init(void){
    mutex=xSemaphoreCreateMutex();if(!mutex)return false;
    badge_alarms_init(&book);nvs_handle_t h;esp_err_t e=nvs_open("badge_alarms",NVS_READONLY,&h);
    if(e==ESP_ERR_NVS_NOT_FOUND)return ready=true;
    if(e!=ESP_OK)return false;
    size_t size=sizeof(book);e=nvs_get_blob(h,"v1",&book,&size);nvs_close(h);
    if(e==ESP_ERR_NVS_NOT_FOUND){badge_alarms_init(&book);return ready=true;}
    return ready=e==ESP_OK&&size==sizeof(book)&&badge_alarms_valid(&book);
}
#endif
bool badge_alarm_service_add(int64_t due,unsigned minute,unsigned days,const char *text,uint32_t *id){
    if(!ready)return false;
    LOCK();badge_alarms_t next=book;uint32_t created=0;
    bool ok=badge_alarms_add(&next,badge_alarm_service_now(),due,minute,days,text,&created);
    if(ok&&memcmp(&next,&book,sizeof(book)))ok=save(&next);
    if(ok){book=next;*id=created;}UNLOCK();return ok;
}
static bool remove_or_ack(uint32_t id,bool ack){
    if(!ready)return false;
    LOCK();badge_alarms_t next=book;
    bool ok=ack?badge_alarms_ack(&next,id):badge_alarms_cancel(&next,id);
    if(ok)ok=save(&next);
    if(ok)book=next;
    UNLOCK();return ok;
}
bool badge_alarm_service_cancel(uint32_t id){return remove_or_ack(id,false);}
bool badge_alarm_service_ack(uint32_t id){return remove_or_ack(id,true);}
bool badge_alarm_service_poll(badge_alarm_t *out){
    if(!ready)return false;
    LOCK();badge_alarms_t next=book;badge_alarm_t candidate;
    bool due=badge_alarms_poll(&next,badge_alarm_service_now(),&candidate),ok=true;
    if(memcmp(&next,&book,sizeof(book)))ok=save(&next);
    if(ok){book=next;if(due)*out=candidate;}UNLOCK();return ok&&due;
}
cJSON *badge_alarm_service_json(void){
    cJSON *o=cJSON_CreateObject();if(!o)return NULL;
    int64_t now=badge_alarm_service_now();char clock[20];badge_alarm_format(now,clock);
    cJSON_AddStringToObject(o,"local_time",clock);cJSON_AddStringToObject(o,"timezone","Asia/Shanghai UTC+8");
    cJSON_AddBoolToObject(o,"clock_ready",now>=BADGE_CLOCK_MIN);cJSON_AddBoolToObject(o,"storage_ready",ready);
    cJSON_AddNumberToObject(o,"capacity",BADGE_ALARM_COUNT);cJSON_AddBoolToObject(o,"survives_reboot",true);
    cJSON *list=cJSON_AddArrayToObject(o,"reminders");if(!ready)return o;
    LOCK();badge_alarms_t copy=book;UNLOCK();unsigned count=0;
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++){
        const badge_alarm_t *a=&copy.items[i];if(!a->id)continue;count++;
        cJSON *item=cJSON_CreateObject();cJSON_AddNumberToObject(item,"id",a->id);cJSON_AddStringToObject(item,"text",a->text);
        badge_alarm_format(a->due,clock);cJSON_AddStringToObject(item,"next_at",clock);
        cJSON_AddNumberToObject(item,"remaining_seconds",now<BADGE_CLOCK_MIN?-1:a->due>now?(double)(a->due-now):0);
        cJSON_AddBoolToObject(item,"pending",a->pending);cJSON *days=cJSON_AddArrayToObject(item,"weekdays");
        for(unsigned d=0;d<7;d++)if(a->weekdays&(1u<<d))cJSON_AddItemToArray(days,cJSON_CreateNumber(d+1));
        cJSON_AddItemToArray(list,item);
    }cJSON_AddNumberToObject(o,"count",count);return o;
}
