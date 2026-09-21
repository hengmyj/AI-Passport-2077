#include "badge_reminder.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    badge_reminder_t r={0};
    assert(!badge_reminder_due(&r,0));
    assert(!badge_reminder_set(&r,0,0,"test"));
    assert(!badge_reminder_set(&r,0,86401,"test"));
    assert(!badge_reminder_set(&r,0,5,""));
    assert(badge_reminder_set(&r,1234567,10,"喝水"));
    assert(!badge_reminder_set(&r,1234567,20,"overwrite"));
    assert(badge_reminder_remaining(&r,1234567)==10);
    assert(badge_reminder_remaining(&r,11234566)==1);
    assert(!badge_reminder_due(&r,11234566));
    assert(badge_reminder_due(&r,11234567));
    assert(badge_reminder_remaining(&r,11234567)==0);
    r.ringing=true;assert(!badge_reminder_due(&r,999999999));
    badge_reminder_clear(&r);assert(!r.deadline&&!r.ringing&&!r.text[0]);
    assert(!badge_reminder_due(&r,999999999));
    const uint64_t late=UINT64_C(5000000000000);
    assert(badge_reminder_set(&r,late,86400,"long uptime"));
    assert(badge_reminder_remaining(&r,late)==86400);
    assert(!badge_reminder_due(&r,late+UINT64_C(86399999999)));
    assert(badge_reminder_due(&r,late+UINT64_C(86400000000)));
    puts("Reminder PASS: boundaries, duplicate rejection, cancellation, long uptime and once-only ringing");
}
