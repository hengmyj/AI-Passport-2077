#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
/* Suppress whole service progress markers/standalone format placeholders.
 * Ordinary percentages and sentences explaining code remain untouched. */
static inline bool xz_caption_is_tool_progress(const char *text){
    if(!text)return false;
    const char *p=text;while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')p++;
    if(*p++!='%')return false;
    while(*p==' '||*p=='\t')p++;
    if(*p=='s'){
        const char *end=p+1;
        if(!strncmp(end,"...",3)||!strncmp(end,"…",3))end+=3;
        while(*end==' '||*end=='\t'||*end=='\r'||*end=='\n')end++;
        if(!*end)return true;
    }
    if(strncmp(p,"self.",5))return false;
    p+=5;
    unsigned part=0,total=0;
    while(*p&&total++<128){
        if(part&&(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')){
            while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')p++;
            return !*p;
        }
        if(part&&(!strncmp(p,"...",3)||!strncmp(p,"…",3))){p+=3;while(*p==' '||*p=='\t'||*p=='\r'||*p=='\n')p++;return !*p;}
        if((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='_'){part++;p++;}
        else if(*p=='.'&&part){part=0;p++;}else return false;
    }
    // Some responses omit the trailing ellipsis. Accept only a complete
    // identifier, possibly followed by whitespace; never a prose sentence.
    return part&&!*p;
}
/* Return a complete UTF-8 code point boundary; malformed/truncated input stops. */
static inline size_t xz_caption_next(const char *text,size_t at){
    const unsigned char *p=(const unsigned char *)text+at;
    if(!*p)return at;
    unsigned n=*p<128?1:((*p&0xe0)==0xc0?2:((*p&0xf0)==0xe0?3:((*p&0xf8)==0xf0?4:0)));
    if(!n)return at;
    for(unsigned i=1;i<n;i++)if(!p[i]||(p[i]&0xc0)!=0x80)return at;
    return at+n;
}
static inline unsigned xz_caption_cost(const char *text,size_t at){
    return (unsigned char)text[at]<128?40:90;
}
static inline size_t xz_caption_step(const char *text,size_t at,uint32_t elapsed,uint32_t *spent){
    /* Catch up to the audio clock in one label update, even after a slow frame. */
    for(;;){
        size_t next=xz_caption_next(text,at);
        unsigned cost=xz_caption_cost(text,at);
        if(next==at||elapsed<*spent||elapsed-*spent<cost)return at;
        *spent+=cost;at=next;
    }
}
/* A small reading lead covers PCM/UI scheduling, without animating through a
 * network stall. Sentence text has no server word timestamps. */
#define XZ_CAPTION_LEAD_MS 180u
