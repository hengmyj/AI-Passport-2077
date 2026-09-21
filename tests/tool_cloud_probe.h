/* Four synthetic utterances: cold status, reused status, two random plays. */
extern unsigned demo_xiaozhi_cloud_completed(void);
static void tool_probe_open(void){
    configASSERT(control_home());
    configASSERT(bsp_lvgl_lock(1000));navigation.page=BADGE_AI_CHAT;navigation.ai_return=BADGE_HOME;
    badge_ui_destroy();demo_xiaozhi_enter();bsp_lvgl_unlock();configASSERT(demo_xiaozhi_start()==ESP_OK);
}
static void cloud_probe_tick(void){
    static unsigned turn=0,volume;static bool original;static int64_t deadline,next=12000000,voice_since=0;
    int64_t now=esp_timer_get_time();if(turn==99||now<next)return;next=now+250000;
    if(!turn){original=xz_wake_enabled();volume=demo_xiaozhi_saved_volume();configASSERT(xz_wake_enable(false)==ESP_OK);tool_probe_open();turn=1;deadline=now+50000000;return;}
    bool played=on_voice_page();
    if(played){if(!voice_since)voice_since=now;if(now-voice_since<2000000)return;}
    bool done=demo_xiaozhi_cloud_completed()>=turn||played||now>deadline;
    if(!done)return;
    ESP_LOGI("tool_cloud","RESULT turn=%u played=%u timeout=%u",turn,played,now>deadline);
    configASSERT(control_home());
    if(turn==4){
        demo_xiaozhi_release_connection();configASSERT(xz_wake_enable(original)==ESP_OK);
        configASSERT(demo_xiaozhi_saved_volume()==volume);
        ESP_LOGI("tool_cloud","BENCH_DONE volume_preserved=1");turn=99;return;
    }
    ++turn;voice_since=0;tool_probe_open();deadline=now+50000000;
}
