/* Adapted from CyberYAO 3c4780f; see assets/cyberyao/LICENSE. */
#include "yao_core.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
static const uint8_t numbers[64] = {
    2,24,7,19,15,36,46,11,16,51,40,54,62,55,32,34,
    8,3,29,60,39,63,48,5,45,17,47,58,31,49,28,43,
    23,27,4,41,52,22,18,26,35,21,64,38,56,30,50,14,
    20,42,59,61,53,37,57,9,12,25,6,10,33,13,44,1
};
bool yao_full_name(const yao_hexagram_t *h,char *out,size_t cap){
    static const char *const nature[]={"地","雷","水","泽","山","火","风","天"};
    if(out&&cap)out[0]=0;
    if(!h||!out||!cap)return false;
    for(unsigned bits=0;bits<64;bits++)if(numbers[bits]==h->number){
        unsigned lower=bits&7,upper=bits>>3;
        int n=upper==lower?snprintf(out,cap,"%s为%s",h->name,nature[upper]):snprintf(out,cap,"%s%s%s",nature[upper],nature[lower],h->name);
        return n>=0&&(size_t)n<cap;
    }
    return false;
}
uint8_t yao_coin_line(uint8_t bits) {
    return (uint8_t)(6 + (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1));
}
const char *yao_reading_rule(unsigned n) {
    static const char *const rules[] = {
        "无动爻：读本卦卦辞。", "一动爻：读本卦动爻爻辞。",
        "二动爻：读本卦两条动爻，上爻为主。", "三动爻：读本卦和之卦卦辞，本卦为主。",
        "四动爻：读之卦两条不变爻，下爻为主。", "五动爻：读之卦唯一不变爻。",
        "六动爻：乾坤读用九或用六，其余读之卦卦辞。"
    };
    return n < 7 ? rules[n] : "";
}
static void append_text(char *out,size_t cap,size_t *used,const char *format,...){
    va_list args;va_start(args,format);
    size_t remaining=out&&*used<cap?cap-*used:0;
    int n=vsnprintf(remaining?out+*used:NULL,remaining,format,args);va_end(args);
    if(n>0)*used+=(size_t)n;
}
static const char *reading_tag(const yao_result_t *r,const yao_hexagram_t *h,unsigned line){
    for(unsigned i=0;i<r->reading_count;i++)if(r->readings[i].hexagram==h&&r->readings[i].line==line)
        return r->readings[i].primary?"主读 · ":"参读 · ";
    return "";
}
size_t yao_format_full_reading(const yao_result_t *r,char *out,size_t cap){
    if(out&&cap)out[0]=0;
    if(!r||!r->original||!r->changed)return 0;
    size_t used=0;append_text(out,cap,&used,"%s\n\n",yao_reading_rule(r->moving_count));
    for(unsigned which=0;which<(r->moving_count?2u:1u);which++){
        const yao_hexagram_t *h=which?r->changed:r->original;
        char name[32];yao_full_name(h,name,sizeof(name));
        append_text(out,cap,&used,"%s · %s\n%s%s\n\n",which?"之卦":"本卦",name,reading_tag(r,h,0),h->text);
        for(unsigned i=0;i<6;i++)append_text(out,cap,&used,"%s%s\n\n",reading_tag(r,h,i+1),h->lines[i]);
        if(h->special)append_text(out,cap,&used,"%s%s\n\n",reading_tag(r,h,7),h->special);
    }
    return used;
}
static void add(yao_result_t *r,const yao_hexagram_t *h,unsigned line,bool primary) {
    yao_reading_t *p=&r->readings[r->reading_count++];
    *p=(yao_reading_t){h,(uint8_t)line,primary,line==7?h->special:line?h->lines[line-1]:h->text};
}
bool yao_calculate(const uint8_t lines[6],yao_result_t *r) {
    if(!lines||!r)return false;
    memset(r,0,sizeof(*r));unsigned a=0,b=0;
    for(unsigned i=0;i<6;i++) {
        if(lines[i]<6||lines[i]>9)return false;
        r->lines[i]=lines[i];bool moving=lines[i]==6||lines[i]==9;
        bool yang=(lines[i]&1)!=0;
        if(yang)a|=1u<<i;
        if(yang!=moving)b|=1u<<i;
        if(moving){r->moving_mask|=1u<<i;r->moving_count++;}
    }
    r->original=&YAO_HEXAGRAMS[numbers[a]-1];r->changed=&YAO_HEXAGRAMS[numbers[b]-1];
    unsigned n=r->moving_count;
    if(!n)add(r,r->original,0,true);
    else if(n<=2){
        /* Highest moving line is primary; return it first. */
        for(int i=5;i>=0;i--)if(r->moving_mask&(1u<<i))add(r,r->original,i+1,r->reading_count==0);
    }else if(n==3){add(r,r->original,0,true);add(r,r->changed,0,false);}
    else if(n<=5){
        for(unsigned i=0;i<6;i++)if(!(r->moving_mask&(1u<<i)))add(r,r->changed,i+1,r->reading_count==0);
    }else if(r->original->special)add(r,r->original,7,true);
    else add(r,r->changed,0,true);
    return true;
}
