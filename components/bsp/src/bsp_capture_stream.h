#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*bsp_capture_write_fn)(const uint8_t *data, size_t size, void *context);
typedef struct {
    int width, height, next_y;
    bool failed;
    bsp_capture_write_fn write;
    void *context;
} bsp_capture_stream_t;

/* Full-width, top-to-bottom RGB565 strips; stride padding is never exported.
 * Fail closed on missing, reordered or overlapping strips, or a failed sink. */
static inline bool bsp_capture_stream_strip(bsp_capture_stream_t *s,
        int x1, int y1, int x2, int y2, const uint8_t *data, size_t stride) {
    if (s->failed) return false;
    if (!data || !s->write || s->width <= 0 || s->height <= 0 ||
        x1 != 0 || x2 != s->width - 1 || y1 != s->next_y ||
        y2 < y1 || y2 >= s->height || stride < (size_t)s->width * 2) {
        s->failed = true;
        return false;
    }
    for (int y = y1; y <= y2; ++y) {
        if (!s->write(data + (size_t)(y - y1) * stride,
                      (size_t)s->width * 2, s->context)) {
            s->failed = true;
            return false;
        }
        s->next_y++;
    }
    return true;
}

static inline bool bsp_capture_stream_complete(const bsp_capture_stream_t *s) {
    return !s->failed && s->height > 0 && s->next_y == s->height;
}
