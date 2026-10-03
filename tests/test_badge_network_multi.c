#include "badge_network_rules.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    badge_wifi_store_t store = {0};
    assert(store.count == 0);

    /* 1. Basic upsert and find */
    assert(badge_wifi_store_upsert(&store, "Home_WiFi", "12345678", 100));
    assert(store.count == 1);
    const badge_known_wifi_t *found = badge_wifi_store_find(&store, "Home_WiFi");
    assert(found != NULL);
    assert(strcmp(found->password, "12345678") == 0);
    assert(found->last_used == 100);

    /* 2. Update existing network password and timestamp */
    assert(badge_wifi_store_upsert(&store, "Home_WiFi", "87654321", 200));
    assert(store.count == 1);
    found = badge_wifi_store_find(&store, "Home_WiFi");
    assert(found != NULL);
    assert(strcmp(found->password, "87654321") == 0);
    assert(found->last_used == 200);

    /* 3. Add multiple networks up to capacity */
    assert(badge_wifi_store_upsert(&store, "Office_5G", "officepass", 150));
    assert(badge_wifi_store_upsert(&store, "Coffee_Shop", "cafepass", 120));
    assert(badge_wifi_store_upsert(&store, "Phone_Hotspot", "hotspotpass", 180));
    assert(badge_wifi_store_upsert(&store, "Hotel_Guest", "hotelpass", 110));
    assert(badge_wifi_store_upsert(&store, "Campus_Net", "campuspass", 90));
    assert(badge_wifi_store_upsert(&store, "Gym_Free", "gympass", 80));
    assert(badge_wifi_store_upsert(&store, "Airport_WiFi", "airpass", 70));
    assert(store.count == BADGE_KNOWN_WIFI_MAX);

    /* 4. LRU Eviction when full: Airport_WiFi has lowest timestamp (70) */
    assert(badge_wifi_store_find(&store, "Airport_WiFi") != NULL);
    assert(badge_wifi_store_upsert(&store, "Friend_House", "friendpass", 300));
    assert(store.count == BADGE_KNOWN_WIFI_MAX);
    assert(badge_wifi_store_find(&store, "Airport_WiFi") == NULL); /* Evicted */
    assert(badge_wifi_store_find(&store, "Friend_House") != NULL);

    /* 5. Match with scanned APs */
    badge_scan_entry_t scanned[4] = {
        {.ssid = "Random_Neighbor", .rssi = -40, .auth = 3},
        {.ssid = "Office_5G", .rssi = -65, .auth = 3},
        {.ssid = "Phone_Hotspot", .rssi = -45, .auth = 3}, /* Hotspot is stronger */
        {.ssid = "Unknown_Network", .rssi = -30, .auth = 3},
    };
    int best = badge_wifi_match_best(&store, scanned, 4);
    assert(best >= 0);
    assert(strcmp(store.items[best].ssid, "Phone_Hotspot") == 0);

    /* If phone hotspot drops, Office_5G becomes the best match */
    scanned[2].rssi = -95;
    best = badge_wifi_match_best(&store, scanned, 4);
    assert(best >= 0);
    assert(strcmp(store.items[best].ssid, "Office_5G") == 0);

    /* If no known networks are around */
    badge_scan_entry_t unknown_only[2] = {
        {.ssid = "Random_1", .rssi = -30, .auth = 3},
        {.ssid = "Random_2", .rssi = -40, .auth = 3},
    };
    best = badge_wifi_match_best(&store, unknown_only, 2);
    assert(best == -1);

    printf("Multi-AP store & scan matching tests: PASS\n");
    return 0;
}
