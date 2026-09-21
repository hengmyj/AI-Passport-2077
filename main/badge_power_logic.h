#pragma once
#include <stdbool.h>
#include <stdint.h>

#define BADGE_SCREEN_IDLE_MS 60000u
#define BADGE_AUDIO_IDLE_MS 15000u
#define BADGE_SCREEN_TIMEOUT_COUNT 5u
#define BADGE_SCREEN_TIMEOUT_DEFAULT 2u
static inline uint32_t badge_screen_timeout_ms(unsigned choice) {
    static const uint32_t values[]={0,30000,60000,120000,300000};
    return values[choice<BADGE_SCREEN_TIMEOUT_COUNT?choice:BADGE_SCREEN_TIMEOUT_DEFAULT];
}
static inline bool badge_idle_due(uint32_t now,uint32_t last,uint32_t delay,bool busy) {
    uint32_t elapsed=now-last;
    /* An input or wake operation may publish a timestamp newer than a
     * worker's sample. Treat it as fresh activity, not a 49-day timeout. */
    return delay!=0 && !busy && elapsed<UINT32_C(0x80000000) && elapsed>=delay;
}
/* Consume the complete wake gesture, including its delayed click/long press. */
typedef struct {bool held,tail;uint32_t until;} badge_wake_filter_t;
static inline bool badge_wake_consumed(badge_wake_filter_t *f,uint32_t now,
                                      bool asleep,bool press,bool release) {
    if(asleep||f->held) {
        if(press)f->held=true;
        if(release)f->held=false;
        f->tail=press||release||f->held;
        f->until=now+350;
        return true;
    }
    if(f->tail){
        /* Rendering during wake may delay queue processing beyond 350 ms.
         * A trailing click still belongs to the wake gesture. Only a new
         * press after the double-click window starts a new gesture. */
        if(press&&(int32_t)(now-f->until)>=0){f->tail=false;return false;}
        if(press)f->held=true;
        if(release)f->until=now+350;
        if(!press&&!release)f->tail=false;
        return true;
    }
    return false;
}
