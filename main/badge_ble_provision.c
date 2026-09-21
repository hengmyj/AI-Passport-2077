#include "badge_ble_provision.h"
#include "badge_network.h"

#include "cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_hs_mbuf.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "os/os_mbuf.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <stdio.h>
#include <string.h>

static const char *TAG="badge_ble";
static const char *DEVICE_NAME="AI Passport WiFi";
#define BLE_STOP_TIMEOUT_MS 2000
#define BLE_PAYLOAD_MAX 160

static SemaphoreHandle_t stopped;
static SemaphoreHandle_t lock;
static bool initialized;
static bool requested;
static bool connected;
static bool stopping;
static uint16_t current_conn=BLE_HS_CONN_HANDLE_NONE;
static uint8_t own_addr_type;
static int last_error;
static char status_text[96]="BLE 未开启";

static int gap_event(struct ble_gap_event *event,void *arg);
static int advertise(void);

static void set_status(const char *text,int error) {
    if(lock)xSemaphoreTake(lock,portMAX_DELAY);
    snprintf(status_text,sizeof(status_text),"%s",text?text:"");
    last_error=error;
    if(lock)xSemaphoreGive(lock);
}

static int status_chr(uint16_t conn_handle,uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn_handle;(void)attr_handle;(void)arg;
    char snapshot[128];
    if(lock)xSemaphoreTake(lock,portMAX_DELAY);
    if(last_error)snprintf(snapshot,sizeof(snapshot),"%s err=%d",status_text,last_error);
    else snprintf(snapshot,sizeof(snapshot),"%s",status_text);
    if(lock)xSemaphoreGive(lock);
    return os_mbuf_append(ctxt->om,snapshot,strlen(snapshot))==0?0:BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int write_wifi_chr(uint16_t conn_handle,uint16_t attr_handle,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn_handle;(void)attr_handle;(void)arg;
    char payload[BLE_PAYLOAD_MAX+1]={0};
    uint16_t length=0;
    int rc=ble_hs_mbuf_to_flat(ctxt->om,payload,BLE_PAYLOAD_MAX,&length);
    if(rc!=0)return BLE_ATT_ERR_UNLIKELY;
    payload[length<BLE_PAYLOAD_MAX?length:BLE_PAYLOAD_MAX]=0;

    cJSON *root=cJSON_Parse(payload);
    const cJSON *ssid=cJSON_GetObjectItemCaseSensitive(root,"ssid");
    const cJSON *password=cJSON_GetObjectItemCaseSensitive(root,"password");
    const cJSON *open=cJSON_GetObjectItemCaseSensitive(root,"open");
    const cJSON *keep=cJSON_GetObjectItemCaseSensitive(root,"keep");
    bool is_open=cJSON_IsTrue(open);
    bool keep_password=cJSON_IsTrue(keep);
    const char *ssid_text=cJSON_IsString(ssid)?ssid->valuestring:"";
    const char *password_text=cJSON_IsString(password)?password->valuestring:"";
    esp_err_t err=badge_network_save(ssid_text,password_text,is_open,keep_password);
    cJSON_Delete(root);

    if(err==ESP_OK) {
        set_status("Wi-Fi 已保存，正在连接",0);
        ESP_LOGI(TAG,"Wi-Fi saved over BLE: ssid=%s",ssid_text);
        return 0;
    }
    char message[96];
    snprintf(message,sizeof(message),"Wi-Fi 保存失败：%s",esp_err_to_name(err));
    set_status(message,err);
    ESP_LOGW(TAG,"BLE Wi-Fi save failed: %s",esp_err_to_name(err));
    return err==ESP_ERR_INVALID_ARG?BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN:BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def services[]={
    {
        .type=BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid=BLE_UUID128_DECLARE(0x77,0x20,0x7a,0x11,0x42,0xe8,0x45,0x3d,0xa4,0x4d,0x66,0x49,0x57,0x49,0x46,0x49),
        .characteristics=(struct ble_gatt_chr_def[]){
            {
                .uuid=BLE_UUID128_DECLARE(0x77,0x20,0x7a,0x12,0x42,0xe8,0x45,0x3d,0xa4,0x4d,0x66,0x49,0x57,0x49,0x46,0x49),
                .access_cb=write_wifi_chr,
                .flags=BLE_GATT_CHR_F_WRITE|BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                .uuid=BLE_UUID128_DECLARE(0x77,0x20,0x7a,0x13,0x42,0xe8,0x45,0x3d,0xa4,0x4d,0x66,0x49,0x57,0x49,0x46,0x49),
                .access_cb=status_chr,
                .flags=BLE_GATT_CHR_F_READ,
            },
            {0}
        },
    },
    {0}
};

static int advertise(void) {
    struct ble_hs_adv_fields fields={0};
    fields.flags=BLE_HS_ADV_F_DISC_GEN|BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name=(const uint8_t *)DEVICE_NAME;
    fields.name_len=strlen(DEVICE_NAME);
    fields.name_is_complete=1;
    int rc=ble_gap_adv_set_fields(&fields);
    if(rc!=0)return rc;

    struct ble_gap_adv_params params={0};
    params.conn_mode=BLE_GAP_CONN_MODE_UND;
    params.disc_mode=BLE_GAP_DISC_MODE_GEN;
    rc=ble_gap_adv_start(own_addr_type,NULL,BLE_HS_FOREVER,&params,gap_event,NULL);
    if(rc==0)set_status("BLE 配网已开启，等待手机连接",0);
    return rc;
}

static int gap_event(struct ble_gap_event *event,void *arg) {
    (void)arg;
    switch(event->type) {
    case BLE_GAP_EVENT_CONNECT:
        connected=event->connect.status==0;
        current_conn=connected?event->connect.conn_handle:BLE_HS_CONN_HANDLE_NONE;
        if(connected)set_status("手机已连接，请写入 Wi-Fi JSON",0);
        else if(requested)advertise();
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        connected=false;
        current_conn=BLE_HS_CONN_HANDLE_NONE;
        if(requested)advertise();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        if(requested)advertise();
        break;
    default:
        break;
    }
    return 0;
}

static void on_reset(int reason) {
    connected=false;
    current_conn=BLE_HS_CONN_HANDLE_NONE;
    set_status("BLE 主机重置",reason);
}

static void on_sync(void) {
    int rc=ble_hs_util_ensure_addr(0);
    if(rc==0)rc=ble_hs_id_infer_auto(0,&own_addr_type);
    if(rc==0&&requested)rc=advertise();
    if(rc!=0)set_status("BLE 广播启动失败",rc);
}

static void host_task(void *arg) {
    (void)arg;
    nimble_port_run();
    if(stopped)xSemaphoreGive(stopped);
    nimble_port_freertos_deinit();
}

esp_err_t badge_ble_provision_start(void) {
    if(initialized)return ESP_OK;
    if(!lock)lock=xSemaphoreCreateMutex();
    if(!lock)return ESP_ERR_NO_MEM;
    set_status("BLE 正在启动",0);
    esp_err_t err=nimble_port_init();
    if(err!=ESP_OK){set_status("BLE 初始化失败",err);return err;}
    initialized=true;
    requested=true;
    stopping=false;
    connected=false;
    stopped=xSemaphoreCreateBinary();
    if(!stopped){
        requested=false;initialized=false;nimble_port_deinit();
        set_status("BLE 内存不足",ESP_ERR_NO_MEM);
        return ESP_ERR_NO_MEM;
    }

    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc=ble_gatts_count_cfg(services);
    if(rc==0)rc=ble_gatts_add_svcs(services);
    if(rc==0)rc=ble_svc_gap_device_name_set(DEVICE_NAME);
    if(rc!=0){
        requested=false;vSemaphoreDelete(stopped);stopped=NULL;nimble_port_deinit();initialized=false;
        set_status("BLE 服务注册失败",rc);
        return ESP_FAIL;
    }

    ble_hs_cfg.reset_cb=on_reset;
    ble_hs_cfg.sync_cb=on_sync;
    nimble_port_freertos_init(host_task);
    return ESP_OK;
}

esp_err_t badge_ble_provision_stop(void) {
    requested=false;
    if(!initialized){set_status("BLE 未开启",0);return ESP_OK;}
    if(!stopping){
        if(connected&&current_conn!=BLE_HS_CONN_HANDLE_NONE)(void)ble_gap_terminate(current_conn,BLE_ERR_REM_USER_CONN_TERM);
        connected=false;
        current_conn=BLE_HS_CONN_HANDLE_NONE;
        (void)ble_gap_adv_stop();
        int rc=nimble_port_stop();
        if(rc!=0){set_status("BLE 停止失败",rc);return ESP_FAIL;}
        stopping=true;
    }
    if(stopped&&xSemaphoreTake(stopped,pdMS_TO_TICKS(BLE_STOP_TIMEOUT_MS))!=pdTRUE){
        set_status("BLE 停止超时",ESP_ERR_TIMEOUT);
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err=nimble_port_deinit();
    if(stopped){vSemaphoreDelete(stopped);stopped=NULL;}
    initialized=false;
    stopping=false;
    set_status(err==ESP_OK?"BLE 未开启":"BLE 释放失败",err==ESP_OK?0:err);
    return err;
}

void badge_ble_provision_status(badge_ble_provision_status_t *out) {
    if(!out)return;
    memset(out,0,sizeof(*out));
    snprintf(out->name,sizeof(out->name),"%s",DEVICE_NAME);
    if(lock)xSemaphoreTake(lock,portMAX_DELAY);
    out->active=initialized&&requested;
    out->connected=connected;
    snprintf(out->status,sizeof(out->status),"%s",status_text);
    out->last_error=last_error;
    if(lock)xSemaphoreGive(lock);
}
