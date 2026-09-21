#include "badge_network_rules.h"
#include <string.h>
int badge_network_power_policy(int voice_policy,bool standby){
    return voice_policy==2?2:(voice_policy==1||standby)?1:0;
}
badge_setup_action_t badge_setup_tick(bool *seen,bool onboarding,bool active,uint32_t idle) {
    bool previous=*seen;*seen=onboarding;
    if(onboarding)return !previous&&!active?BADGE_SETUP_START:BADGE_SETUP_IDLE;
    if(previous)return BADGE_SETUP_EXTEND;
    return active&&idle>600?BADGE_SETUP_CLOSE:BADGE_SETUP_IDLE;
}
bool badge_address_is_ap(const uint8_t *address,size_t size) {
    const uint8_t ap[4]={192,168,4,1};
    const uint8_t mapped[12]={0,0,0,0,0,0,0,0,0,0,255,255};
    if(!address)return false;
    if(size==4)return memcmp(address,ap,4)==0;
    return size==16 && memcmp(address,mapped,12)==0 && memcmp(address+12,ap,4)==0;
}

void badge_scan_insert(badge_scan_entry_t *entries,size_t *count,const badge_scan_entry_t *entry) {
    if(!entry->ssid[0]||entry->ssid[32]||*count>BADGE_SCAN_LIMIT)return;
    size_t index=*count;
    for(size_t i=0;i<*count;i++)if(entries[i].auth==entry->auth&&!strcmp(entries[i].ssid,entry->ssid)){index=i;break;}
    if(index<*count){if(entries[index].rssi>=entry->rssi)return;}
    else if(*count<BADGE_SCAN_LIMIT)(*count)++;
    else {if(entry->rssi<=entries[*count-1].rssi)return;index=*count-1;}
    entries[index]=*entry;
    while(index&&entries[index].rssi>entries[index-1].rssi){badge_scan_entry_t tmp=entries[index-1];entries[index-1]=entries[index];entries[index]=tmp;index--;}
}
