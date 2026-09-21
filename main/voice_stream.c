#include "voice_stream.h"
#define FIRST VOICE_FIRST_BYTES
#define TOTAL VOICE_TOTAL_BYTES
bool voice_stream_span(uint32_t offset,size_t size,unsigned *partition,uint32_t *local,size_t *count) {
    if(offset>=TOTAL||!size||size>TOTAL-offset)return false;
    *partition=offset>=FIRST;*local=*partition?offset-FIRST:offset;
    size_t available=*partition?TOTAL-offset:FIRST-offset;
    *count=size<available?size:available;return true;
}
int voice_stream_next(voice_read_fn read,void *ctx,uint32_t start,uint32_t length,uint32_t *cursor,uint8_t packet[1500]) {
    if(start>TOTAL||length>TOTAL-start||*cursor>length)return -1;
    if(*cursor==length)return 0;
    uint8_t head[2];
    if(length-*cursor<2||!read(ctx,start+*cursor,head,2))return -1;
    unsigned size=(unsigned)head[0]|((unsigned)head[1]<<8);
    if(!size||size>1500||size>length-*cursor-2)return -1;
    if(!read(ctx,start+*cursor+2,packet,size))return -1;
    *cursor+=size+2;return (int)size;
}
