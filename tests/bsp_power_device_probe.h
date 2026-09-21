/* Opt-in local hardware validation; no microphone data is retained or sent. */
static void bsp_power_device_probe(void) {
    vTaskDelay(pdMS_TO_TICKS(200));
    uint8_t volume=bsp_audio_get_volume(),brightness=bsp_display_brightness();
    int16_t pcm[160],silence[160]={0};
    for(unsigned i=0;i<12;i++){
        configASSERT(bsp_audio_init()==ESP_OK);
        configASSERT(bsp_audio_set_format(i%2?8000:16000,16,1)==ESP_OK);
        bsp_audio_set_volume(63);
        configASSERT(bsp_audio_write(silence,sizeof(silence))==ESP_OK);
        configASSERT(bsp_audio_read(pcm,sizeof(pcm))==ESP_OK);
        configASSERT(bsp_audio_sleep()==ESP_OK&&bsp_audio_is_sleeping());
        configASSERT(bsp_audio_sleep()==ESP_OK);
        configASSERT(bsp_display_suspend()==ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(150));
        configASSERT(bsp_display_resume()==ESP_OK);
        configASSERT(bsp_display_brightness()==(brightness?brightness:80));
        configASSERT(bsp_audio_wake()==ESP_OK&&!bsp_audio_is_sleeping());
        configASSERT(bsp_audio_get_volume()==63);
        configASSERT(bsp_audio_write(silence,sizeof(silence))==ESP_OK);
        configASSERT(bsp_audio_read(pcm,sizeof(pcm))==ESP_OK);
        ESP_LOGI("bsp_power_probe","cycle=%u PASS",i+1);
    }
    bsp_audio_set_volume(volume);configASSERT(bsp_audio_sleep()==ESP_OK);
    ESP_LOGI("bsp_power_probe","ALL_PASS cycles=12");
}
