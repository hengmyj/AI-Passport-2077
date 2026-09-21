#pragma once
#include "xiaozhi_face.h"
enum {XZ_FACE_VARIANTS=3,XZ_EYE_COOL_ROUND=XZ_FACE_COUNT+1,XZ_EYE_COOL_SPORT,XZ_EYE_ANGRY_PUFF,XZ_EYE_WINK_LEFT,XZ_EYE_COUNT};
typedef struct {uint8_t eyes,mouth;int8_t eye_x,mouth_x;} xz_face_pose_t;
/* Deliberate mood-preserving combinations, not random cross-emotion parts. */
static inline const xz_face_pose_t *xz_face_pose(xz_face_t emotion,unsigned variant){
    static const xz_face_pose_t poses[XZ_FACE_COUNT][XZ_FACE_VARIANTS]={
        {{XZ_FACE_NEUTRAL,XZ_FACE_NEUTRAL,0,0},{XZ_FACE_HAPPY,XZ_FACE_NEUTRAL,0,0},{XZ_FACE_RELAXED,XZ_FACE_RELAXED,0,0}},
        {{XZ_FACE_HAPPY,XZ_FACE_HAPPY,0,0},{XZ_FACE_WINKING,XZ_FACE_HAPPY,0,0},{XZ_FACE_LAUGHING,XZ_FACE_LAUGHING,0,0}},
        {{XZ_FACE_LAUGHING,XZ_FACE_LAUGHING,0,0},{XZ_FACE_HAPPY,XZ_FACE_LAUGHING,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_LAUGHING,0,0}},
        {{XZ_FACE_FUNNY,XZ_FACE_FUNNY,0,0},{XZ_FACE_WINKING,XZ_FACE_LAUGHING,0,0},{XZ_FACE_LAUGHING,XZ_FACE_SILLY,0,0}},
        {{XZ_FACE_SAD,XZ_FACE_SAD,0,0},{XZ_FACE_RELAXED,XZ_FACE_SAD,0,-2},{XZ_FACE_CRYING,XZ_FACE_EMBARRASSED,0,0}},
        {{XZ_FACE_ANGRY,XZ_FACE_ANGRY,0,0},{XZ_EYE_ANGRY_PUFF,XZ_FACE_ANGRY,0,0},{XZ_FACE_CONFIDENT,XZ_FACE_SAD,0,2}},
        {{XZ_FACE_CRYING,XZ_FACE_CRYING,0,0},{XZ_FACE_SAD,XZ_FACE_CRYING,0,0},{XZ_FACE_RELAXED,XZ_FACE_SAD,0,0}},
        {{XZ_FACE_LOVING,XZ_FACE_LOVING,0,0},{XZ_FACE_KISSY,XZ_FACE_KISSY,0,0},{XZ_FACE_WINKING,XZ_FACE_HAPPY,0,0}},
        {{XZ_FACE_EMBARRASSED,XZ_FACE_EMBARRASSED,0,0},{XZ_FACE_RELAXED,XZ_FACE_NEUTRAL,0,-2},{XZ_FACE_WINKING,XZ_FACE_EMBARRASSED,-2,0}},
        {{XZ_FACE_SURPRISED,XZ_FACE_SURPRISED,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_SURPRISED,0,0},{XZ_FACE_SHOCKED,XZ_FACE_SURPRISED,0,0}},
        {{XZ_FACE_SHOCKED,XZ_FACE_SHOCKED,0,0},{XZ_FACE_SURPRISED,XZ_FACE_SHOCKED,0,0},{XZ_FACE_CONFUSED,XZ_FACE_SHOCKED,0,0}},
        {{XZ_FACE_THINKING,XZ_FACE_THINKING,0,0},{XZ_FACE_CONFUSED,XZ_FACE_THINKING,0,-2},{XZ_FACE_CONFIDENT,XZ_FACE_CONFUSED,-2,0}},
        {{XZ_FACE_WINKING,XZ_FACE_WINKING,0,0},{XZ_FACE_HAPPY,XZ_FACE_HAPPY,0,0},{XZ_FACE_WINKING,XZ_FACE_KISSY,2,0}},
        {{XZ_FACE_COOL,XZ_FACE_COOL,0,0},{XZ_EYE_COOL_ROUND,XZ_FACE_CONFIDENT,0,0},{XZ_EYE_COOL_SPORT,XZ_FACE_WINKING,0,0}},
        {{XZ_FACE_RELAXED,XZ_FACE_RELAXED,0,0},{XZ_FACE_HAPPY,XZ_FACE_RELAXED,0,0},{XZ_FACE_SLEEPY,XZ_FACE_NEUTRAL,0,0}},
        {{XZ_FACE_DELICIOUS,XZ_FACE_DELICIOUS,0,0},{XZ_FACE_WINKING,XZ_FACE_SILLY,0,0},{XZ_FACE_HAPPY,XZ_FACE_DELICIOUS,0,-2}},
        {{XZ_FACE_KISSY,XZ_FACE_KISSY,0,0},{XZ_FACE_LOVING,XZ_FACE_KISSY,0,0},{XZ_EYE_WINK_LEFT,XZ_FACE_KISSY,0,2}},
        {{XZ_FACE_CONFIDENT,XZ_FACE_CONFIDENT,0,0},{XZ_FACE_WINKING,XZ_FACE_CONFIDENT,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_CONFIDENT,2,0}},
        {{XZ_FACE_SLEEPY,XZ_FACE_SLEEPY,0,0},{XZ_FACE_RELAXED,XZ_FACE_SLEEPY,0,0},{XZ_FACE_BLINK,XZ_FACE_RELAXED,0,0}},
        {{XZ_FACE_SILLY,XZ_FACE_SILLY,0,0},{XZ_EYE_WINK_LEFT,XZ_FACE_SILLY,0,0},{XZ_FACE_FUNNY,XZ_FACE_SILLY,0,0}},
        {{XZ_FACE_CONFUSED,XZ_FACE_CONFUSED,0,0},{XZ_FACE_THINKING,XZ_FACE_SURPRISED,0,-2},{XZ_FACE_EMBARRASSED,XZ_FACE_CONFUSED,-2,0}}
    };
    return &poses[(unsigned)emotion<XZ_FACE_COUNT?emotion:XZ_FACE_NEUTRAL][variant<XZ_FACE_VARIANTS?variant:0];
}
/* Human portraits use quieter, coordinated expressions instead of mascot combinations. */
static inline const xz_face_pose_t *xz_portrait_pose(xz_face_t emotion,unsigned variant){
    static const xz_face_pose_t poses[XZ_FACE_COUNT][XZ_FACE_VARIANTS]={
        {{XZ_FACE_NEUTRAL,XZ_FACE_NEUTRAL,0,0},{XZ_FACE_HAPPY,XZ_FACE_NEUTRAL,0,0},{XZ_FACE_RELAXED,XZ_FACE_RELAXED,0,0}},
        {{XZ_FACE_HAPPY,XZ_FACE_HAPPY,0,0},{XZ_FACE_WINKING,XZ_FACE_HAPPY,0,0},{XZ_FACE_LAUGHING,XZ_FACE_LAUGHING,0,0}},
        {{XZ_FACE_LAUGHING,XZ_FACE_LAUGHING,0,0},{XZ_FACE_HAPPY,XZ_FACE_LAUGHING,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_LAUGHING,0,0}},
        {{XZ_FACE_FUNNY,XZ_FACE_FUNNY,0,0},{XZ_FACE_WINKING,XZ_FACE_LAUGHING,0,0},{XZ_FACE_LAUGHING,XZ_FACE_FUNNY,0,0}},
        {{XZ_FACE_SAD,XZ_FACE_SAD,0,0},{XZ_FACE_RELAXED,XZ_FACE_SAD,0,-2},{XZ_FACE_CRYING,XZ_FACE_SAD,0,0}},
        {{XZ_FACE_ANGRY,XZ_FACE_ANGRY,0,0},{XZ_EYE_ANGRY_PUFF,XZ_FACE_ANGRY,0,0},{XZ_FACE_ANGRY,XZ_FACE_SAD,0,0}},
        {{XZ_FACE_CRYING,XZ_FACE_CRYING,0,0},{XZ_FACE_SAD,XZ_FACE_CRYING,0,0},{XZ_FACE_RELAXED,XZ_FACE_SAD,0,0}},
        {{XZ_FACE_LOVING,XZ_FACE_LOVING,0,0},{XZ_FACE_KISSY,XZ_FACE_KISSY,0,0},{XZ_FACE_WINKING,XZ_FACE_HAPPY,0,0}},
        {{XZ_FACE_EMBARRASSED,XZ_FACE_EMBARRASSED,0,0},{XZ_FACE_RELAXED,XZ_FACE_NEUTRAL,0,-2},{XZ_FACE_THINKING,XZ_FACE_EMBARRASSED,-2,0}},
        {{XZ_FACE_SURPRISED,XZ_FACE_SURPRISED,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_SURPRISED,0,0},{XZ_FACE_SHOCKED,XZ_FACE_SURPRISED,0,0}},
        {{XZ_FACE_SHOCKED,XZ_FACE_SHOCKED,0,0},{XZ_FACE_SURPRISED,XZ_FACE_SHOCKED,0,0},{XZ_FACE_CONFUSED,XZ_FACE_SHOCKED,0,0}},
        {{XZ_FACE_THINKING,XZ_FACE_THINKING,0,0},{XZ_FACE_CONFUSED,XZ_FACE_THINKING,0,-2},{XZ_FACE_NEUTRAL,XZ_FACE_THINKING,0,0}},
        {{XZ_FACE_WINKING,XZ_FACE_WINKING,0,0},{XZ_FACE_HAPPY,XZ_FACE_HAPPY,0,0},{XZ_FACE_WINKING,XZ_FACE_KISSY,2,0}},
        {{XZ_FACE_COOL,XZ_FACE_COOL,0,0},{XZ_EYE_COOL_ROUND,XZ_FACE_CONFIDENT,0,0},{XZ_EYE_COOL_SPORT,XZ_FACE_WINKING,0,0}},
        {{XZ_FACE_RELAXED,XZ_FACE_RELAXED,0,0},{XZ_FACE_HAPPY,XZ_FACE_RELAXED,0,0},{XZ_FACE_SLEEPY,XZ_FACE_NEUTRAL,0,0}},
        {{XZ_FACE_DELICIOUS,XZ_FACE_DELICIOUS,0,0},{XZ_FACE_WINKING,XZ_FACE_DELICIOUS,0,0},{XZ_FACE_HAPPY,XZ_FACE_DELICIOUS,0,-2}},
        {{XZ_FACE_KISSY,XZ_FACE_KISSY,0,0},{XZ_FACE_LOVING,XZ_FACE_KISSY,0,0},{XZ_EYE_WINK_LEFT,XZ_FACE_KISSY,0,2}},
        {{XZ_FACE_CONFIDENT,XZ_FACE_CONFIDENT,0,0},{XZ_FACE_WINKING,XZ_FACE_CONFIDENT,0,0},{XZ_FACE_NEUTRAL,XZ_FACE_CONFIDENT,2,0}},
        {{XZ_FACE_SLEEPY,XZ_FACE_SLEEPY,0,0},{XZ_FACE_RELAXED,XZ_FACE_SLEEPY,0,0},{XZ_FACE_BLINK,XZ_FACE_RELAXED,0,0}},
        {{XZ_FACE_SILLY,XZ_FACE_SILLY,0,0},{XZ_EYE_WINK_LEFT,XZ_FACE_SILLY,0,0},{XZ_FACE_FUNNY,XZ_FACE_SILLY,0,0}},
        {{XZ_FACE_CONFUSED,XZ_FACE_CONFUSED,0,0},{XZ_FACE_THINKING,XZ_FACE_THINKING,0,-2},{XZ_FACE_EMBARRASSED,XZ_FACE_CONFUSED,-2,0}}
    };
    return &poses[(unsigned)emotion<XZ_FACE_COUNT?emotion:XZ_FACE_NEUTRAL][variant<XZ_FACE_VARIANTS?variant:0];
}
/* Road King defaults to two angry poses and one confident pose per variant cycle.
 * Explicit emotions still use their own portrait expressions. */
