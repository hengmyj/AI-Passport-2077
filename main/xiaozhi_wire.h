#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { char host[192],path[384]; unsigned port; bool tls; } xz_endpoint_t;
bool xz_endpoint(const char *url,xz_endpoint_t *out);
bool xz_hex16(const char *text,uint8_t out[16]);
/* Validate lengths before exposing any remotely supplied Opus packet. */
bool xz_ws_audio(unsigned version,const uint8_t *data,size_t size,const uint8_t **audio,size_t *length);
bool xz_udp_audio(const uint8_t *data,size_t size,uint32_t previous,uint32_t *sequence);
void xz_udp_header(uint8_t header[16],size_t length,uint32_t timestamp,uint32_t sequence);
void xz_text_copy(char *out,size_t capacity,const char *text);
#ifdef __cplusplus
}
#endif
