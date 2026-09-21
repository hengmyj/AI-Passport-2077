#include "xiaozhi_face_transition.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    xz_face_transition_t t={0};
    assert(xz_face_transition(&t,XZ_THINKING,XZ_FACE_NEUTRAL,100)==XZ_FACE_THINKING);
    assert(xz_face_transition(&t,XZ_THINKING,XZ_FACE_HAPPY,300)==XZ_FACE_THINKING);
    assert(t.until==1000);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_HAPPY,350)==XZ_FACE_THINKING);
    assert(xz_face_mouth(XZ_SPEAKING,85,450,350)==3);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_SAD,999)==XZ_FACE_THINKING);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_SAD,1000)==XZ_FACE_SAD);
    xz_state_t cancels[]={XZ_LISTENING,XZ_READY,XZ_ERROR,XZ_ACTIVATION};
    for(unsigned i=0;i<sizeof(cancels)/sizeof(cancels[0]);i++){
        xz_face_transition(&t,XZ_THINKING,XZ_FACE_NEUTRAL,2000);
        xz_face_transition(&t,cancels[i],XZ_FACE_NEUTRAL,2050);assert(!t.armed);
        assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_HAPPY,2100)==XZ_FACE_HAPPY);
    }
    xz_face_transition(&t,XZ_THINKING,XZ_FACE_NEUTRAL,UINT32_MAX-399);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_HAPPY,499)==XZ_FACE_THINKING);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_HAPPY,500)==XZ_FACE_HAPPY);
    xz_face_transition(&t,XZ_THINKING,XZ_FACE_NEUTRAL,1000);
    assert(xz_face_transition(&t,XZ_THINKING,XZ_FACE_NEUTRAL,9000)==XZ_FACE_THINKING);
    assert(xz_face_transition(&t,XZ_SPEAKING,XZ_FACE_HAPPY,9001)==XZ_FACE_HAPPY);
    puts("Thinking visual hold: deadline, cancellation, wraparound and independent PCM mouth PASS");
}
