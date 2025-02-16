





















#include "config.h"

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/pixblockdsp.h"

#if HAVE_ALTIVEC

#if HAVE_VSX
static void get_pixels_altivec(int16_t *restrict block, const uint8_t *pixels,
ptrdiff_t stride)
{
int i;
vector unsigned char perm =
(vector unsigned char) {0x00,0x10, 0x01,0x11,0x02,0x12,0x03,0x13,\
0x04,0x14,0x05,0x15,0x06,0x16,0x07,0x17};
const vector unsigned char zero =
(const vector unsigned char) vec_splat_u8(0);

for (i = 0; i < 8; i++) {



vector unsigned char bytes = vec_vsx_ld(0, pixels);



vector signed short shorts = (vector signed short) vec_perm(bytes, zero, perm);


vec_vsx_st(shorts, i * 16, (vector signed short *) block);

pixels += stride;
}
}
#else
static void get_pixels_altivec(int16_t *restrict block, const uint8_t *pixels,
ptrdiff_t stride)
{
int i;
const vec_u8 zero = (const vec_u8)vec_splat_u8(0);

for (i = 0; i < 8; i++) {
vec_u8 perm = vec_lvsl(0, pixels);



vec_u8 pixl = vec_ld(0, pixels);
vec_u8 pixr = vec_ld(7, pixels);
vec_u8 bytes = vec_perm(pixl, pixr, perm);


vec_s16 shorts = (vec_s16)vec_mergeh(zero, bytes);


vec_st(shorts, i * 16, (vec_s16 *)block);

pixels += stride;
}
}

#endif 

#if HAVE_VSX
static void diff_pixels_altivec(int16_t *restrict block, const uint8_t *s1,
const uint8_t *s2, ptrdiff_t stride)
{
int i;
const vector unsigned char zero =
(const vector unsigned char) vec_splat_u8(0);
vector signed short shorts1, shorts2;

for (i = 0; i < 4; i++) {



vector unsigned char bytes = vec_vsx_ld(0, s1);


shorts1 = (vector signed short) vec_mergeh(bytes, zero);


bytes =vec_vsx_ld(0, s2);


shorts2 = (vector signed short) vec_mergeh(bytes, zero);


shorts1 = vec_sub(shorts1, shorts2);


vec_vsx_st(shorts1, 0, (vector signed short *) block);

s1 += stride;
s2 += stride;
block += 8;







bytes = vec_vsx_ld(0, s1);


shorts1 = (vector signed short) vec_mergeh(bytes, zero);


bytes = vec_vsx_ld(0, s2);


shorts2 = (vector signed short) vec_mergeh(bytes, zero);


shorts1 = vec_sub(shorts1, shorts2);


vec_vsx_st(shorts1, 0, (vector signed short *) block);

s1 += stride;
s2 += stride;
block += 8;
}
}
#else
static void diff_pixels_altivec(int16_t *restrict block, const uint8_t *s1,
const uint8_t *s2, ptrdiff_t stride)
{
int i;
vec_u8 perm;
const vec_u8 zero = (const vec_u8)vec_splat_u8(0);
vec_s16 shorts1, shorts2;

for (i = 0; i < 4; i++) {



perm = vec_lvsl(0, s1);
vec_u8 pixl = vec_ld(0, s1);
vec_u8 pixr = vec_ld(15, s1);
vec_u8 bytes = vec_perm(pixl, pixr, perm);


shorts1 = (vec_s16)vec_mergeh(zero, bytes);


perm = vec_lvsl(0, s2);
pixl = vec_ld(0, s2);
pixr = vec_ld(15, s2);
bytes = vec_perm(pixl, pixr, perm);


shorts2 = (vec_s16)vec_mergeh(zero, bytes);


shorts1 = vec_sub(shorts1, shorts2);


vec_st(shorts1, 0, (vec_s16 *)block);

s1 += stride;
s2 += stride;
block += 8;







perm = vec_lvsl(0, s1);
pixl = vec_ld(0, s1);
pixr = vec_ld(15, s1);
bytes = vec_perm(pixl, pixr, perm);


shorts1 = (vec_s16)vec_mergeh(zero, bytes);


perm = vec_lvsl(0, s2);
pixl = vec_ld(0, s2);
pixr = vec_ld(15, s2);
bytes = vec_perm(pixl, pixr, perm);


shorts2 = (vec_s16)vec_mergeh(zero, bytes);


shorts1 = vec_sub(shorts1, shorts2);


vec_st(shorts1, 0, (vec_s16 *)block);

s1 += stride;
s2 += stride;
block += 8;
}
}

#endif 

#endif 

#if HAVE_VSX
static void get_pixels_vsx(int16_t *restrict block, const uint8_t *pixels,
ptrdiff_t stride)
{
int i;
for (i = 0; i < 8; i++) {
vec_s16 shorts = vsx_ld_u8_s16(0, pixels);

vec_vsx_st(shorts, i * 16, block);

pixels += stride;
}
}

static void diff_pixels_vsx(int16_t *restrict block, const uint8_t *s1,
const uint8_t *s2, ptrdiff_t stride)
{
int i;
vec_s16 shorts1, shorts2;
for (i = 0; i < 8; i++) {
shorts1 = vsx_ld_u8_s16(0, s1);
shorts2 = vsx_ld_u8_s16(0, s2);

shorts1 = vec_sub(shorts1, shorts2);

vec_vsx_st(shorts1, 0, block);

s1 += stride;
s2 += stride;
block += 8;
}
}
#endif 

av_cold void ff_pixblockdsp_init_ppc(PixblockDSPContext *c,
AVCodecContext *avctx,
unsigned high_bit_depth)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->diff_pixels = diff_pixels_altivec;

if (!high_bit_depth) {
c->get_pixels = get_pixels_altivec;
}
#endif 

#if HAVE_VSX
if (!PPC_VSX(av_get_cpu_flags()))
return;

c->diff_pixels = diff_pixels_vsx;

if (!high_bit_depth)
c->get_pixels = get_pixels_vsx;
#endif 
}
