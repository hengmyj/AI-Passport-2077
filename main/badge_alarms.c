#include "badge_alarms.h"
#include <string.h>
#include <stdio.h>
#define ALARM_MAGIC 0x414C0001u
static bool leap(unsigned y){return y%4==0&&(y%100!=0||y%400==0);}
static unsigned month_days(unsigned y,unsigned m){static const unsigned d[]={31,28,31,30,31,30,31,31,30,31,30,31};return d[m-1]+(m==2&&leap(y));}
static bool digits(const char *p,unsigned n){for(unsigned i=0;i<n;i++)if(p[i]<'0'||p[i]>'9')return false;return true;}
void badge_alarms_init(badge_alarms_t *b){memset(b,0,sizeof(*b));b->magic=ALARM_MAGIC;b->next_id=1;}
bool badge_alarms_valid(const badge_alarms_t *b){
    if(b->magic!=ALARM_MAGIC||!b->next_id||b->next_id>2147483647u)return false;
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++){
        const badge_alarm_t *a=&b->items[i];if(!a->id)continue;
        if(a->minute>=1440||a->weekdays>127||a->pending>1||(a->due<BADGE_CLOCK_MIN||a->due>=BADGE_CLOCK_MAX||a->last<0||a->last>=BADGE_CLOCK_MAX)||!memchr(a->text,0,sizeof(a->text))||!a->text[0])return false;
        for(unsigned j=0;j<i;j++)if(b->items[j].id==a->id)return false;
    }return true;
}
int64_t badge_alarm_next(int64_t now,unsigned minute,unsigned weekdays){
    if((now<BADGE_CLOCK_MIN||now>=BADGE_CLOCK_MAX-604800)||minute>=1440||weekdays>127)return 0;
    int64_t day=(now+28800)/86400;
    for(unsigned n=0;n<8;n++){
        int64_t d=day+n,t=d*86400+(int64_t)minute*60-28800;
        if(t>now&&(!weekdays||(weekdays&(1u<<((d+3)%7)))))return t;
    }return 0;
}
bool badge_alarm_time(const char *s,unsigned *minute){
    if(!s||strlen(s)!=5||s[2]!=':'||!digits(s,2)||!digits(s+3,2))return false;
    unsigned h=(s[0]-'0')*10+s[1]-'0',m=(s[3]-'0')*10+s[4]-'0';if(h>23||m>59)return false;*minute=h*60+m;return true;
}
bool badge_alarm_date(const char *s,int64_t *day){
    if(!s||strlen(s)!=10||s[4]!='-'||s[7]!='-'||!digits(s,4)||!digits(s+5,2)||!digits(s+8,2))return false;
    unsigned y=(s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+s[3]-'0',m=(s[5]-'0')*10+s[6]-'0',d=(s[8]-'0')*10+s[9]-'0';
    if(y<2024||y>2099||m<1||m>12||d<1||d>month_days(y,m))return false;
    int64_t n=0;for(unsigned i=1970;i<y;i++)n+=leap(i)?366:365;for(unsigned i=1;i<m;i++)n+=month_days(y,i);*day=n+d-1;return true;
}
bool badge_alarm_schedule(int64_t now,unsigned seconds,const char *time,const char *date,unsigned days,int64_t *due,unsigned *minute){
    if((now<BADGE_CLOCK_MIN||now>=BADGE_CLOCK_MAX-604800)||days>127)return false;
    *minute=0;
    if(seconds){if(seconds>86400||(time&&*time)||(date&&*date)||days)return false;*due=now+seconds;return true;}
    if(!badge_alarm_time(time,minute))return false;
    if(date&&*date){int64_t day;if(days||!badge_alarm_date(date,&day))return false;*due=day*86400+(int64_t)*minute*60-28800;return *due>now;}
    *due=badge_alarm_next(now,*minute,days);return *due!=0;
}
bool badge_alarms_add(badge_alarms_t *b,int64_t now,int64_t due,unsigned minute,unsigned days,const char *text,uint32_t *id){
    if(!badge_alarms_valid(b)||(now<BADGE_CLOCK_MIN||now>=BADGE_CLOCK_MAX-604800)||due<=now||due>=BADGE_CLOCK_MAX||minute>=1440||days>127||!text||!*text||strlen(text)>96)return false;
    /* Retried identical requests must not create duplicate reminders. */
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++){badge_alarm_t *a=&b->items[i];if(a->id&&a->due==due&&a->minute==minute&&a->weekdays==days&&!strcmp(a->text,text)){*id=a->id;return true;}}
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++)if(!b->items[i].id){
        uint32_t n=b->next_id;bool collision;
        do{collision=false;for(unsigned j=0;j<BADGE_ALARM_COUNT;j++)if(b->items[j].id==n){collision=true;n++;if(n>2147483647u)n=1;break;}}while(collision);
        b->next_id=n+1;if(b->next_id>2147483647u)b->next_id=1;
        badge_alarm_t *a=&b->items[i];memset(a,0,sizeof(*a));a->id=n;a->due=due;a->minute=(uint16_t)minute;a->weekdays=(uint8_t)days;strcpy(a->text,text);*id=n;return true;
    }return false;
}
bool badge_alarms_cancel(badge_alarms_t *b,uint32_t id){for(unsigned i=0;i<BADGE_ALARM_COUNT;i++)if(b->items[i].id==id&&id){memset(&b->items[i],0,sizeof(b->items[i]));return true;}return false;}
bool badge_alarms_ack(badge_alarms_t *b,uint32_t id){
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++)if(b->items[i].id==id&&id){if(!b->items[i].pending)return false;if(b->items[i].weekdays)b->items[i].pending=0;else memset(&b->items[i],0,sizeof(b->items[i]));return true;}
    return false;
}
bool badge_alarms_poll(badge_alarms_t *b,int64_t now,badge_alarm_t *out){
    for(unsigned i=0;i<BADGE_ALARM_COUNT;i++)if(b->items[i].id&&b->items[i].pending){*out=b->items[i];return true;}
    if((now<BADGE_CLOCK_MIN||now>=BADGE_CLOCK_MAX-604800))return false;
    int found=-1;for(unsigned i=0;i<BADGE_ALARM_COUNT;i++){
        badge_alarm_t *a=&b->items[i];if(!a->id||a->due>now)continue;
        /* Do not replay every missed daily occurrence after a long power-off. */
        if(a->weekdays&&now-a->due>600){a->due=badge_alarm_next(now>a->last?now:a->last,a->minute,a->weekdays);continue;}
        if(found<0||a->due<b->items[found].due)found=(int)i;
    }
    if(found<0)return false;
    badge_alarm_t *a=&b->items[found];a->pending=1;a->last=a->due;
    if(a->weekdays)a->due=badge_alarm_next(now>a->last?now:a->last,a->minute,a->weekdays);
    *out=*a;return true;
}
void badge_alarm_format(int64_t utc,char out[20]){
    if(utc<BADGE_CLOCK_MIN||utc>=BADGE_CLOCK_MAX){strcpy(out,"unsynchronized");return;}
    int64_t day=(utc+28800)/86400;unsigned sec=(unsigned)((utc+28800)%86400),y=1970,m=1;
    while(day>=(leap(y)?366:365)){day-=leap(y)?366:365;y++;}
    while(day>=month_days(y,m)){day-=month_days(y,m);m++;}
    snprintf(out,20,"%04u-%02u-%02u %02u:%02u",y,m,(unsigned)day+1,sec/3600,(sec%3600)/60);
}
