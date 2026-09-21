#include "xiaozhi_text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool supported(uint32_t c) {return c<128 || c==0x4f60 || c==0x597d || c==0x53e3;}
int main(void) {
    char out[256]; uint32_t cp;
    xz_caption_normalize(out,sizeof(out),"你好😊！",supported);
    assert(!strcmp(out,"你好[微笑][U+FF01]"));
    xz_caption_normalize(out,sizeof(out),"口 %s 90%",supported);
    assert(!strcmp(out,"口 %s 90%")); /* An actual Han 'mouth' is not a missing glyph. */
    xz_caption_normalize(out,sizeof(out),"A❤️ B👍🏽 👩‍💻",supported);
    assert(!strcmp(out,"A[爱心] B[赞] [表情][表情]"));
    xz_caption_normalize(out,sizeof(out),"𠮷",supported);
    assert(!strcmp(out,"[U+20BB7]"));
    xz_caption_normalize(out,7,"你好a",supported); assert(!strcmp(out,"你好"));
    xz_caption_normalize(out,5,"a😊",supported); assert(!strcmp(out,"a"));
    xz_caption_normalize(out,1,"你好",supported); assert(!*out);
    out[0]='X';assert(!xz_caption_normalize(out,0,"a",supported));assert(out[0]=='X');
    assert(!xz_utf8_decode("\xc0\xaf",&cp));
    assert(!xz_utf8_decode("\xed\xa0\x80",&cp));
    assert(!xz_utf8_decode("\xf4\x90\x80\x80",&cp));
    assert(!xz_utf8_decode("\xe4\xb8",&cp));
    assert(xz_utf8_decode("\xf4\x8f\xbf\xbf",&cp)==4 && cp==0x10ffff);
    xz_caption_normalize(out,sizeof(out),"a\xff" "b",supported);
    assert(!strcmp(out,"a[编码错误]b"));
    xz_caption_normalize(out,sizeof(out),"a\t\r\nb",supported); assert(!strcmp(out,"a \nb"));
    puts("Caption UTF-8, emoji labels, missing glyph tokens and bounds: PASS");
    return 0;
}
