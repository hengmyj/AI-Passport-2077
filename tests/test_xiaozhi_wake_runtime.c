/* Run the real wake lifecycle against deterministic task/audio substitutes. */
#include "xiaozhi_wake.h"
#include "xiaozhi_wake_test_stubs.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
static int64_t now;
static size_t heap=100000,largest=60000;
static unsigned tasks,deleted,models,lists,signals,prepares,writes;
static bool task_fail,mic_fail,reclaim,power,listening;
static int semaphore;static bool semaphore_live;
static uint8_t preference;
static void (*entry)(void *);
static jmp_buf parked;
static model_iface_data_t model;
static srmodel_list_t list={{"model"}};
static void run_worker(void){assert(entry);if(!setjmp(parked))entry(NULL);}
int64_t esp_timer_get_time(void){return now;}
size_t esp_get_free_heap_size(void){return heap;}
size_t heap_caps_get_largest_free_block(int cap){assert(cap==MALLOC_CAP_8BIT);return largest;}
srmodel_list_t *srmodel_load(const void *data){assert(data&&!lists);lists++;return &list;}
static model_iface_data_t *create(const char *name,int mode){(void)name;(void)mode;assert(!models);models++;return &model;}
static void threshold(model_iface_data_t *p,float v,int word){assert(p==&model&&v>0&&word==1);}
static int chunks(model_iface_data_t *p){assert(p==&model);return 512;}
static int rate(model_iface_data_t *p){assert(p==&model);return 16000;}
static int detect(model_iface_data_t *p,int16_t *pcm){assert(p==&model&&pcm&&power&&listening);now+=1000000;return 1;}
static void destroy(model_iface_data_t *p){assert(p==&model&&models==1);models--;}
static const esp_wn_iface_t engine={create,threshold,chunks,rate,detect,destroy};
const esp_wn_iface_t *esp_wn_handle_from_name(const char *name){(void)name;return &engine;}
void esp_srmodel_deinit(srmodel_list_t *p){assert(p==&list&&lists==1);lists--;}
void badge_power_enter(void){assert(!power);power=true;}
void badge_power_set_local_listening(bool v){listening=v;}
void badge_power_tick(bool busy,bool audio){assert(!busy&&audio&&power&&listening);}
esp_err_t badge_power_leave(void){assert(power&&!listening);power=false;return ESP_OK;}
esp_err_t bsp_audio_init(void){return ESP_OK;}
esp_err_t bsp_audio_set_format(int r,int b,int c){assert(r==16000&&b==16&&c==1);return ESP_OK;}
esp_err_t bsp_audio_read(void *b,size_t n){assert(b&&n==1024);memset(b,0,n);return mic_fail?ESP_FAIL:ESP_OK;}
esp_err_t nvs_open(const char *s,int mode,nvs_handle_t *n){assert(!strcmp(s,"xiaozhi"));(void)mode;*n=1;return ESP_OK;}
esp_err_t nvs_get_u8(nvs_handle_t n,const char *k,uint8_t *v){assert(n==1&&!strcmp(k,"wake_enabled"));*v=preference;return ESP_OK;}
esp_err_t nvs_set_u8(nvs_handle_t n,const char *k,uint8_t v){assert(n==1&&!strcmp(k,"wake_enabled"));preference=v;writes++;return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t n){assert(n==1);return ESP_OK;}
void nvs_close(nvs_handle_t n){assert(n==1);}
SemaphoreHandle_t xSemaphoreCreateBinary(void){assert(!semaphore_live);semaphore_live=true;semaphore=0;return &semaphore;}
int xSemaphoreTake(SemaphoreHandle_t s,unsigned wait){assert(s==&semaphore&&wait==3000);if(!semaphore)run_worker();assert(semaphore);semaphore=0;return pdTRUE;}
void xSemaphoreGive(SemaphoreHandle_t s){assert(s==&semaphore&&!semaphore);semaphore=1;}
void vSemaphoreDelete(SemaphoreHandle_t s){assert(s==&semaphore&&semaphore_live);semaphore_live=false;}
int xTaskCreate(void (*fn)(void *),const char *name,unsigned bytes,void *arg,unsigned priority,TaskHandle_t *out){
 assert(!entry&&name&&bytes==8192&&!arg&&priority==4);if(task_fail)return 0;entry=fn;*out=&entry;tasks++;return pdPASS;
}
void vTaskDelete(TaskHandle_t h){assert(h==&entry&&entry&&!models&&!lists&&!power&&!listening);entry=NULL;deleted++;}
void vTaskSuspend(TaskHandle_t h){assert(!h);longjmp(parked,1);}
void taskYIELD(void){}
static void notify(void){signals++;}
static void prepare(void){prepares++;if(reclaim){heap=100000;largest=60000;}}
static void clean(void){assert(!entry&&!semaphore_live&&!models&&!lists&&!power&&!listening&&tasks==deleted);}
int main(void){
 xz_wake_init();assert(!xz_wake_enabled());xz_wake_on_trigger(notify);xz_wake_on_prepare(prepare);
 assert(xz_wake_enable(true)==ESP_OK&&preference==1);unsigned stored=writes;
 /* A failed memory gate must be reaped and retried, without toggling NVS. */
 heap=40000;largest=24000;xz_wake_tick(true);run_worker();assert(!xz_wake_take_trigger()&&signals==0);
 xz_wake_tick(true);clean();unsigned before=tasks;
 now+=2999999;xz_wake_tick(true);assert(tasks==before);
 reclaim=true;now++;xz_wake_tick(true);assert(prepares>=2&&entry);run_worker();assert(xz_wake_take_trigger()&&signals==1);
 assert(xz_wake_stop()==ESP_OK);clean();assert(writes==stored);
 /* Task creation failure must release the semaphore and retry on its own. */
 now+=1000000;task_fail=true;xz_wake_tick(true);clean();task_fail=false;now+=3000000;xz_wake_tick(true);run_worker();assert(xz_wake_take_trigger());assert(xz_wake_stop()==ESP_OK);clean();
 /* Microphone failure must release power/model ownership before retry. */
 now+=1000000;mic_fail=true;xz_wake_tick(true);run_worker();assert(!xz_wake_take_trigger()&&!power&&!listening);xz_wake_tick(true);clean();
 mic_fail=false;now+=3000000;xz_wake_tick(true);run_worker();assert(xz_wake_take_trigger());assert(xz_wake_stop()==ESP_OK);clean();
 /* Disable joins a still-live worker synchronously; no delayed stack reuse. */
 now+=1000000;xz_wake_tick(true);assert(entry);const char *pending_status=xz_wake_status();
 assert(xz_wake_enable(true)==ESP_OK&&!strcmp(xz_wake_status(),pending_status));
 assert(xz_wake_enable(false)==ESP_OK);clean();assert(!xz_wake_enabled()&&preference==0);
 now+=4000000;xz_wake_tick(true);clean();assert(!xz_wake_take_trigger());
 puts("Wake lifecycle: memory/task/microphone retry, reclaim, synchronous join and preference preservation PASS");
}
