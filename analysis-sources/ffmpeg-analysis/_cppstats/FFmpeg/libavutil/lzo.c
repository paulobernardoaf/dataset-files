#include <string.h>
#include "avutil.h"
#include "avassert.h"
#include "common.h"
#include "intreadwrite.h"
#include "lzo.h"
#define OUTBUF_PADDED 1
#define INBUF_PADDED 1
typedef struct LZOContext {
const uint8_t *in, *in_end;
uint8_t *out_start, *out, *out_end;
int error;
} LZOContext;
static inline int get_byte(LZOContext *c)
{
if (c->in < c->in_end)
return *c->in++;
c->error |= AV_LZO_INPUT_DEPLETED;
return 1;
}
#if defined(INBUF_PADDED)
#define GETB(c) (*(c).in++)
#else
#define GETB(c) get_byte(&(c))
#endif
static inline int get_len(LZOContext *c, int x, int mask)
{
int cnt = x & mask;
if (!cnt) {
while (!(x = get_byte(c))) {
if (cnt >= INT_MAX - 1000) {
c->error |= AV_LZO_ERROR;
break;
}
cnt += 255;
}
cnt += mask + x;
}
return cnt;
}
static inline void copy(LZOContext *c, int cnt)
{
register const uint8_t *src = c->in;
register uint8_t *dst = c->out;
av_assert0(cnt >= 0);
if (cnt > c->in_end - src) {
cnt = FFMAX(c->in_end - src, 0);
c->error |= AV_LZO_INPUT_DEPLETED;
}
if (cnt > c->out_end - dst) {
cnt = FFMAX(c->out_end - dst, 0);
c->error |= AV_LZO_OUTPUT_FULL;
}
#if defined(INBUF_PADDED) && defined(OUTBUF_PADDED)
AV_COPY32U(dst, src);
src += 4;
dst += 4;
cnt -= 4;
if (cnt > 0)
#endif
memcpy(dst, src, cnt);
c->in = src + cnt;
c->out = dst + cnt;
}
static inline void copy_backptr(LZOContext *c, int back, int cnt)
{
register uint8_t *dst = c->out;
av_assert0(cnt > 0);
if (dst - c->out_start < back) {
c->error |= AV_LZO_INVALID_BACKPTR;
return;
}
if (cnt > c->out_end - dst) {
cnt = FFMAX(c->out_end - dst, 0);
c->error |= AV_LZO_OUTPUT_FULL;
}
av_memcpy_backptr(dst, back, cnt);
c->out = dst + cnt;
}
int av_lzo1x_decode(void *out, int *outlen, const void *in, int *inlen)
{
int state = 0;
int x;
LZOContext c;
if (*outlen <= 0 || *inlen <= 0) {
int res = 0;
if (*outlen <= 0)
res |= AV_LZO_OUTPUT_FULL;
if (*inlen <= 0)
res |= AV_LZO_INPUT_DEPLETED;
return res;
}
c.in = in;
c.in_end = (const uint8_t *)in + *inlen;
c.out = c.out_start = out;
c.out_end = (uint8_t *)out + *outlen;
c.error = 0;
x = GETB(c);
if (x > 17) {
copy(&c, x - 17);
x = GETB(c);
if (x < 16)
c.error |= AV_LZO_ERROR;
}
if (c.in > c.in_end)
c.error |= AV_LZO_INPUT_DEPLETED;
while (!c.error) {
int cnt, back;
if (x > 15) {
if (x > 63) {
cnt = (x >> 5) - 1;
back = (GETB(c) << 3) + ((x >> 2) & 7) + 1;
} else if (x > 31) {
cnt = get_len(&c, x, 31);
x = GETB(c);
back = (GETB(c) << 6) + (x >> 2) + 1;
} else {
cnt = get_len(&c, x, 7);
back = (1 << 14) + ((x & 8) << 11);
x = GETB(c);
back += (GETB(c) << 6) + (x >> 2);
if (back == (1 << 14)) {
if (cnt != 1)
c.error |= AV_LZO_ERROR;
break;
}
}
} else if (!state) {
cnt = get_len(&c, x, 15);
copy(&c, cnt + 3);
x = GETB(c);
if (x > 15)
continue;
cnt = 1;
back = (1 << 11) + (GETB(c) << 2) + (x >> 2) + 1;
} else {
cnt = 0;
back = (GETB(c) << 2) + (x >> 2) + 1;
}
copy_backptr(&c, back, cnt + 2);
state =
cnt = x & 3;
copy(&c, cnt);
x = GETB(c);
}
*inlen = c.in_end - c.in;
if (c.in > c.in_end)
*inlen = 0;
*outlen = c.out_end - c.out;
return c.error;
}
