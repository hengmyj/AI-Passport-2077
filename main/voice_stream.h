#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "voice_catalog.h"
typedef bool (*voice_read_fn)(void *context, uint32_t offset, void *data, size_t size);
/* 1: packet, 0: EOF, -1: malformed/read error. Cursor changes only on success. */
int voice_stream_next(voice_read_fn read, void *context, uint32_t start,
                      uint32_t length, uint32_t *cursor, uint8_t packet[1500]);
bool voice_stream_span(uint32_t offset,size_t size,unsigned *partition,uint32_t *local,size_t *count);
