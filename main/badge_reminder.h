#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Main task owns this state; monotonic time survives light sleep, not reboot. */
typedef struct {uint64_t deadline;bool ringing;char text[97];} badge_reminder_t;
bool badge_reminder_set(badge_reminder_t *r,uint64_t now,unsigned seconds,const char *text);
unsigned badge_reminder_remaining(const badge_reminder_t *r,uint64_t now);
bool badge_reminder_due(const badge_reminder_t *r,uint64_t now);
void badge_reminder_clear(badge_reminder_t *r);
