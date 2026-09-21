#include "bsp_capture_stream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t output[32];
static size_t written, fail_after;
static bool sink(const uint8_t *data, size_t size, void *context) {
    assert(context == output);
    if (written + size > fail_after) return false;
    memcpy(output + written, data, size); written += size; return true;
}
static bsp_capture_stream_t reset(void) {
    written = 0; fail_after = sizeof(output); memset(output, 0, sizeof(output));
    bsp_capture_stream_t s = {.width=2, .height=3, .write=sink, .context=output};
    return s;
}
int main(void) {
    /* Native little-endian red, green, blue, white; omit padded bytes. */
    const uint8_t strip[] = {0x00,0xf8,0xe0,0x07,0xaa,0xbb,0x1f,0x00,0xff,0xff,0xcc,0xdd};
    const uint8_t last[] = {0,0,0xff,0xff};
    const uint8_t expected[] = {0,0xf8,0xe0,7,0x1f,0,0xff,0xff,0,0,0xff,0xff};
    bsp_capture_stream_t s=reset();
    assert(bsp_capture_stream_strip(&s,0,0,1,1,strip,6));
    assert(!bsp_capture_stream_complete(&s));
    assert(bsp_capture_stream_strip(&s,0,2,1,2,last,4));
    assert(bsp_capture_stream_complete(&s) && written==sizeof(expected));
    assert(memcmp(output,expected,sizeof(expected))==0);
    assert(!bsp_capture_stream_strip(&s,0,2,1,2,last,4)); /* duplicate */
    s=reset(); assert(!bsp_capture_stream_strip(&s,0,1,1,1,strip,6)); /* gap */
    s=reset(); assert(!bsp_capture_stream_strip(&s,1,0,1,1,strip,6)); /* partial width */
    s=reset(); assert(!bsp_capture_stream_strip(&s,0,0,1,1,strip,3)); /* short stride */
    s=reset(); assert(!bsp_capture_stream_strip(&s,0,0,1,3,strip,6)); /* bounds */
    s=reset(); assert(!bsp_capture_stream_strip(&s,0,0,1,0,NULL,6));
    s=reset(); fail_after=4;
    assert(!bsp_capture_stream_strip(&s,0,0,1,1,strip,6));
    assert(written==4 && !bsp_capture_stream_complete(&s));
    assert(!bsp_capture_stream_strip(&s,0,1,1,1,strip,6) && written==4);
    s=reset(); assert(bsp_capture_stream_strip(&s,0,0,1,1,strip,6));
    assert(bsp_capture_stream_strip(&s,0,2,1,2,last,4));
    assert(bsp_capture_stream_complete(&s)); /* fresh request after failure */
    puts("Capture stream: pixel order, padding, truncation, malformed strips, sink failure and retry PASS");
    return 0;
}
