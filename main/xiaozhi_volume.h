#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Worker-owned preference. Coalesce key repeats; never write flash in playback. */
typedef struct { unsigned saved, value; int64_t changed_at, retry_at; } xz_volume_t;
static inline unsigned xz_volume_valid(unsigned value, unsigned fallback) {
    return value <= 100 ? value : (fallback <= 100 ? fallback : 40);
}
static inline void xz_volume_observe(xz_volume_t *v, unsigned value, int64_t now) {
    value=xz_volume_valid(value,40);
    if(value!=v->value){v->value=value;v->changed_at=now;}
}
static inline bool xz_volume_due(const xz_volume_t *v,int64_t now,bool safe,bool flush) {
    return v->saved!=v->value && (flush || (safe && now>=v->retry_at && now-v->changed_at>=1000000));
}
static inline void xz_volume_result(xz_volume_t *v,int64_t now,bool success) {
    if(success)v->saved=v->value;
    v->retry_at=now+3000000;
}
