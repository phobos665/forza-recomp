#ifndef FZ_SHA1_H
#define FZ_SHA1_H
#include <stdint.h>
#include <string.h>
static uint32_t fz_sha1_rol(uint32_t x,unsigned n){return (x<<n)|(x>>(32-n));}
/* The title's SHA-1 context has five chaining words, counters, then a
 * 64-byte block at +32. Keep all 80 rounds and the original counter reset. */
static void fz_sha1_block(uint8_t *context,uint32_t *out_a,uint32_t *out_c,uint32_t *out_d)
{
    uint32_t h[5],w[80];memcpy(h,context,sizeof h);
    const uint8_t *p=context+32;
    for(unsigned i=0;i<16;++i)w[i]=((uint32_t)p[4*i]<<24)|((uint32_t)p[4*i+1]<<16)|((uint32_t)p[4*i+2]<<8)|p[4*i+3];
    for(unsigned i=16;i<80;++i)w[i]=fz_sha1_rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4];
    for(unsigned i=0;i<80;++i){
        uint32_t f,k;
        if(i<20){f=(b&c)|(~b&d);k=0x5A827999u;}
        else if(i<40){f=b^c^d;k=0x6ED9EBA1u;}
        else if(i<60){f=(b&c)|(b&d)|(c&d);k=0x8F1BBCDCu;}
        else {f=b^c^d;k=0xCA62C1D6u;}
        uint32_t next=fz_sha1_rol(a,5)+f+e+k+w[i];
        e=d;d=c;c=fz_sha1_rol(b,30);b=a;a=next;
    }
    h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;
    memcpy(context,h,sizeof h);uint32_t zero=0;memcpy(context+28,&zero,4);
    *out_a=h[4];*out_c=h[1];*out_d=c;
}
#endif
