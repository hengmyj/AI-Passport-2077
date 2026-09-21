#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct { char region[64]; int32_t longitude_e6; bool configured; } yao_location_t;
/* Solar wall-clock seconds: not a UTC timestamp. East longitude is positive. */
bool yao_solar_time(int64_t utc,double longitude,int64_t *solar,double *equation_minutes);
bool yao_time_valid(int64_t utc);
bool yao_time_format(int64_t seconds,char out[20]);
/* Result-page text, using the cast snapshot rather than the current clock/location. */
void yao_time_summary(int64_t utc,const yao_location_t *location,char *out,size_t size);
int64_t yao_time_now(void);
bool yao_location_valid(const yao_location_t *location);
void yao_location_init(void);
yao_location_t yao_location_get(void);
/* Successful location is cached in RAM for one monotonic hour, including network
 * reconnects. Page re-entry reuses fresh results and never cancels an active lookup. */
void yao_location_network(bool connected);
void yao_location_recheck(void);
uint32_t yao_location_begin(void);
bool yao_location_active(uint32_t generation);
void yao_location_finish(uint32_t generation,const yao_location_t *result);
const char *yao_location_status(void);
/* Called by the network worker; may start a short-lived background task. */
void yao_location_poll(void);
/* Foreground voice joins any HTTP cleanup before allocating its TLS/audio RAM. */
void yao_location_voice_active(bool active);
bool yao_location_worker_running(void);
#ifdef BADGE_CONTROL_HOST_TEST
extern int64_t yao_test_now;
extern int64_t yao_location_test_ms;
#endif
