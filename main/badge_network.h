#pragma once
/* MAX modem sleep for silent screen-off standby/XiaoZhi idle, NONE during
 * voice, MIN otherwise. Non-blocking requests; no forced Wi-Fi disconnect. */

#include "esp_err.h"
#include "cJSON.h"
#include <stdbool.h>
void badge_network_xiaozhi_power(bool active,bool busy);
void badge_network_standby(bool asleep);
typedef struct {bool active,connected;char ssid[33],ap_ssid[24],ap_password[13],ip[16],message[48];} badge_network_status_t;
esp_err_t badge_network_init(void);
esp_err_t badge_network_toggle(void);
esp_err_t badge_network_start_setup(void);
/* Explicitly close the SoftAP and configuration web server if active. */
esp_err_t badge_network_close_ap(void);
/* Atomic request; safe under LVGL lock. Worker owns AP and idle timeout. */
void badge_network_onboarding(bool enabled);
esp_err_t badge_network_save(const char *ssid,const char *password,bool open,bool keep);
void badge_network_status(badge_network_status_t *status);
void badge_network_json(cJSON *reply);

esp_err_t badge_network_scan(void);
void badge_network_scan_json(cJSON *reply);
/* Start shared RF if necessary, without scanning, reconnecting or opening AP. */
esp_err_t badge_network_prepare_entropy(void);
bool badge_network_entropy_ready(void);
/* Terminal shutdown: joins the network worker; restart required afterwards. */
esp_err_t badge_network_shutdown(void);
