#include "xiaozhi_buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void){
    size_t offset,bytes;
    assert(!xz_audio_arena_layout(0,1,&offset,&bytes));
    assert(!xz_audio_arena_layout(1,SIZE_MAX,&offset,&bytes));
    assert(!xz_audio_arena_layout(1,SIZE_MAX-7,&offset,&bytes));
    assert(xz_audio_arena_layout(24524,18228,&offset,&bytes)&&bytes==24524&&offset%8==0);
    assert(offset>=18228&&offset+sizeof(xz_audio_buffer_t)<=bytes);
    /* Decoder + queue wins if a future codec changes sizes. */
    assert(xz_audio_arena_layout(128,257,&offset,&bytes)&&offset==264&&bytes==264+sizeof(xz_audio_buffer_t));
    uint8_t *arena=malloc(bytes+16);assert(arena);memset(arena,0x5a,bytes+16);
    xz_audio_buffer_t *shared=(xz_audio_buffer_t *)(arena+offset);
    uint8_t frame[1500]={0x18},decoded[1500];
    for(unsigned turn=0;turn<100;turn++){
        memset(shared,0,sizeof(*shared));
        for(unsigned i=0;i<30;i++){
            assert(xz_audio_push(shared,frame,sizeof(frame),i));
            assert(xz_audio_pop(shared,decoded,sizeof(decoded))==sizeof(frame));
            assert(!memcmp(frame,decoded,sizeof(frame)));
        }
        for(size_t i=0;i<257;i++)assert(arena[i]==0x5a);
        for(size_t i=bytes;i<bytes+16;i++)assert(arena[i]==0x5a);
    }
    free(arena);
    uint8_t p[1500]={0x18},out[1500]; // SILK 60 ms
    assert(xz_opus_samples(p,1)==960);
    p[0]=0x80;assert(xz_opus_samples(p,1)==40); // CELT 2.5 ms
    p[0]=0x19;assert(xz_opus_samples(p,2)==1920); // 120 ms
    p[0]=0x1b;p[1]=3;assert(!xz_opus_samples(p,2));
    p[0]=0x18;
    xz_audio_buffer_t q={0};
    assert(xz_audio_push(&q,p,100,0));assert(!xz_audio_ready(&q,100000,false));
    assert(xz_audio_push(&q,p,100,60000));assert(!xz_audio_ready(&q,110000,false));
    assert(xz_audio_push(&q,p,100,120000));assert(xz_audio_ready(&q,120000,false));
    for(unsigned i=0;i<3;i++)assert(xz_audio_pop(&q,out,sizeof(out))==100&&!memcmp(out,p,100));
    assert(!xz_audio_ready(&q,200000,false));
    assert(xz_audio_push(&q,p,100,210000));assert(!xz_audio_ready(&q,220000,false));
    assert(xz_audio_ready(&q,220000,true)); // short final answer
    assert(xz_audio_pop(&q,out,99)==0&&q.count==1);assert(xz_audio_pop(&q,out,1500)==100);
    assert(!xz_audio_ready(&q,500000,false));assert(xz_audio_push(&q,p,100,510000));
    assert(!xz_audio_ready(&q,809999,false));assert(xz_audio_ready(&q,810000,false));
    assert(xz_audio_pop(&q,out,1500)==100);
    // Repeated wrapping must retain exact packet bytes and order.
    for(unsigned i=0;i<1000;i++){
        p[2]=(uint8_t)i;assert(xz_audio_push(&q,p,777,i));assert(xz_audio_pop(&q,out,1500)==777);
        assert(!memcmp(out,p,777)&&!q.used&&!q.samples);
    }
    assert(xz_audio_push(&q,p,1500,0));assert(xz_audio_push(&q,p,1500,0));
    unsigned before=q.used;assert(!xz_audio_push(&q,p,1500,0)&&q.used==before);
    assert(!xz_audio_room(&q)&&xz_audio_ready(&q,0,false));
    assert(xz_audio_pop(&q,out,1500)==1500&&xz_audio_pop(&q,out,1500)==1500);
    puts("XiaoZhi jitter buffer: prefill, short tails, starvation, wrap and overflow PASS");
}
