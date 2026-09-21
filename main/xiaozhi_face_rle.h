#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Little-endian row offsets followed by (count, alpha) runs. Random access
 * keeps clipped repaints bounded to visible rows; no whole-image buffer. */
static inline unsigned xz_rle_u16(const uint8_t *p){return p[0]|((unsigned)p[1]<<8);}
static inline bool xz_face_rle_row(const uint8_t *data,size_t size,unsigned w,unsigned h,
                                 unsigned y,unsigned x,unsigned count,uint8_t *out){
    size_t table=2u*(h+1u);
    if(!data||!out||!w||!h||y>=h||x>w||count>w-x||size<table)return false;
    unsigned begin=xz_rle_u16(data+2*y),end=xz_rle_u16(data+2*y+2);
    if(begin<table||end<begin||end>size||((end-begin)&1u))return false;
    unsigned at=0;
    for(unsigned i=begin;i<end;i+=2){
        unsigned n=data[i];if(!n||n>w-at)return false;
        unsigned left=at>x?at:x,right=at+n<x+count?at+n:x+count;
        if(right>left)memset(out+left-x,data[i+1],right-left);
        at+=n;
    }
    return at==w;
}
