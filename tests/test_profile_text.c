#include "profile_text.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static const uint8_t bits[25]={255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
static bool lookup(uint32_t cp,profile_glyph_t *g){if(cp==0x10ffff)return false;*g=(profile_glyph_t){bits,cp=='.'?3:14,14,cp=='.'?4:14,0,0};return true;}
int main(void){
    static uint8_t img[90000];profile_text_patch_t p;
    assert(profile_text_prepare(&p,2,"产品设计师",0x1234,0xabcd,lookup));
    memset(img,0x55,sizeof(img));for(size_t i=0;i<sizeof(img);i+=512){size_t n=sizeof(img)-i;if(n>512)n=512;profile_text_apply(i,img+i,n,&p);}
    unsigned ink=0;for(size_t i=0;i<sizeof(img);i+=2){size_t y=i/2/84;if(y<90||y>=122)assert(img[i]==0x55&&img[i+1]==0x55);else{assert((img[i]==0x34&&img[i+1]==0x12)||(img[i]==0xcd&&img[i+1]==0xab));if(img[i]==0xcd)ink++;}}
    assert(ink>0);assert(!profile_text_prepare(&p,0,"",0,1,lookup));assert(!profile_text_prepare(&p,0,"   ",0,1,lookup));
    assert(!profile_text_prepare(&p,0,"\xc0\x80",0,1,lookup));assert(!profile_text_prepare(&p,0,"\xe4",0,1,lookup));assert(!profile_text_prepare(&p,1,"\xed\xa0\x80",0,1,lookup));
    assert(!profile_text_prepare(&p,2,"\xf4\x8f\xbf\xbf",0,1,lookup));
    assert(profile_text_prepare(&p,1,"",0,1,lookup));assert(profile_text_prepare(&p,0,"abcdefghijklmnopqrst",0,1,lookup));
    assert(p.count==14&&p.glyphs[13].width==3); /* Two rows, ellipsis fits. */
    puts("Profile text PASS: UTF-8 rejection, two lines, ellipsis, pixel region isolation");
}
