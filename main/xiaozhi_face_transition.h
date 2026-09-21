#pragma once
#include "xiaozhi_face.h"
enum {XZ_THINKING_MIN_MS=900};
typedef struct {xz_state_t previous;uint32_t until;bool armed;} xz_face_transition_t;
/* Visual hold only: neither PCM, subtitles nor network state waits for this. */
static inline xz_face_t xz_face_transition(xz_face_transition_t *t,xz_state_t state,xz_face_t emotion,uint32_t now){
    if(state==XZ_THINKING&&t->previous!=XZ_THINKING&&!t->armed){t->until=now+XZ_THINKING_MIN_MS;t->armed=true;}
    if(state!=XZ_THINKING&&state!=XZ_SPEAKING)t->armed=false;
    if(state==XZ_SPEAKING&&t->armed&&(int32_t)(t->until-now)<=0)t->armed=false;
    t->previous=state;
    if(t->armed)return XZ_FACE_THINKING;
    return xz_face_effective(state,emotion);
}
