#include "yao_time.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){
    yao_location_t a={.region="Original",.longitude_e6=116407400,.configured=true};
    yao_location_test_ms=0;yao_location_init();
    assert(!yao_location_get().configured&&!yao_location_begin());
    yao_location_network(true);assert(!strcmp(yao_location_status(),"pending"));
    uint32_t first=yao_location_begin();assert(first&&!yao_location_begin());
    assert(!strcmp(yao_location_status(),"fetching"));
    /* Re-entry does not abort a lookup already in progress. */
    yao_location_recheck();assert(yao_location_active(first)&&!yao_location_begin());
    yao_location_finish(first,&a);assert(yao_location_get().configured&&!yao_location_begin());
    assert(!strcmp(yao_location_status(),"ready"));
    /* Fresh longitude survives re-entry, voice pause and Wi-Fi reconnect. */
    yao_location_voice_active(true);yao_location_recheck();
    assert(yao_location_get().configured&&!yao_location_begin());
    yao_location_voice_active(false);yao_location_network(false);
    assert(yao_location_get().configured&&!yao_location_active(first));
    yao_location_network(true);yao_location_recheck();assert(!yao_location_begin());
    yao_location_test_ms=3599999;yao_test_now+=86400; /* NTP jumps do not age the cache. */
    yao_location_recheck();assert(yao_location_get().configured&&!yao_location_begin());
    yao_test_now-=172800;assert(yao_location_get().configured);
    yao_location_test_ms=3600000;assert(!yao_location_get().configured);
    uint32_t next=yao_location_begin();assert(next&&next!=first);
    /* Disconnect invalidates replies but does not permit concurrent TLS workers. */
    yao_location_network(false);yao_location_network(true);assert(!yao_location_begin());
    yao_location_finish(next,&a);assert(!yao_location_get().configured);
    next=yao_location_begin();assert(next);yao_location_finish(next,NULL);
    assert(!strcmp(yao_location_status(),"retrying"));
    next=yao_location_begin();assert(next);yao_location_finish(next,NULL);
    /* A stale completion must not release a newer worker's lease. */
    yao_location_recheck();uint32_t old=yao_location_begin();assert(old);
    yao_location_network(false);yao_location_network(true);
    assert(!yao_location_active(old)&&!yao_location_begin());
    yao_location_finish(old,&a);assert(!yao_location_get().configured);
    next=yao_location_begin();assert(next&&next!=old);
    yao_location_finish(old,&a);assert(!yao_location_begin());
    yao_location_finish(next,&a);assert(yao_location_get().configured);
    /* Each new success starts a new full hour, including while offline. */
    yao_location_test_ms+=3599999;assert(yao_location_get().configured);
    yao_location_network(false);yao_location_test_ms++;
    assert(!yao_location_get().configured&&!strcmp(yao_location_status(),"offline"));
    yao_location_network(true);next=yao_location_begin();a.longitude_e6=180000001;
    yao_location_finish(next,&a);assert(!yao_location_get().configured);
    yao_location_init();assert(!yao_location_get().configured); /* no disk cache on reboot */
    yao_location_network(true);next=yao_location_begin();assert(next&&yao_location_worker_running());
    yao_location_voice_active(true);assert(!yao_location_active(next)&&!yao_location_begin());
    assert(yao_location_worker_running());
    yao_location_finish(next,NULL);assert(!yao_location_worker_running()&&!yao_location_begin());
    yao_location_voice_active(false);next=yao_location_begin();assert(next);
    a.longitude_e6=116407400;yao_location_finish(next,&a);
    assert(!strcmp(yao_location_status(),"ready"));
    puts("Location PASS: one-hour monotonic cache, live-clock independence, reconnect, re-entry, voice resume, stale worker rejection");
}
