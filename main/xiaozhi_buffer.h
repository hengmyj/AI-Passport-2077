#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define XZ_AUDIO_CAPACITY 4096u
#define XZ_AUDIO_PACKET_MAX 1500u
/* One owner: the network/audio worker. Lives after the decoder in its arena;
 * encoder and decoder/playback queue are never active at the same time. */
typedef struct {
    uint8_t data[XZ_AUDIO_CAPACITY];
    unsigned head,tail,used,count,samples,peak;
    int64_t first_us;
    bool playing;
} xz_audio_buffer_t;
bool xz_audio_arena_layout(size_t encoder,size_t decoder,size_t *queue_offset,size_t *bytes);
unsigned xz_opus_samples(const uint8_t *data,size_t size);
bool xz_audio_push(xz_audio_buffer_t *q,const uint8_t *data,size_t size,int64_t now);
bool xz_audio_ready(xz_audio_buffer_t *q,int64_t now,bool ended);
size_t xz_audio_pop(xz_audio_buffer_t *q,uint8_t *out,size_t capacity);
bool xz_audio_room(const xz_audio_buffer_t *q);
#ifdef __cplusplus
}
#endif
