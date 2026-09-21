#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool badge_address_is_ap(const uint8_t *address,size_t size);
/* 0 MIN, 1 MAX, 2 NONE. Active voice always wins over a stale screen state. */
int badge_network_power_policy(int voice_policy,bool standby);
typedef enum {BADGE_SETUP_IDLE,BADGE_SETUP_START,BADGE_SETUP_EXTEND,BADGE_SETUP_CLOSE} badge_setup_action_t;
badge_setup_action_t badge_setup_tick(bool *was_onboarding,bool onboarding,bool active,uint32_t idle_seconds);

#define BADGE_SCAN_LIMIT 16
/* No hardware dependencies: merge repeated APs and retain strongest entries. */
typedef struct {char ssid[33];int rssi;unsigned auth;} badge_scan_entry_t;
void badge_scan_insert(badge_scan_entry_t *entries,size_t *count,const badge_scan_entry_t *entry);
