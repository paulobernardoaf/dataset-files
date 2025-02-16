#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vlc_common.h>
#include <vlc_md5.h>
typedef uint32_t u32;
typedef uint8_t byte;
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
typedef struct md5_s MD5_CONTEXT;
static void
md5_init( void *context )
{
MD5_CONTEXT *ctx = context;
ctx->A = 0x67452301;
ctx->B = 0xefcdab89;
ctx->C = 0x98badcfe;
ctx->D = 0x10325476;
ctx->nblocks = 0;
ctx->count = 0;
}
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))
static void
transform ( MD5_CONTEXT *ctx, const unsigned char *data )
{
u32 correct_words[16];
register u32 A = ctx->A;
register u32 B = ctx->B;
register u32 C = ctx->C;
register u32 D = ctx->D;
u32 *cwp = correct_words;
#if defined(WORDS_BIGENDIAN)
{
int i;
byte *p2, *p1;
for(i=0, p1=data, p2=(byte*)correct_words; i < 16; i++, p2 += 4 )
{
p2[3] = *p1++;
p2[2] = *p1++;
p2[1] = *p1++;
p2[0] = *p1++;
}
}
#else
memcpy( correct_words, data, 64 );
#endif
#define OP(a, b, c, d, s, T) do { a += FF (b, c, d) + (*cwp++) + T; a = rol(a, s); a += b; } while (0)
OP (A, B, C, D, 7, 0xd76aa478);
OP (D, A, B, C, 12, 0xe8c7b756);
OP (C, D, A, B, 17, 0x242070db);
OP (B, C, D, A, 22, 0xc1bdceee);
OP (A, B, C, D, 7, 0xf57c0faf);
OP (D, A, B, C, 12, 0x4787c62a);
OP (C, D, A, B, 17, 0xa8304613);
OP (B, C, D, A, 22, 0xfd469501);
OP (A, B, C, D, 7, 0x698098d8);
OP (D, A, B, C, 12, 0x8b44f7af);
OP (C, D, A, B, 17, 0xffff5bb1);
OP (B, C, D, A, 22, 0x895cd7be);
OP (A, B, C, D, 7, 0x6b901122);
OP (D, A, B, C, 12, 0xfd987193);
OP (C, D, A, B, 17, 0xa679438e);
OP (B, C, D, A, 22, 0x49b40821);
#undef OP
#define OP(f, a, b, c, d, k, s, T) do { a += f (b, c, d) + correct_words[k] + T; a = rol(a, s); a += b; } while (0)
OP (FG, A, B, C, D, 1, 5, 0xf61e2562);
OP (FG, D, A, B, C, 6, 9, 0xc040b340);
OP (FG, C, D, A, B, 11, 14, 0x265e5a51);
OP (FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
OP (FG, A, B, C, D, 5, 5, 0xd62f105d);
OP (FG, D, A, B, C, 10, 9, 0x02441453);
OP (FG, C, D, A, B, 15, 14, 0xd8a1e681);
OP (FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
OP (FG, A, B, C, D, 9, 5, 0x21e1cde6);
OP (FG, D, A, B, C, 14, 9, 0xc33707d6);
OP (FG, C, D, A, B, 3, 14, 0xf4d50d87);
OP (FG, B, C, D, A, 8, 20, 0x455a14ed);
OP (FG, A, B, C, D, 13, 5, 0xa9e3e905);
OP (FG, D, A, B, C, 2, 9, 0xfcefa3f8);
OP (FG, C, D, A, B, 7, 14, 0x676f02d9);
OP (FG, B, C, D, A, 12, 20, 0x8d2a4c8a);
OP (FH, A, B, C, D, 5, 4, 0xfffa3942);
OP (FH, D, A, B, C, 8, 11, 0x8771f681);
OP (FH, C, D, A, B, 11, 16, 0x6d9d6122);
OP (FH, B, C, D, A, 14, 23, 0xfde5380c);
OP (FH, A, B, C, D, 1, 4, 0xa4beea44);
OP (FH, D, A, B, C, 4, 11, 0x4bdecfa9);
OP (FH, C, D, A, B, 7, 16, 0xf6bb4b60);
OP (FH, B, C, D, A, 10, 23, 0xbebfbc70);
OP (FH, A, B, C, D, 13, 4, 0x289b7ec6);
OP (FH, D, A, B, C, 0, 11, 0xeaa127fa);
OP (FH, C, D, A, B, 3, 16, 0xd4ef3085);
OP (FH, B, C, D, A, 6, 23, 0x04881d05);
OP (FH, A, B, C, D, 9, 4, 0xd9d4d039);
OP (FH, D, A, B, C, 12, 11, 0xe6db99e5);
OP (FH, C, D, A, B, 15, 16, 0x1fa27cf8);
OP (FH, B, C, D, A, 2, 23, 0xc4ac5665);
OP (FI, A, B, C, D, 0, 6, 0xf4292244);
OP (FI, D, A, B, C, 7, 10, 0x432aff97);
OP (FI, C, D, A, B, 14, 15, 0xab9423a7);
OP (FI, B, C, D, A, 5, 21, 0xfc93a039);
OP (FI, A, B, C, D, 12, 6, 0x655b59c3);
OP (FI, D, A, B, C, 3, 10, 0x8f0ccc92);
OP (FI, C, D, A, B, 10, 15, 0xffeff47d);
OP (FI, B, C, D, A, 1, 21, 0x85845dd1);
OP (FI, A, B, C, D, 8, 6, 0x6fa87e4f);
OP (FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
OP (FI, C, D, A, B, 6, 15, 0xa3014314);
OP (FI, B, C, D, A, 13, 21, 0x4e0811a1);
OP (FI, A, B, C, D, 4, 6, 0xf7537e82);
OP (FI, D, A, B, C, 11, 10, 0xbd3af235);
OP (FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
OP (FI, B, C, D, A, 9, 21, 0xeb86d391);
ctx->A += A;
ctx->B += B;
ctx->C += C;
ctx->D += D;
}
static void
md5_write( void *context, const void *inbuf_arg , size_t inlen)
{
const unsigned char *inbuf = inbuf_arg;
MD5_CONTEXT *hd = context;
if( hd->count == 64 ) 
{
transform( hd, hd->buf );
hd->count = 0;
hd->nblocks++;
}
if( !inbuf )
return;
if( hd->count )
{
for( ; inlen && hd->count < 64; inlen-- )
hd->buf[hd->count++] = *inbuf++;
md5_write( hd, NULL, 0 );
if( !inlen )
return;
}
while( inlen >= 64 )
{
transform( hd, inbuf );
hd->count = 0;
hd->nblocks++;
inlen -= 64;
inbuf += 64;
}
for( ; inlen && hd->count < 64; inlen-- )
hd->buf[hd->count++] = *inbuf++;
}
static void
md5_final( void *context)
{
MD5_CONTEXT *hd = context;
u32 t, msb, lsb;
byte *p;
md5_write(hd, NULL, 0); ;
t = hd->nblocks;
lsb = t << 6;
msb = t >> 26;
t = lsb;
if( (lsb += hd->count) < t )
msb++;
t = lsb;
lsb <<= 3;
msb <<= 3;
msb |= t >> 29;
if( hd->count < 56 ) 
{
hd->buf[hd->count++] = 0x80; 
while( hd->count < 56 )
hd->buf[hd->count++] = 0; 
}
else 
{
hd->buf[hd->count++] = 0x80; 
while( hd->count < 64 )
hd->buf[hd->count++] = 0;
md5_write(hd, NULL, 0); ;
memset(hd->buf, 0, 56 ); 
}
hd->buf[56] = lsb ;
hd->buf[57] = lsb >> 8;
hd->buf[58] = lsb >> 16;
hd->buf[59] = lsb >> 24;
hd->buf[60] = msb ;
hd->buf[61] = msb >> 8;
hd->buf[62] = msb >> 16;
hd->buf[63] = msb >> 24;
transform( hd, hd->buf );
p = hd->buf;
#if defined(WORDS_BIGENDIAN)
#define X(a) do { *p++ = hd->a ; *p++ = hd->a >> 8; *p++ = hd->a >> 16; *p++ = hd->a >> 24; } while(0)
#else 
#define X(a) do { *(u32*)p = (*hd).a ; p += 4; } while(0)
#endif
X(A);
X(B);
X(C);
X(D);
#undef X
}
#if 0
static byte *
md5_read( void *context )
{
MD5_CONTEXT *hd = (MD5_CONTEXT *) context;
return hd->buf;
}
#endif
void InitMD5( struct md5_s *h )
{
md5_init( h );
}
void AddMD5( struct md5_s *restrict h, const void *data, size_t len )
{
md5_write( h, data, len );
}
void EndMD5( struct md5_s *h )
{
md5_final( h );
}
