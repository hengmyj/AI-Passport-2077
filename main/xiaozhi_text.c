#include "xiaozhi_text.h"
#include <stdio.h>
#include <string.h>

size_t xz_utf8_decode(const char *s, uint32_t *cp) {
    const unsigned char *p = (const unsigned char *)s;
    if(!p || !*p) return 0;
    unsigned n = *p < 0x80 ? 1 : *p >= 0xc2 && *p <= 0xdf ? 2 :
                 *p >= 0xe0 && *p <= 0xef ? 3 : *p >= 0xf0 && *p <= 0xf4 ? 4 : 0;
    if(!n) return 0;
    uint32_t c = *p & (n == 1 ? 0x7f : (1u << (7-n))-1);
    for(unsigned i=1; i<n; ++i) {
        if(!p[i] || (p[i]&0xc0)!=0x80) return 0;
        c = (c<<6) | (p[i]&0x3f);
    }
    if((n==2 && c<0x80) || (n==3 && c<0x800) || (n==4 && c<0x10000) ||
       (c>=0xd800 && c<=0xdfff) || c>0x10ffff) return 0;
    *cp=c;
    return n;
}

static const char *emoji(uint32_t c) {
    switch(c) {
    case 0x1f600: case 0x1f601: case 0x1f603: case 0x1f604: case 0x1f606: return "[开心]";
    case 0x1f602: case 0x1f923: return "[大笑]";
    case 0x1f60a: case 0x1f642: case 0x263a: return "[微笑]";
    case 0x1f609: return "[眨眼]";
    case 0x1f60d: case 0x1f970: return "[喜欢]";
    case 0x1f622: case 0x1f62d: return "[哭泣]";
    case 0x1f914: return "[思考]";
    case 0x1f44d: return "[赞]";
    case 0x1f44f: return "[鼓掌]";
    case 0x1f44b: return "[挥手]";
    case 0x1f64f: return "[感谢]";
    case 0x2764: case 0x1f496: case 0x1f497: case 0x1f499: case 0x1f49a: return "[爱心]";
    case 0x1f389: case 0x1f38a: return "[庆祝]";
    case 0x2728: case 0x1f31f: return "[闪亮]";
    case 0x1f525: return "[火焰]";
    case 0x1f4a1: return "[灵感]";
    case 0x1f3b5: case 0x1f3b6: return "[音乐]";
    case 0x2705: case 0x2714: return "[完成]";
    case 0x274c: return "[错误]";
    case 0x26a0: return "[注意]";
    default: return NULL;
    }
}

size_t xz_caption_normalize(char *out,size_t cap,const char *text,bool (*supported)(uint32_t)) {
    if(!cap) return 0;
    size_t used=0;
    const char *p=text ? text : "";
    while(*p) {
        uint32_t cp=0;
        size_t n=xz_utf8_decode(p,&cp), count;
        const char *replacement=NULL;
        char token[16];
        if(!n) { replacement="[编码错误]"; n=1; }
        else if(cp==0x200b || cp==0x200c || cp==0x200d || cp==0xfeff ||
                (cp>=0xfe00 && cp<=0xfe0f) || (cp>=0xe0100 && cp<=0xe01ef) ||
                (cp>=0x1f3fb && cp<=0x1f3ff) || cp=='\r') { p+=n; continue; }
        else if(cp=='\t' || cp==0xa0) replacement=" ";
        else if(cp<32 && cp!='\n') { p+=n; continue; }
        else if((replacement=emoji(cp))!=NULL) { /* Named emoji remain readable. */ }
        else if(cp!='\n' && supported && !supported(cp)) {
            /* Do not silently erase rare names or replace one Han character with another. */
            if(cp>=0x1f000 && cp<=0x1faff) replacement="[表情]";
            else { snprintf(token,sizeof(token),"[U+%04lX]",(unsigned long)cp); replacement=token; }
        }
        count=replacement ? strlen(replacement) : n;
        if(count>=cap-used) break;
        memcpy(out+used,replacement ? replacement : p,count);
        used+=count; p+=n;
    }
    out[used]=0;
    return used;
}
