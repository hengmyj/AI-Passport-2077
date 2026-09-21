#include "xiaozhi_face_variants.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    xz_face_selector_t s={0};
    for(unsigned e=0;e<XZ_FACE_COUNT;e++){
        unsigned last=99,seen=0,cycle=0;
        for(unsigned turn=0;turn<30;turn++){
            xz_face_new_turn(&s);unsigned v=xz_face_select(&s,(xz_face_t)e,turn);
            assert(v<XZ_FACE_VARIANTS&&v!=last);last=v;seen|=1u<<v;cycle|=1u<<v;
            if(turn%XZ_FACE_VARIANTS==XZ_FACE_VARIANTS-1){assert(cycle==7);cycle=0;}
            /* Network duplicates and different entropy do not change a held pose. */
            for(unsigned repeat=0;repeat<10;repeat++)assert(xz_face_select(&s,(xz_face_t)e,repeat+999)==v);
        }
        assert(seen==7);
        for(unsigned v=0;v<XZ_FACE_VARIANTS;v++){
            const xz_face_pose_t*p=xz_face_pose((xz_face_t)e,v);assert(p->eyes<XZ_EYE_COUNT&&p->mouth<XZ_FACE_COUNT);
            const xz_face_pose_t*human=xz_portrait_pose((xz_face_t)e,v);
            assert(human->eyes<XZ_EYE_COUNT&&human->mouth<XZ_FACE_COUNT);
            assert(xz_face_pose_mouth(XZ_SPEAKING,(xz_face_t)e,v,80,1100,1000)==XZ_FACE_COUNT+2);
            assert(xz_face_pose_mouth(XZ_SPEAKING,(xz_face_t)e,v,80,1100,1100)==p->mouth);
            assert(xz_face_pose_mouth(XZ_READY,(xz_face_t)e,v,80,1100,1000)==p->mouth);
            assert(xz_face_pose_mouth(XZ_SPEAKING,(xz_face_t)e,v,80,40,UINT32_MAX-20)==XZ_FACE_COUNT+2);
            assert(xz_face_pose_mouth(XZ_SPEAKING,(xz_face_t)e,v,80,40,50)==p->mouth);
        }
    }
    xz_face_new_turn(&s);unsigned first=xz_face_select(&s,XZ_FACE_HAPPY,0);
    xz_face_select(&s,XZ_FACE_SAD,1);assert(xz_face_select(&s,XZ_FACE_HAPPY,0)!=first);
    assert(xz_face_pose((xz_face_t)999,999)==xz_face_pose(XZ_FACE_NEUTRAL,0));
    assert(xz_portrait_pose((xz_face_t)999,999)==xz_portrait_pose(XZ_FACE_NEUTRAL,0));
    assert(xz_portrait_pose(XZ_FACE_ANGRY,2)->eyes==XZ_FACE_ANGRY);
    assert(xz_portrait_pose(XZ_FACE_THINKING,2)->eyes==XZ_FACE_NEUTRAL);
    assert(xz_portrait_pose(XZ_FACE_DELICIOUS,1)->mouth==XZ_FACE_DELICIOUS);
    assert(xz_roadking_pose(XZ_FACE_NEUTRAL,0)==xz_portrait_pose(XZ_FACE_ANGRY,0));
    assert(xz_roadking_pose(XZ_FACE_NEUTRAL,1)==xz_portrait_pose(XZ_FACE_ANGRY,1));
    assert(xz_roadking_pose(XZ_FACE_NEUTRAL,2)==xz_portrait_pose(XZ_FACE_CONFIDENT,0));
    assert(xz_roadking_pose((xz_face_t)999,999)==xz_roadking_pose(XZ_FACE_NEUTRAL,0));
    for(unsigned e=1;e<XZ_FACE_COUNT;e++)for(unsigned v=0;v<XZ_FACE_VARIANTS;v++){
        if(v==2&&(e==XZ_FACE_LAUGHING||e==XZ_FACE_KISSY)){
            assert(xz_roadking_pose((xz_face_t)e,v)->eyes==XZ_FACE_NEUTRAL);
            assert(xz_roadking_pose((xz_face_t)e,v)->mouth==e);
        }else if(v==1&&e==XZ_FACE_KISSY){
            assert(xz_roadking_pose((xz_face_t)e,v)->eyes==XZ_FACE_HAPPY);
            assert(xz_roadking_pose((xz_face_t)e,v)->mouth==XZ_FACE_KISSY);
        }else assert(xz_roadking_pose((xz_face_t)e,v)==xz_portrait_pose((xz_face_t)e,v));
    }
    assert(xz_roadking_eyes(XZ_FACE_NEUTRAL,0,true)==XZ_EYE_ANGRY_PUFF);
    assert(xz_roadking_eyes(XZ_FACE_NEUTRAL,2,false)==XZ_FACE_CONFIDENT);
    assert(xz_portrait_pose(XZ_FACE_NEUTRAL,0)->eyes==XZ_FACE_NEUTRAL);
    puts("21 x 3 poses: no consecutive repetition, stable duplicate tags, valid assets, speech expiry/interruption/wraparound PASS");
}
