#include "badge_reminder.h"
#include <string.h>
bool badge_reminder_set(badge_reminder_t *r,uint64_t now,unsigned seconds,const char *text){
    if(r->deadline||r->ringing||seconds<1||seconds>86400||!text||!text[0]||strlen(text)>96)return false;
    r->deadline=now+(uint64_t)seconds*1000000;strcpy(r->text,text);return true;
}
unsigned badge_reminder_remaining(const badge_reminder_t *r,uint64_t now){return r->deadline>now?(unsigned)((r->deadline-now+999999)/1000000):0;}
bool badge_reminder_due(const badge_reminder_t *r,uint64_t now){return r->deadline&&now>=r->deadline&&!r->ringing;}
void badge_reminder_clear(badge_reminder_t *r){memset(r,0,sizeof(*r));}
