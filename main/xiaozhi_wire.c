#include "xiaozhi_wire.h"
#include <string.h>
#include <stdlib.h>
static unsigned be16(const uint8_t *p){return (unsigned)p[0]*256+p[1];}
static uint32_t be32(const uint8_t *p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
bool xz_endpoint(const char *url,xz_endpoint_t *out){
    if(!url||!out)return false;
    memset(out,0,sizeof(*out));
    const char *p;
    if(!strncmp(url,"wss://",6)){p=url+6;out->tls=true;out->port=443;}
    else if(!strncmp(url,"ws://",5)){p=url+5;out->port=80;}
    else return false;
    for(const unsigned char *s=(const void *)p;*s;s++)if(*s<=32||*s>=127||*s=='#'||*s=='@')return false;
    const char *end=strpbrk(p,"/?");if(!end)end=p+strlen(p);
    const char *colon=memchr(p,':',(size_t)(end-p));const char *host_end=colon?colon:end;
    size_t n=(size_t)(host_end-p);if(!n||n>=sizeof(out->host))return false;
    memcpy(out->host,p,n);
    if(colon){unsigned port=0;if(colon+1==end)return false;for(const char *s=colon+1;s<end;s++){if(*s<'0'||*s>'9')return false;port=port*10+(unsigned)(*s-'0');if(port>65535)return false;}if(!port)return false;out->port=port;}
    n=strlen(end);if(n+2>sizeof(out->path))return false;
    if(*end!='/'){strcpy(out->path,"/");}strcat(out->path,end);return true;
}
bool xz_hex16(const char *text,uint8_t out[16]){
    if(!text||strlen(text)!=32)return false;
    for(unsigned i=0;i<16;i++){unsigned v=0;for(unsigned j=0;j<2;j++){char c=text[i*2+j];unsigned d=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:16;if(d>15)return false;v=v*16+d;}out[i]=(uint8_t)v;}return true;
}
bool xz_ws_audio(unsigned version,const uint8_t *p,size_t n,const uint8_t **audio,size_t *length){
    if(!p||!audio||!length){return false;}size_t h=version==2?16:version==3?4:0;
    if(version<1||version>3||n<=h||n-h>1500)return false;
    if(version==2&&(be16(p)!=2||be16(p+2)!=0||be32(p+12)!=n-h))return false;
    if(version==3&&(p[0]!=0||be16(p+2)!=n-h))return false;
    *audio=p+h;*length=n-h;return true;
}
bool xz_udp_audio(const uint8_t *p,size_t n,uint32_t previous,uint32_t *sequence){
    if(!p||!sequence||n<=16||n>1516||p[0]!=1||be16(p+2)!=n-16)return false;
    uint32_t s=be32(p+12);if(s<=previous)return false;*sequence=s;return true;
}
static void put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(24-i*8));}
void xz_udp_header(uint8_t h[16],size_t n,uint32_t time,uint32_t sequence){h[2]=(uint8_t)(n>>8);h[3]=(uint8_t)n;put32(h+8,time);put32(h+12,sequence);}
void xz_text_copy(char *out,size_t capacity,const char *text){
    if(!capacity){return;}size_t used=0;const unsigned char *p=(const void *)(text?text:"");
    while(*p){size_t n=*p<128?1:(*p&0xe0)==0xc0?2:(*p&0xf0)==0xe0?3:(*p&0xf8)==0xf0?4:0;if(!n||used+n>=capacity)break;bool valid=true;for(size_t i=1;i<n;i++)if(!p[i]||(p[i]&0xc0)!=0x80){valid=false;break;}if(!valid)break;memcpy(out+used,p,n);used+=n;p+=n;}out[used]=0;
}
