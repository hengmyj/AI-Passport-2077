#include "xiaozhi_face_rle.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    /* Three rows: transparent, mixed alpha, solid. */
    uint8_t data[]={8,0,10,0,16,0,18,0, 8,0, 2,0,3,128,3,255, 8,255};
    uint8_t out[10];memset(out,77,sizeof(out));
    assert(xz_face_rle_row(data,sizeof(data),8,3,1,1,5,out+1));
    const uint8_t expected[]={77,0,128,128,128,255,77,77,77,77};
    assert(!memcmp(out,expected,sizeof(out)));
    assert(xz_face_rle_row(data,sizeof(data),8,3,2,0,8,out));
    for(unsigned i=0;i<8;i++)assert(out[i]==255);
    assert(xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    for(unsigned i=0;i<8;i++)assert(out[i]==0);
    assert(!xz_face_rle_row(data,7,8,3,0,0,8,out));
    assert(!xz_face_rle_row(data,sizeof(data),8,3,3,0,8,out));
    assert(!xz_face_rle_row(data,sizeof(data),8,3,0,7,2,out));
    assert(!xz_face_rle_row(data,sizeof(data),8,3,0,9,0,out));
    data[8]=0;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    data[8]=9;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    data[8]=7;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    data[8]=8;data[2]=19;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    data[2]=9;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    data[0]=0;assert(!xz_face_rle_row(data,sizeof(data),8,3,0,0,8,out));
    puts("Face lossless row decoder: clipped/random rows and invalid streams PASS");
}
