/* Exercise deletion failures against the actual ADC/button BSP. */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include "../components/bsp/src/bsp_button.c"
struct button_dev_t {bool live;};
static struct button_dev_t buttons[3];
static int adc_token,cal_token,adc_live,cal_live,live_buttons;
static int fail_delete,fail_create,creates,fail_callback,callbacks;
esp_err_t adc_oneshot_new_unit(const adc_oneshot_unit_init_cfg_t *c,adc_oneshot_unit_handle_t *h){(void)c;assert(!adc_live);adc_live=1;*h=&adc_token;return ESP_OK;}
esp_err_t adc_oneshot_del_unit(adc_oneshot_unit_handle_t h){assert(h==&adc_token&&adc_live&&!live_buttons);adc_live=0;return ESP_OK;}
esp_err_t adc_oneshot_read(adc_oneshot_unit_handle_t h,int c,int *v){(void)c;assert(h==&adc_token&&adc_live);*v=200;return ESP_OK;}
esp_err_t adc_cali_create_scheme_curve_fitting(const adc_cali_curve_fitting_config_t *c,adc_cali_handle_t *h){(void)c;assert(adc_live&&!cal_live);cal_live=1;*h=&cal_token;return ESP_OK;}
esp_err_t adc_cali_delete_scheme_curve_fitting(adc_cali_handle_t h){assert(h==&cal_token&&!live_buttons);cal_live=0;return ESP_OK;}
esp_err_t adc_cali_raw_to_voltage(adc_cali_handle_t h,int v,int *mv){assert(h==&cal_token);*mv=v;return ESP_OK;}
esp_err_t iot_button_new_adc_device(const button_config_t *b,const button_adc_config_t *c,button_handle_t *h){(void)b;assert(adc_live&&*c->adc_handle==&adc_token);if(++creates==fail_create)return ESP_FAIL;*h=&buttons[c->button_index];(*h)->live=true;live_buttons++;return ESP_OK;}
esp_err_t iot_button_delete(button_handle_t h){assert(h->live&&adc_live);if(fail_delete)return ESP_FAIL;h->live=false;live_buttons--;return ESP_OK;}
esp_err_t iot_button_register_cb(button_handle_t h,button_event_t e,button_event_args_t *a,button_cb_t cb,void *u){(void)e;(void)a;(void)cb;(void)u;assert(h->live);return ++callbacks==fail_callback?ESP_FAIL:ESP_OK;}
esp_err_t gpio_config(const gpio_config_t *c){(void)c;assert(!adc_live&&!live_buttons);return ESP_OK;}
esp_err_t esp_deep_sleep_enable_gpio_wakeup(uint64_t mask,int mode){assert(mask==(1ULL<<BSP_BTN_GPIO)&&mode==ESP_GPIO_WAKEUP_GPIO_LOW);return ESP_OK;}
static void clean(void){assert(!adc_live&&!cal_live&&!live_buttons&&!s_ready);creates=callbacks=0;fail_create=fail_callback=0;}
int main(void){
 for(int n=1;n<=3;n++){fail_create=n;assert(bsp_button_init(NULL,NULL)!=ESP_OK);clean();}
 for(int n=1;n<=15;n++){fail_callback=n;assert(bsp_button_init(NULL,NULL)!=ESP_OK);clean();}
 assert(bsp_button_init(NULL,NULL)==ESP_OK);assert(bsp_button_read_mv()==200);
 fail_delete=1;button_cleanup();assert(adc_live&&cal_live&&live_buttons==3);
 assert(bsp_button_init(NULL,NULL)==ESP_ERR_INVALID_STATE);
 assert(bsp_button_prepare_deep_sleep()==ESP_ERR_INVALID_STATE);
 fail_delete=0;button_cleanup();clean();
 assert(bsp_button_init(NULL,NULL)==ESP_OK);
 assert(bsp_button_prepare_deep_sleep()==ESP_OK);clean();
 puts("Button init faults, deletion failure preserves ADC, retry and deep sleep: PASS");
}
