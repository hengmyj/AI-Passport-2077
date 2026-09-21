#pragma once
#include "xiaozhi_logic.h"
#include <string.h>
typedef enum {
    XZ_FACE_NEUTRAL,XZ_FACE_HAPPY,XZ_FACE_LAUGHING,XZ_FACE_FUNNY,
    XZ_FACE_SAD,XZ_FACE_ANGRY,XZ_FACE_CRYING,XZ_FACE_LOVING,
    XZ_FACE_EMBARRASSED,XZ_FACE_SURPRISED,XZ_FACE_SHOCKED,XZ_FACE_THINKING,
    XZ_FACE_WINKING,XZ_FACE_COOL,XZ_FACE_RELAXED,XZ_FACE_DELICIOUS,
    XZ_FACE_KISSY,XZ_FACE_CONFIDENT,XZ_FACE_SLEEPY,XZ_FACE_SILLY,XZ_FACE_CONFUSED,
    XZ_FACE_COUNT,XZ_FACE_BLINK=XZ_FACE_COUNT
} xz_face_t;
static inline xz_face_t xz_face_emotion(const char *name){
    static const char *const names[]={"neutral","happy","laughing","funny","sad","angry","crying","loving","embarrassed","surprised","shocked","thinking","winking","cool","relaxed","delicious","kissy","confident","sleepy","silly","confused"};
    if(name)for(unsigned i=0;i<XZ_FACE_COUNT;i++)if(!strcmp(name,names[i]))return (xz_face_t)i;
    return XZ_FACE_NEUTRAL;
}
/* Latest PCM envelope only. A stale speaking state must never keep the mouth open.
 * Deadlines and UI ticks use the same monotonic LVGL clock, including wraparound. */
static inline unsigned xz_face_mouth(xz_state_t state,unsigned level,uint32_t until,uint32_t now){
    if(state!=XZ_SPEAKING||(int32_t)(until-now)<=0||level<4)return 0;
    return level<25?1:level<60?2:3;
}
static inline xz_face_t xz_face_effective(xz_state_t state,xz_face_t emotion){
    if(state==XZ_ERROR)return XZ_FACE_SAD;
    if(state==XZ_CONNECTING||state==XZ_THINKING)return XZ_FACE_THINKING;
    if(state==XZ_LISTENING)return XZ_FACE_NEUTRAL;
    return (unsigned)emotion<XZ_FACE_COUNT?emotion:XZ_FACE_NEUTRAL;
}
static inline xz_face_t xz_face_eyes(xz_state_t state,xz_face_t emotion,uint32_t now){
    xz_face_t face=xz_face_effective(state,emotion);
    /* Never replace glasses, a deliberate wink, sleepy or expressive eyes. */
    if(state!=XZ_LISTENING&&face==XZ_FACE_NEUTRAL&&now%4400<120)return XZ_FACE_BLINK;
    return face;
}
static inline unsigned xz_face_mouth_frame(xz_state_t state,xz_face_t emotion,unsigned level,uint32_t until,uint32_t now){
    unsigned phase=xz_face_mouth(state,level,until,now);
    return phase?XZ_FACE_COUNT+phase-1:(unsigned)xz_face_effective(state,emotion);
}
