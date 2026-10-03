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
#define BADGE_KNOWN_WIFI_MAX 8

typedef struct {
    char ssid[33];
    char password[65];
    uint32_t last_used;
} badge_known_wifi_t;

typedef struct {
    uint8_t count;
    badge_known_wifi_t items[BADGE_KNOWN_WIFI_MAX];
} badge_wifi_store_t;

/* No hardware dependencies: merge repeated APs and retain strongest entries. */
typedef struct {char ssid[33];int rssi;unsigned auth;} badge_scan_entry_t;
void badge_scan_insert(badge_scan_entry_t *entries,size_t *count,const badge_scan_entry_t *entry);

/* Manage known Wi-Fi networks (add/update/LRU evict/find) */
bool badge_wifi_store_upsert(badge_wifi_store_t *store, const char *ssid, const char *password, uint32_t timestamp);
bool badge_wifi_store_delete(badge_wifi_store_t *store, const char *ssid);
const badge_known_wifi_t *badge_wifi_store_find(const badge_wifi_store_t *store, const char *ssid);
bool badge_wifi_store_touch(badge_wifi_store_t *store, const char *ssid, uint32_t timestamp);

/* Match scanned APs with known Wi-Fi list and return the strongest matching entry, or -1 if none */
int badge_wifi_match_best(const badge_wifi_store_t *store, const badge_scan_entry_t *scanned, size_t scan_count);

