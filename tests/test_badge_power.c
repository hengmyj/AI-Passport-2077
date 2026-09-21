#include "badge_power_logic.h"
#include <assert.h>
#include <stdio.h>

int main(void){
    const uint32_t times[]={0,30000,60000,120000,300000};
    for(unsigned i=0;i<5;i++){
        assert(badge_screen_timeout_ms(i)==times[i]);
        if(i){assert(!badge_idle_due(times[i]-1,0,times[i],false));assert(badge_idle_due(times[i],0,times[i],false));}
        assert(!badge_idle_due(900000,0,times[i],true));
    }
    assert(!badge_idle_due(900000,0,badge_screen_timeout_ms(0),false));
    assert(badge_screen_timeout_ms(99)==60000);
    assert(!badge_idle_due(59999,0,BADGE_SCREEN_IDLE_MS,false));
    assert(badge_idle_due(60000,0,BADGE_SCREEN_IDLE_MS,false));
    assert(!badge_idle_due(120000,0,BADGE_SCREEN_IDLE_MS,true));
    assert(!badge_idle_due(14999,0,BADGE_AUDIO_IDLE_MS,false));
    assert(badge_idle_due(15000,0,BADGE_AUDIO_IDLE_MS,false));
    assert(!badge_idle_due(60000,55000,BADGE_SCREEN_IDLE_MS,false));
    assert(!badge_idle_due(60000,60120,BADGE_AUDIO_IDLE_MS,false)); // wake finished later
    assert(!badge_idle_due(60000,60001,BADGE_SCREEN_IDLE_MS,false)); // concurrent key
    uint32_t before=UINT32_MAX-30000;
    assert(!badge_idle_due(29998,before,BADGE_SCREEN_IDLE_MS,false));
    assert(badge_idle_due(29999,before,BADGE_SCREEN_IDLE_MS,false));
    badge_wake_filter_t f={0};
    assert(!badge_wake_consumed(&f,100,false,true,false));
    assert(badge_wake_consumed(&f,60000,true,true,false)); // press wakes
    assert(badge_wake_consumed(&f,61500,false,false,false)); // long press swallowed
    assert(badge_wake_consumed(&f,61600,false,false,true)); // release
    assert(badge_wake_consumed(&f,61800,false,false,false)); // delayed click
    assert(!badge_wake_consumed(&f,62300,false,true,false)); // next gesture works
    f=(badge_wake_filter_t){0};
    assert(badge_wake_consumed(&f,UINT32_MAX-100,true,true,false));
    assert(badge_wake_consumed(&f,0,false,false,true));
    assert(badge_wake_consumed(&f,200,false,false,false));
    assert(!badge_wake_consumed(&f,600,false,true,false));
    f=(badge_wake_filter_t){0};
    assert(badge_wake_consumed(&f,1000,true,true,false));
    assert(badge_wake_consumed(&f,1150,false,false,true));
    assert(badge_wake_consumed(&f,2200,false,false,false)); // delayed by UI restore
    assert(!badge_wake_consumed(&f,2400,false,true,false));
    f=(badge_wake_filter_t){0};
    assert(badge_wake_consumed(&f,1000,true,true,false));
    assert(badge_wake_consumed(&f,3000,false,false,true)); // long wake; no click follows
    assert(!badge_wake_consumed(&f,3500,false,true,false));
    puts("Power idle deadlines, active-audio protection, wraparound and wake gesture: PASS");
    return 0;
}
