#include "yao_geo.h"
#include "cJSON.h"
#include <math.h>
#include <stdatomic.h>
#include <string.h>
#include <stdio.h>
bool yao_location_parse(const char *json,yao_location_t *out){
    if(!out)return false;
    *out=(yao_location_t){0};cJSON *o=cJSON_Parse(json);if(!o)return false;
    const cJSON *ok=cJSON_GetObjectItemCaseSensitive(o,"success"),*status=cJSON_GetObjectItemCaseSensitive(o,"status");
    const cJSON *lon=cJSON_GetObjectItemCaseSensitive(o,"longitude");if(!cJSON_IsNumber(lon))lon=cJSON_GetObjectItemCaseSensitive(o,"lon");
    const cJSON *city=cJSON_GetObjectItemCaseSensitive(o,"city"),*region=cJSON_GetObjectItemCaseSensitive(o,"region");if(!cJSON_IsString(region))region=cJSON_GetObjectItemCaseSensitive(o,"regionName");
    const char *name=cJSON_IsString(city)&&city->valuestring[0]?city->valuestring:cJSON_IsString(region)?region->valuestring:"";
    bool success=cJSON_IsTrue(ok)||(cJSON_IsString(status)&&!strcmp(status->valuestring,"success"));
    bool valid=success&&cJSON_IsNumber(lon)&&isfinite(lon->valuedouble)&&lon->valuedouble>=-180&&lon->valuedouble<=180&&strlen(name)<sizeof(out->region);
    if(valid){snprintf(out->region,sizeof(out->region),"%s",name);out->longitude_e6=(int32_t)llround(lon->valuedouble*1000000);out->configured=true;valid=yao_location_valid(out);}
    cJSON_Delete(o);if(!valid)*out=(yao_location_t){0};return valid;
}
#ifndef BADGE_CONTROL_HOST_TEST
#include "badge_power.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef struct {char data[1024];size_t used;uint32_t generation;int64_t deadline;bool invalid;} response_t;
static atomic_uint retry_at_seconds;
static bool idle(void){badge_power_status_t p=badge_power_status();return !p.screen_off&&!p.audio_owner;}
static esp_err_t receive(esp_http_client_event_t *event){
    response_t *r=event->user_data;
    if(r->invalid||!yao_location_active(r->generation)||!idle()||esp_timer_get_time()>r->deadline){r->invalid=true;return ESP_FAIL;}
    if(event->event_id==HTTP_EVENT_ON_DATA){
        if(event->data_len<0||r->used+(size_t)event->data_len>=sizeof(r->data)){r->invalid=true;return ESP_FAIL;}
        memcpy(r->data+r->used,event->data,event->data_len);r->used+=(size_t)event->data_len;r->data[r->used]=0;
    }
    return ESP_OK;
}
static bool request(uint32_t id,const char *url,yao_location_t *place){
    response_t response={.generation=id,.deadline=esp_timer_get_time()+6000000};
    bool ok=false;
    if(yao_location_active(id)&&idle()){
        esp_http_client_config_t config={
            .url=url,.timeout_ms=3500,.buffer_size=512,.buffer_size_tx=256,
            .disable_auto_redirect=true,.crt_bundle_attach=esp_crt_bundle_attach,
            .event_handler=receive,.user_data=&response,
        };
        esp_http_client_handle_t client=esp_http_client_init(&config);
        if(client){
            esp_http_client_set_header(client,"Accept-Encoding","identity");
            esp_err_t e=esp_http_client_perform(client);
            ok=e==ESP_OK&&!response.invalid&&esp_http_client_get_status_code(client)==200&&esp_timer_get_time()<=response.deadline&&yao_location_active(id)&&idle()&&yao_location_parse(response.data,place);
            esp_http_client_cleanup(client);
        }
    }
    return ok;
}
static void fetch(void *arg){
    uint32_t id=(uint32_t)(uintptr_t)arg;yao_location_t place={0};bool ok=false;
    static const char *urls[]={"https://ipwho.is/?fields=success,region,city,longitude&lang=zh-CN","http://ip-api.com/json/?fields=status,regionName,city,lon"};
    for(unsigned i=0;i<sizeof(urls)/sizeof(*urls)&&!ok&&yao_location_active(id);i++)ok=request(id,urls[i],&place);
    atomic_store(&retry_at_seconds,ok?0:(unsigned)(esp_timer_get_time()/1000000)+15);
    yao_location_finish(id,ok?&place:NULL);vTaskDelete(NULL);
}
void yao_location_poll(void){
    /* Never wake the screen or contend with active audio. Failed providers are
     * retried after a short delay without a persistent task or flash write. */
    const char *status=yao_location_status();if(!strcmp(status,"offline")){atomic_store(&retry_at_seconds,0);return;}
    unsigned now=(unsigned)(esp_timer_get_time()/1000000);
    if((strcmp(status,"pending")&&strcmp(status,"retrying"))||
       (!strcmp(status,"retrying")&&now<atomic_load(&retry_at_seconds))||
       !yao_time_valid(yao_time_now())||!idle()||heap_caps_get_free_size(MALLOC_CAP_8BIT)<70000||heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)<32768)return;
    uint32_t id=yao_location_begin();if(!id)return;
    if(xTaskCreate(fetch,"yao_location",10240,(void *)(uintptr_t)id,2,NULL)!=pdPASS)yao_location_finish(id,NULL);
}
#endif
