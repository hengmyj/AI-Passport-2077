#pragma once
#include "radio_player.h"
#include "radio_controls.h"
struct radio_ui_snapshot_t {
    RadioStation station{};unsigned index=0,count=0;int volume=40,battery=-1;
    RadioPlaybackState state=RadioPlaybackState::Stopped;
    bool network=false;char location[48]="通用电台",detail[80]="等待网络";
    uint8_t levels[18]{};
};
/* Setters only copy state. They never touch LVGL or hold its lock. */
void radio_ui_reset_state();
radio_ui_snapshot_t radio_ui_snapshot();
void radio_ui_set_station(std::size_t index,std::size_t count,const RadioStation &station);
void radio_ui_set_playback(RadioPlaybackState state,const char *detail=nullptr);
void radio_ui_set_audio_levels(const uint8_t *levels,std::size_t count);
void radio_ui_set_volume(uint8_t volume);
void radio_ui_set_location(const char *name);
void radio_ui_set_network(bool connected);
void radio_ui_set_battery(int battery);
/* Lifecycle/refresh below require the LVGL lock. */
void radio_ui_create();
void radio_ui_destroy();
void radio_ui_render(const radio_controls_t &control,const radio_ui_snapshot_t &state,unsigned seconds_left);
