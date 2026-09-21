#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Display China standard time without changing the process-wide timezone.
 * System time is UTC; uninitialized clocks must not display an invented time. */
static inline void badge_clock_format(int64_t utc,char out[6]){
    if(utc<1704067200LL){memcpy(out,"--:--",6);return;}
    unsigned minutes=(unsigned)((utc/60+8*60)%(24*60));
    snprintf(out,6,"%02u:%02u",minutes/60,minutes%60);
}
