/* Normal entry with synthetic silence only; no microphone audio is uploaded. */
extern unsigned demo_xiaozhi_connect_probe(unsigned field);
static void connect_probe_tick(void){
    static unsigned phase,round;static int64_t next=15000000,deadline,stable;
    int64_t now=esp_timer_get_time();if(phase==99||now<next)return;
    next=now+250000;badge_power_activity();
    if(phase==0){
        if(!round)badge_power_set_timeout(0); /* Diagnostic session only; keep USB reachable after completion. */
        configASSERT(control_home());input_event_t e={BSP_BTN_UP,BSP_BTN_LONG};process_input(&e);
        configASSERT(navigation.page==BADGE_AI_CHAT);deadline=now+25000000;stable=0;phase=1;round++;
        ESP_LOGI("connect_probe","OPEN round=%u",round);
    }else if(phase==1){
        unsigned state=demo_xiaozhi_connect_probe(0),fault=demo_xiaozhi_connect_probe(1);
        if(fault||now>deadline){
            ESP_LOGE("connect_probe","REPRODUCED round=%u state=%u fault=%u catalogs=%u frames=%u",round,state,fault,demo_xiaozhi_connect_probe(2),demo_xiaozhi_connect_probe(3));
            configASSERT(control_home());ESP_LOGI("connect_probe","DONE failed=1");phase=99;return;
        }
        if(state==4&&!stable)stable=now;
        if(stable&&now-stable>8000000){
            configASSERT(state==4);ESP_LOGI("connect_probe","STABLE_PASS round=%u catalogs=%u frames=%u",round,demo_xiaozhi_connect_probe(2),demo_xiaozhi_connect_probe(3));
            configASSERT(control_home());
            /* Keep one reuse case; rebuild TLS/catalog on subsequent rounds
             * to stress fragmented-heap discovery, not only a parked socket. */
            if(round>=2)demo_xiaozhi_release_connection();
            if(round==6){ESP_LOGI("connect_probe","DONE failed=0");phase=99;}
            else{phase=0;next=now+3000000;}
        }
    }
}
