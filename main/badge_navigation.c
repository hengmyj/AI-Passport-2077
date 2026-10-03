#include "badge_navigation.h"
#include "badge_network.h"
#include "xiaozhi_style.h"
#include "badge_power_logic.h"
badge_input_t badge_ai_long_gesture(badge_page_t page,bool up,bool down,bool long_press){
    if(page!=BADGE_AI_CHAT||!long_press)return BADGE_NONE;
    return up?BADGE_AI_CYCLE_STYLE:down?BADGE_AI_OPEN_SETTINGS:BADGE_NONE;
}
badge_input_t badge_ok_gesture(badge_page_t page,bool long_press,bool click) {
    if(long_press)return page==BADGE_HOME?BADGE_SWITCH:BADGE_BACK;
    return click?BADGE_OK:BADGE_NONE;
}
void badge_navigation_init(badge_navigation_t *s, size_t games) {
    *s=(badge_navigation_t){.page=BADGE_HOME,.settings_return=BADGE_TERMINAL,.ai_settings_return=BADGE_SETTINGS,.wifi_return=BADGE_SETTINGS,.games_return=BADGE_TERMINAL,.playing_return=BADGE_GAMES,.game_count=games,.brightness=3,.badge_count=1,.badge_mask=1};
    s->screen_timeout=BADGE_SCREEN_TIMEOUT_DEFAULT;
}
badge_input_t badge_up_gesture(badge_page_t page,bool configured,bool long_press,bool click) {
    if(long_press)return page==BADGE_HOME&&configured?BADGE_AI:BADGE_NONE;
    return click?BADGE_UP:BADGE_NONE;
}
badge_action_t badge_navigation_quick_launch(badge_navigation_t *s,size_t index) {
    if(s->page!=BADGE_HOME||!s->badge_mask||index>=s->game_count)return BADGE_IDLE;
    s->game_selected=index;s->playing_return=BADGE_HOME;s->page=BADGE_PLAYING;
    return BADGE_LAUNCH;
}
badge_input_t badge_return_menu_handle(badge_return_menu_t *m,badge_page_t page,badge_input_t input) {
    if(!m->open){if(input==BADGE_BACK&&page!=BADGE_HOME){m->open=true;m->selected=0;return BADGE_NONE;}return input;}
    if(input==BADGE_BACK){m->open=false;return BADGE_NONE;}
    if(input==BADGE_UP)m->selected=(m->selected+2)%3;
    if(input==BADGE_DOWN)m->selected=(m->selected+1)%3;
    if(input==BADGE_OK){m->open=false;return m->selected==0?BADGE_BACK:m->selected==1?BADGE_GO_HOME:BADGE_NONE;}
    return BADGE_NONE;
}
void badge_navigation_badges(badge_navigation_t *s,unsigned count,unsigned active,unsigned mask) {
    s->badge_count=count?count:1;s->active_badge=active<count?active:0;s->badge_mask=mask;
    if(s->page!=BADGE_CARDS||s->badge_selected>=s->badge_count)s->badge_selected=s->active_badge;
}
badge_action_t badge_navigation_handle(badge_navigation_t *s,badge_input_t input) {
    if(input==BADGE_NONE) return BADGE_IDLE;
    if(input==BADGE_AI){if(s->page!=BADGE_HOME||!s->badge_mask)return BADGE_IDLE;s->ai_return=s->page;s->page=BADGE_AI_CHAT;return BADGE_AI_START;}
    if(input==BADGE_GO_HOME){badge_action_t result=s->page==BADGE_PLAYING?BADGE_STOP:s->page==BADGE_AI_CHAT?BADGE_AI_STOP:BADGE_REDRAW;s->page=BADGE_HOME;return result;}
    if(input==BADGE_SWITCH) {
        if(s->page==BADGE_HOME&&s->badge_mask){s->badge_selected=s->active_badge;s->page=BADGE_CARDS;return BADGE_REDRAW;}
        input=BADGE_OK;
    }
    if(s->page==BADGE_CARDS) {
        if(input==BADGE_BACK){s->page=BADGE_HOME;return BADGE_REDRAW;}
        if(input==BADGE_UP)s->badge_selected=(s->badge_selected+s->badge_count-1)%s->badge_count;
        if(input==BADGE_DOWN)s->badge_selected=(s->badge_selected+1)%s->badge_count;
        if(input==BADGE_OK) {
            if(!(s->badge_mask&(1u<<s->badge_selected)))return BADGE_IDLE;
            s->page=BADGE_HOME;return BADGE_SELECT;
        }
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_AI_CHAT){
        if(input==BADGE_AI_CYCLE_STYLE){s->ai_style=(s->ai_style+1)%XZ_STYLE_COUNT;return BADGE_AI_STYLE;}
        if(input==BADGE_AI_OPEN_SETTINGS){s->ai_settings_return=BADGE_AI_CHAT;s->ai_selected=0;s->page=BADGE_AI_SETTINGS;return BADGE_AI_STOP;}
        if(input==BADGE_BACK){s->page=s->ai_return;return BADGE_AI_STOP;}
        return BADGE_IDLE;
    }
    if(s->page==BADGE_AI_SETTINGS){
        if(input==BADGE_BACK){if(s->ai_settings_return==BADGE_AI_CHAT){s->page=BADGE_AI_CHAT;s->ai_settings_return=BADGE_TERMINAL;return BADGE_AI_START;}s->page=s->ai_settings_return;return BADGE_REDRAW;}
        if(input==BADGE_UP)s->ai_selected=(s->ai_selected+4)%5;
        if(input==BADGE_DOWN)s->ai_selected=(s->ai_selected+1)%5;
        if(input==BADGE_OK){
            if(s->ai_selected==0){if(s->ai_settings_return!=BADGE_AI_CHAT)s->ai_return=BADGE_AI_SETTINGS;s->ai_settings_return=s->page==BADGE_AI_SETTINGS?BADGE_TERMINAL:s->ai_settings_return;s->page=BADGE_AI_CHAT;return BADGE_AI_START;}
            if(s->ai_selected==1)return BADGE_AI_TOGGLE;
            if(s->ai_selected==2){s->page=BADGE_AI_VOLUME_PAGE;return BADGE_REDRAW;}
            if(s->ai_selected==4){s->ai_style=(s->ai_style+1)%XZ_STYLE_COUNT;return BADGE_AI_STYLE;}
        }
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_AI_VOLUME_PAGE){
        if(input==BADGE_BACK||input==BADGE_OK){s->page=BADGE_AI_SETTINGS;return BADGE_REDRAW;}
        if(input==BADGE_UP&&s->ai_volume<100)s->ai_volume+=10;
        if(input==BADGE_DOWN&&s->ai_volume>0)s->ai_volume=s->ai_volume>10?s->ai_volume-10:0;
        if(s->ai_volume>100)s->ai_volume=100;
        return BADGE_AI_VOLUME;
    }
    if(s->page==BADGE_SLEEP_SETTINGS){
        if(input==BADGE_BACK){s->page=BADGE_SETTINGS;return BADGE_REDRAW;}
        if(input==BADGE_UP)s->timeout_selected=(s->timeout_selected+BADGE_SCREEN_TIMEOUT_COUNT-1)%BADGE_SCREEN_TIMEOUT_COUNT;
        if(input==BADGE_DOWN)s->timeout_selected=(s->timeout_selected+1)%BADGE_SCREEN_TIMEOUT_COUNT;
        if(input==BADGE_OK){s->screen_timeout=s->timeout_selected;s->page=BADGE_SETTINGS;return BADGE_SLEEP_SAVE;}
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_ABOUT_SETTINGS){
        if(input==BADGE_BACK||input==BADGE_OK){s->page=BADGE_SETTINGS;return BADGE_REDRAW;}
        return BADGE_IDLE;
    }
    if(s->page==BADGE_DISPLAY_SETTINGS){
        if(input==BADGE_BACK||input==BADGE_OK){s->page=BADGE_SETTINGS;return BADGE_REDRAW;}
        if(input==BADGE_UP&&s->brightness<4)s->brightness++;
        if(input==BADGE_DOWN&&s->brightness>0)s->brightness--;
        return BADGE_BRIGHTNESS;
    }
    if(s->page==BADGE_PLAYING) {
        if(input!=BADGE_BACK) return BADGE_IDLE;
        s->page=s->playing_return; return BADGE_STOP;
    }
    if(s->page==BADGE_QR) {
        if(input==BADGE_OK || input==BADGE_DOWN || input==BADGE_BACK) {
            s->page=s->qr_return;return BADGE_REDRAW;
        }
        return BADGE_IDLE;
    }
    if(input==BADGE_BACK) {
        if(s->page==BADGE_GAMES){s->page=s->games_return;return BADGE_REDRAW;}
        if(s->page==BADGE_WIFI){s->page=s->wifi_return;return BADGE_REDRAW;}
        if(s->page==BADGE_SETTINGS){s->page=s->settings_return;return BADGE_REDRAW;}
        s->page=(s->page==BADGE_TERMINAL||s->page==BADGE_HOME)?BADGE_HOME:BADGE_TERMINAL;
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_HOME) {
        if(!s->badge_mask){
            if(input==BADGE_OK)return BADGE_WIFI_START;
            if(input==BADGE_DOWN){s->page=BADGE_TERMINAL;return BADGE_REDRAW;}
        }
        if(input==BADGE_UP){s->games_return=BADGE_HOME;s->page=BADGE_GAMES;return BADGE_REDRAW;}
        if(input==BADGE_DOWN) s->qr_return=BADGE_HOME;
        s->page=input==BADGE_DOWN?BADGE_QR:BADGE_TERMINAL;
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_TERMINAL) {
        if(input==BADGE_UP) s->home_selected=(s->home_selected+4)%5;
        if(input==BADGE_DOWN) s->home_selected=(s->home_selected+1)%5;
        if(input==BADGE_OK) {
            static const badge_page_t destinations[]={BADGE_AI_SETTINGS,BADGE_GAMES,BADGE_QR,BADGE_SETTINGS,BADGE_HOME};
            s->page=destinations[s->home_selected];
            if(s->page==BADGE_AI_SETTINGS){s->ai_settings_return=BADGE_TERMINAL;s->ai_selected=0;badge_network_wake_probe();}
            if(s->page==BADGE_GAMES)s->games_return=BADGE_TERMINAL;
            if(s->page==BADGE_SETTINGS)s->settings_return=BADGE_TERMINAL;
            if(s->page==BADGE_QR)s->qr_return=BADGE_TERMINAL;
        }
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_GAMES) {
        if(!s->game_count) return BADGE_IDLE;
        if(input==BADGE_UP) s->game_selected=(s->game_selected+s->game_count-1)%s->game_count;
        if(input==BADGE_DOWN) s->game_selected=(s->game_selected+1)%s->game_count;
        if(input==BADGE_OK) {s->playing_return=BADGE_GAMES;s->page=BADGE_PLAYING;return BADGE_LAUNCH;}
        return BADGE_REDRAW;
    }
    if(s->page==BADGE_WIFI)return BADGE_IDLE;
    if(s->page==BADGE_PROFILE) {
        if(input==BADGE_OK)return BADGE_WIFI_TOGGLE;
        return BADGE_IDLE;
    }
    if(s->page==BADGE_SETTINGS){
        if(input==BADGE_UP)s->settings_selected=(s->settings_selected+3)%4;
        if(input==BADGE_DOWN)s->settings_selected=(s->settings_selected+1)%4;
        if(input==BADGE_OK){
            static const badge_page_t pages[]={BADGE_DISPLAY_SETTINGS,BADGE_WIFI,BADGE_SLEEP_SETTINGS,BADGE_ABOUT_SETTINGS};
            s->page=pages[s->settings_selected];s->wifi_return=BADGE_SETTINGS;s->ai_settings_return=BADGE_SETTINGS;
            if(s->page==BADGE_SLEEP_SETTINGS)s->timeout_selected=s->screen_timeout;
        }
        return BADGE_REDRAW;
    }
    return BADGE_IDLE;
}
