#include "yao_time.h"
#include <string.h>
static yao_location_t current;
static uint32_t generation,working_generation;
static bool connected,attempted,running,failed,voice_active;
static int64_t calibrated_ms;
#define LOCATION_CACHE_MS (60LL*60*1000)
#ifdef BADGE_CONTROL_HOST_TEST
int64_t yao_location_test_ms;
static int64_t monotonic_ms(void){return yao_location_test_ms;}
#define LOCK() ((void)0)
#define UNLOCK() ((void)0)
#else
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
static int64_t monotonic_ms(void){return esp_timer_get_time()/1000;}
static portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
#define LOCK() portENTER_CRITICAL(&mux)
#define UNLOCK() portEXIT_CRITICAL(&mux)
#endif
/* Called under mux. Clock synchronization must not change the cache lifetime. */
static void expire_cache(void){
    if(current.configured&&monotonic_ms()-calibrated_ms>=LOCATION_CACHE_MS){
        current=(yao_location_t){0};attempted=false;failed=false;
    }
}
void yao_location_init(void){LOCK();current=(yao_location_t){0};calibrated_ms=0;generation=working_generation=0;connected=attempted=running=failed=voice_active=false;UNLOCK();}
void yao_location_voice_active(bool active){LOCK();voice_active=active;UNLOCK();}
bool yao_location_worker_running(void){LOCK();bool active=running;UNLOCK();return active;}
yao_location_t yao_location_get(void){LOCK();expire_cache();yao_location_t p=current;UNLOCK();return p;}
void yao_location_network(bool online){
    LOCK();
    /* Invalidate in-flight replies, but keep the worker lease until HTTP/TLS cleanup.
     * A reconnect must not allocate another network task on top of this one. */
    expire_cache();
    if(connected!=online){connected=online;if(++generation==0)generation=1;attempted=false;failed=false;}
    UNLOCK();
}
void yao_location_recheck(void){
    LOCK();
    expire_cache();
    if(connected&&!current.configured&&!running){if(++generation==0)generation=1;attempted=false;failed=false;}
    UNLOCK();
}
uint32_t yao_location_begin(void){
    LOCK();expire_cache();uint32_t id=0;
    if(connected&&!current.configured&&!voice_active&&!attempted&&!running){id=generation;working_generation=id;running=true;attempted=true;}
    UNLOCK();return id;
}
bool yao_location_active(uint32_t id){LOCK();bool ok=id&&connected&&!voice_active&&generation==id;UNLOCK();return ok;}
void yao_location_finish(uint32_t id,const yao_location_t *result){
    bool valid=result&&result->configured&&yao_location_valid(result);
    LOCK();
    if(running&&working_generation==id){
        if(connected&&generation==id){current=valid?*result:(yao_location_t){0};failed=!valid;if(valid)calibrated_ms=monotonic_ms();else attempted=false;}
        running=false;
    }
    UNLOCK();
}
const char *yao_location_status(void){
    LOCK();expire_cache();const char *s=current.configured?"ready":!connected?"offline":running&&working_generation==generation?"fetching":failed?"retrying":"pending";UNLOCK();return s;
}
