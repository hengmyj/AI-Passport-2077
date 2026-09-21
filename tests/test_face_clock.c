#include "xiaozhi_face.h"
#include "badge_clock.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    assert(xz_face_emotion("happy")==XZ_FACE_HAPPY);
    assert(xz_face_emotion("crying")==XZ_FACE_CRYING);
    assert(xz_face_emotion("unknown")==XZ_FACE_NEUTRAL);
    assert(xz_face_emotion(NULL)==XZ_FACE_NEUTRAL);
    assert(xz_face_mouth(XZ_SPEAKING,80,1100,1000)==3);
    assert(xz_face_mouth(XZ_SPEAKING,80,1100,1100)==0);
    assert(xz_face_mouth(XZ_SPEAKING,80,1100,6000)==0);
    assert(xz_face_mouth(XZ_SPEAKING,0,1100,1000)==0);
    assert(xz_face_mouth(XZ_READY,80,1100,1000)==0);
    assert(xz_face_mouth(XZ_LISTENING,80,1100,1000)==0);
    assert(xz_face_mouth(XZ_SPEAKING,80,40,UINT32_MAX-20)==3);
    assert(xz_face_mouth(XZ_SPEAKING,80,40,50)==0);
    assert(xz_face_eyes(XZ_LISTENING,XZ_FACE_ANGRY,200)==XZ_FACE_NEUTRAL);
    assert(xz_face_eyes(XZ_THINKING,XZ_FACE_HAPPY,200)==XZ_FACE_THINKING);
    const char *names[]={"neutral","happy","laughing","funny","sad","angry","crying","loving","embarrassed","surprised","shocked","thinking","winking","cool","relaxed","delicious","kissy","confident","sleepy","silly","confused"};
    for(unsigned i=0;i<XZ_FACE_COUNT;i++){
        assert(xz_face_emotion(names[i])==(xz_face_t)i);
        assert(xz_face_eyes(XZ_SPEAKING,(xz_face_t)i,200)==(xz_face_t)i);
        assert(xz_face_mouth_frame(XZ_SPEAKING,(xz_face_t)i,80,1100,1000)==XZ_FACE_COUNT+2);
        assert(xz_face_mouth_frame(XZ_SPEAKING,(xz_face_t)i,80,1100,1100)==i);
        assert(xz_face_mouth_frame(XZ_READY,(xz_face_t)i,80,1100,1000)==i);
    }
    assert(xz_face_eyes(XZ_READY,(xz_face_t)99,200)==XZ_FACE_NEUTRAL);
    assert(xz_face_eyes(XZ_READY,XZ_FACE_COOL,10)==XZ_FACE_COOL);
    assert(xz_face_eyes(XZ_READY,XZ_FACE_WINKING,10)==XZ_FACE_WINKING);
    char b[16];badge_clock_format(0,b);assert(!strcmp(b,"--:--"));
    badge_clock_format(1704067200,b);assert(!strcmp(b,"08:00"));
    badge_clock_format(1704124799,b);assert(!strcmp(b,"23:59"));
    badge_clock_format(1704124800,b);assert(!strcmp(b,"00:00"));
    puts("Face stale envelope, interruption, wraparound and UTC+8 clock checks PASS");
}
