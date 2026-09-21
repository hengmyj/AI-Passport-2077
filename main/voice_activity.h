#pragma once
#include <stdatomic.h>
#include <stdbool.h>
/* A completed/cancelled request must not clear a newer queued request. */
typedef struct { atomic_bool ready; atomic_uint pending; } voice_activity_t;
static inline void voice_activity_init(voice_activity_t *s){atomic_store(&s->ready,false);atomic_store(&s->pending,0);}
static inline bool voice_activity_busy(voice_activity_t *s){return !atomic_load(&s->ready)||atomic_load(&s->pending)!=0;}
static inline void voice_activity_submit(voice_activity_t *s,unsigned ticket){atomic_store(&s->pending,ticket);}
static inline void voice_activity_complete(voice_activity_t *s,unsigned ticket){atomic_compare_exchange_strong(&s->pending,&ticket,0);}
