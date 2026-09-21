#include "xiaozhi_buffer.h"
bool xz_audio_arena_layout(size_t encoder,size_t decoder,size_t *queue_offset,size_t *bytes){
    const size_t alignment=8; /* int64_t fields in the playback queue */
    if(!encoder||!decoder||!queue_offset||!bytes||decoder>SIZE_MAX-(alignment-1))return false;
    size_t offset=(decoder+alignment-1)&~(alignment-1);
    if(offset>SIZE_MAX-sizeof(xz_audio_buffer_t))return false;
    *queue_offset=offset;
    *bytes=encoder>offset+sizeof(xz_audio_buffer_t)?encoder:offset+sizeof(xz_audio_buffer_t);
    return true;
}
/* RFC 6716 TOC duration, at the decoder's 16 kHz output rate. Full packet
 * validity is still checked by the Opus decoder before PCM reaches I2S. */
unsigned xz_opus_samples(const uint8_t *data,size_t size){
    if(!data||!size)return 0;
    unsigned config=data[0]>>3,code=data[0]&3;
    unsigned samples=config>=16?(40u<<(config&3)):config>=12?(160u<<(config&1)):
                     (config&3)==3?960u:(160u<<(config&3));
    unsigned frames=code==0?1:code==3?(size>=2?(data[1]&63):0):2;
    return frames&&samples*frames<=1920?samples*frames:0;
}
static void put(xz_audio_buffer_t *q,uint8_t v){q->data[q->tail]=v;q->tail=(q->tail+1)%XZ_AUDIO_CAPACITY;}
static uint8_t take(xz_audio_buffer_t *q){uint8_t v=q->data[q->head];q->head=(q->head+1)%XZ_AUDIO_CAPACITY;return v;}
bool xz_audio_room(const xz_audio_buffer_t *q){return !q||XZ_AUDIO_CAPACITY-q->used>=XZ_AUDIO_PACKET_MAX+4;}
bool xz_audio_push(xz_audio_buffer_t *q,const uint8_t *data,size_t size,int64_t now){
    unsigned samples=xz_opus_samples(data,size);
    if(!q||!samples||size>XZ_AUDIO_PACKET_MAX||size+4>XZ_AUDIO_CAPACITY-q->used)return false;
    if(!q->count)q->first_us=now;
    put(q,size&255);put(q,size>>8);put(q,samples&255);put(q,samples>>8);
    for(size_t i=0;i<size;i++)put(q,data[i]);
    q->used+=size+4;q->samples+=samples;q->count++;
    if(q->used>q->peak)q->peak=q->used;
    return true;
}
bool xz_audio_ready(xz_audio_buffer_t *q,int64_t now,bool ended){
    if(!q||!q->count){if(q)q->playing=false;return false;}
    /* 180 ms jitter reserve; short answers drain on stop or after 300 ms.
     * A byte-full queue must drain even with unusually large/short packets. */
    if(q->playing||ended||q->samples>=2880||now-q->first_us>=300000||!xz_audio_room(q))q->playing=true;
    return q->playing;
}
size_t xz_audio_pop(xz_audio_buffer_t *q,uint8_t *out,size_t capacity){
    if(!q||!q->count||!out)return 0;
    unsigned length=q->data[q->head]|((unsigned)q->data[(q->head+1)%XZ_AUDIO_CAPACITY]<<8);
    if(length>capacity)return 0;
    (void)take(q);(void)take(q);unsigned samples=take(q);samples|=(unsigned)take(q)<<8;
    for(unsigned i=0;i<length;i++)out[i]=take(q);
    q->count--;q->used-=length+4;q->samples-=samples;
    return length;
}
