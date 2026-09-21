#include "profile_text.h"
#include <string.h>
static bool codepoint(const unsigned char **cursor,uint32_t *out){
    const unsigned char *p=*cursor;unsigned n;uint32_t c;
    if(*p<128){n=1;c=*p;}else if(*p>=0xc2&&*p<0xe0){n=2;c=*p&31;}else if(*p>=0xe0&&*p<0xf0){n=3;c=*p&15;}else if(*p>=0xf0&&*p<0xf5){n=4;c=*p&7;}else return false;
    for(unsigned i=1;i<n;i++){if((p[i]&0xc0)!=0x80)return false;c=(c<<6)|(p[i]&63);}
    if(c<32||c==127||(n==2&&c<128)||(n==3&&c<2048)||(n==4&&c<65536)||c>0x10ffff||(c>=0xd800&&c<=0xdfff))return false;
    *cursor=p+n;*out=c;return true;
}
bool profile_text_prepare(profile_text_patch_t *p,unsigned field,const char *text,uint16_t bg,uint16_t fg,profile_glyph_lookup_t lookup){
    if(field>2||!text||strlen(text)>128||!lookup)return false;
    memset(p,0,sizeof(*p));p->top=field==0?0:field==1?44:90;p->height=field==1?34:32;p->bg=bg;p->fg=fg;
    profile_glyph_t decoded[24];unsigned count=0;const unsigned char *s=(const unsigned char *)text;bool nonspace=false;
    while(*s){uint32_t cp;if(count>=(field==0?20u:24u)||!codepoint(&s,&cp)||!lookup(cp,&decoded[count]))return false;if(cp!=32)nonspace=true;count++;}
    if(field==0&&!nonspace)return false;
    unsigned x=0,row=0;bool truncated=false;
    for(unsigned i=0;i<count;i++){
        profile_glyph_t g=decoded[i];if(x+g.advance>84){if(row==1){truncated=true;break;}row++;x=0;}
        g.x+=(int)x;g.y=(int)row*16+14-(int)g.height-g.y;p->glyphs[p->count++]=g;x+=g.advance;
    }
    if(truncated){
        profile_glyph_t dot;if(!lookup('.',&dot))return false;unsigned width=dot.advance*3;
        while(p->count&&x+width>84){profile_glyph_t *g=&p->glyphs[--p->count];x-=g->advance;}
        for(unsigned i=0;i<3;i++){profile_glyph_t g=dot;g.x+=(int)x;g.y=16+14-(int)g.height-g.y;p->glyphs[p->count++]=g;x+=dot.advance;}
    }
    unsigned padding=(p->height-(row+1)*16)/2;for(unsigned i=0;i<p->count;i++)p->glyphs[i].y+=(int)(p->top+padding);
    return true;
}
void profile_text_apply(size_t offset,uint8_t *bytes,size_t size,void *context){
    const profile_text_patch_t *p=context;
    for(size_t i=0;i+1<size;i+=2){size_t pixel=(offset+i)/2;unsigned x=(unsigned)(pixel%84),y=(unsigned)(pixel/84);
        if(y<p->top||y>=p->top+p->height)continue;
        uint16_t color=p->bg;
        for(unsigned n=0;n<p->count;n++){
            const profile_glyph_t *g=&p->glyphs[n];int gx=(int)x-g->x,gy=(int)y-g->y;
            if(gx<0||gy<0||(unsigned)gx>=g->width||(unsigned)gy>=g->height)continue;
            unsigned bit=(unsigned)gy*g->width+(unsigned)gx;if(g->bits[bit/8]&(128u>>(bit%8))){color=p->fg;break;}
        }bytes[i]=(uint8_t)color;bytes[i+1]=(uint8_t)(color>>8);
    }
}