static inline const xz_face_pose_t *xz_roadking_pose(xz_face_t emotion,unsigned variant){
    if((unsigned)emotion>=XZ_FACE_COUNT)emotion=XZ_FACE_NEUTRAL;
    if(variant>=XZ_FACE_VARIANTS)variant=0;
    if(emotion==XZ_FACE_NEUTRAL)
        return variant<2?xz_portrait_pose(XZ_FACE_ANGRY,variant):xz_portrait_pose(XZ_FACE_CONFIDENT,0);
    /* Symmetric eyes remove unilateral-wink differences; keep a distinct
     * open-eyed variant without changing the requested laughing/kissing mouth. */
    static const xz_face_pose_t laugh_open={XZ_FACE_NEUTRAL,XZ_FACE_LAUGHING,0,0};
    static const xz_face_pose_t kiss_open={XZ_FACE_NEUTRAL,XZ_FACE_KISSY,0,0};
    static const xz_face_pose_t kiss_smile={XZ_FACE_HAPPY,XZ_FACE_KISSY,0,0};
    if(variant==2&&emotion==XZ_FACE_LAUGHING)return &laugh_open;
    if(variant==2&&emotion==XZ_FACE_KISSY)return &kiss_open;
    if(variant==1&&emotion==XZ_FACE_KISSY)return &kiss_smile;
    return xz_portrait_pose(emotion,variant);
}
static inline unsigned xz_roadking_eyes(xz_face_t emotion,unsigned variant,bool blink){
    const xz_face_pose_t *pose=xz_roadking_pose(emotion,variant);
    if(!blink)return pose->eyes;
    /* Keep the angry brows during an automatic blink. */
    return pose->eyes==XZ_FACE_ANGRY||pose->eyes==XZ_EYE_ANGRY_PUFF?XZ_EYE_ANGRY_PUFF:XZ_FACE_BLINK;
}
/* Worker-owned, RAM-only history. Zero means no prior choice; stored ids are +1. */
typedef struct {uint8_t last[XZ_FACE_COUNT],remaining[XZ_FACE_COUNT],emotion,variant;bool active;} xz_face_selector_t;
static inline void xz_face_new_turn(xz_face_selector_t *s){s->active=false;}
static inline unsigned xz_face_select(xz_face_selector_t *s,xz_face_t emotion,uint32_t entropy){
    unsigned e=(unsigned)emotion<XZ_FACE_COUNT?(unsigned)emotion:XZ_FACE_NEUTRAL;
    if(s->active&&s->emotion==e)return s->variant;
    unsigned previous=s->last[e],bag=s->remaining[e];
    if(!bag)bag=(1u<<XZ_FACE_VARIANTS)-1;
    unsigned available=previous?bag&~(1u<<(previous-1)):bag,count=0;
    for(unsigned i=0;i<XZ_FACE_VARIANTS;i++)if(available&(1u<<i))count++;
    unsigned ticket=entropy%count,pick=0;
    for(unsigned i=0;i<XZ_FACE_VARIANTS;i++)if(available&(1u<<i)){if(!ticket){pick=i;break;}ticket--;}
    s->remaining[e]=(uint8_t)(bag&~(1u<<pick));
    s->last[e]=(uint8_t)(pick+1);s->emotion=(uint8_t)e;s->variant=(uint8_t)pick;s->active=true;
    return pick;
}
static inline unsigned xz_face_pose_mouth(xz_state_t state,xz_face_t emotion,unsigned variant,unsigned level,uint32_t until,uint32_t now){
    unsigned phase=xz_face_mouth(state,level,until,now);
    return phase?XZ_FACE_COUNT+phase-1:xz_face_pose(xz_face_effective(state,emotion),variant)->mouth;
}
