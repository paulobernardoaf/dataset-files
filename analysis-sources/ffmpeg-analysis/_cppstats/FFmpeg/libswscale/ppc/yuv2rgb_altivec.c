#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "config.h"
#include "libswscale/rgb2rgb.h"
#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/pixdesc.h"
#include "yuv2rgb_altivec.h"
#if HAVE_ALTIVEC
#undef PROFILE_THE_BEAST
#undef INC_SCALING
typedef unsigned char ubyte;
typedef signed char sbyte;
static const vector unsigned char
perm_rgb_0 = { 0x00, 0x01, 0x10, 0x02, 0x03, 0x11, 0x04, 0x05,
0x12, 0x06, 0x07, 0x13, 0x08, 0x09, 0x14, 0x0a },
perm_rgb_1 = { 0x0b, 0x15, 0x0c, 0x0d, 0x16, 0x0e, 0x0f, 0x17,
0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f },
perm_rgb_2 = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
0x00, 0x01, 0x18, 0x02, 0x03, 0x19, 0x04, 0x05 },
perm_rgb_3 = { 0x1a, 0x06, 0x07, 0x1b, 0x08, 0x09, 0x1c, 0x0a,
0x0b, 0x1d, 0x0c, 0x0d, 0x1e, 0x0e, 0x0f, 0x1f };
#define vec_merge3(x2, x1, x0, y0, y1, y2) do { __typeof__(x0) o0, o2, o3; o0 = vec_mergeh(x0, x1); y0 = vec_perm(o0, x2, perm_rgb_0); o2 = vec_perm(o0, x2, perm_rgb_1); o3 = vec_mergel(x0, x1); y1 = vec_perm(o3, o2, perm_rgb_2); y2 = vec_perm(o3, o2, perm_rgb_3); } while (0)
#define vec_mstbgr24(x0, x1, x2, ptr) do { __typeof__(x0) _0, _1, _2; vec_merge3(x0, x1, x2, _0, _1, _2); vec_st(_0, 0, ptr++); vec_st(_1, 0, ptr++); vec_st(_2, 0, ptr++); } while (0)
#define vec_mstrgb24(x0, x1, x2, ptr) do { __typeof__(x0) _0, _1, _2; vec_merge3(x2, x1, x0, _0, _1, _2); vec_st(_0, 0, ptr++); vec_st(_1, 0, ptr++); vec_st(_2, 0, ptr++); } while (0)
#define vec_mstrgb32(T, x0, x1, x2, x3, ptr) do { T _0, _1, _2, _3; _0 = vec_mergeh(x0, x1); _1 = vec_mergeh(x2, x3); _2 = (T) vec_mergeh((vector unsigned short) _0, (vector unsigned short) _1); _3 = (T) vec_mergel((vector unsigned short) _0, (vector unsigned short) _1); vec_st(_2, 0 * 16, (T *) ptr); vec_st(_3, 1 * 16, (T *) ptr); _0 = vec_mergel(x0, x1); _1 = vec_mergel(x2, x3); _2 = (T) vec_mergeh((vector unsigned short) _0, (vector unsigned short) _1); _3 = (T) vec_mergel((vector unsigned short) _0, (vector unsigned short) _1); vec_st(_2, 2 * 16, (T *) ptr); vec_st(_3, 3 * 16, (T *) ptr); ptr += 4; } while (0)
#if HAVE_BIGENDIAN
#define vec_unh(x) (vector signed short) vec_perm(x, (__typeof__(x)) { 0 }, ((vector unsigned char) { 0x10, 0x00, 0x10, 0x01, 0x10, 0x02, 0x10, 0x03, 0x10, 0x04, 0x10, 0x05, 0x10, 0x06, 0x10, 0x07 }))
#define vec_unl(x) (vector signed short) vec_perm(x, (__typeof__(x)) { 0 }, ((vector unsigned char) { 0x10, 0x08, 0x10, 0x09, 0x10, 0x0A, 0x10, 0x0B, 0x10, 0x0C, 0x10, 0x0D, 0x10, 0x0E, 0x10, 0x0F }))
#else
#define vec_unh(x)(vector signed short) vec_mergeh(x,(__typeof__(x)) { 0 })
#define vec_unl(x)(vector signed short) vec_mergel(x,(__typeof__(x)) { 0 })
#endif
#define vec_clip_s16(x) vec_max(vec_min(x, ((vector signed short) { 235, 235, 235, 235, 235, 235, 235, 235 })), ((vector signed short) { 16, 16, 16, 16, 16, 16, 16, 16 }))
#define vec_packclp(x, y) (vector unsigned char) vec_packs((vector unsigned short) vec_max(x, ((vector signed short) { 0 })), (vector unsigned short) vec_max(y, ((vector signed short) { 0 })))
static inline void cvtyuvtoRGB(SwsContext *c, vector signed short Y,
vector signed short U, vector signed short V,
vector signed short *R, vector signed short *G,
vector signed short *B)
{
vector signed short vx, ux, uvx;
Y = vec_mradds(Y, c->CY, c->OY);
U = vec_sub(U, (vector signed short)
vec_splat((vector signed short) { 128 }, 0));
V = vec_sub(V, (vector signed short)
vec_splat((vector signed short) { 128 }, 0));
ux = vec_sl(U, c->CSHIFT);
*B = vec_mradds(ux, c->CBU, Y);
vx = vec_sl(V, c->CSHIFT);
*R = vec_mradds(vx, c->CRV, Y);
uvx = vec_mradds(U, c->CGU, Y);
*G = vec_mradds(V, c->CGV, uvx);
}
#define DEFCSP420_CVT(name, out_pixels) static int altivec_ ##name(SwsContext *c, const unsigned char **in, int *instrides, int srcSliceY, int srcSliceH, unsigned char **oplanes, int *outstrides) { int w = c->srcW; int h = srcSliceH; int i, j; int instrides_scl[3]; vector unsigned char y0, y1; vector signed char u, v; vector signed short Y0, Y1, Y2, Y3; vector signed short U, V; vector signed short vx, ux, uvx; vector signed short vx0, ux0, uvx0; vector signed short vx1, ux1, uvx1; vector signed short R0, G0, B0; vector signed short R1, G1, B1; vector unsigned char R, G, B; vector signed short lCY = c->CY; vector signed short lOY = c->OY; vector signed short lCRV = c->CRV; vector signed short lCBU = c->CBU; vector signed short lCGU = c->CGU; vector signed short lCGV = c->CGV; vector unsigned short lCSHIFT = c->CSHIFT; const ubyte *y1i = in[0]; const ubyte *y2i = in[0] + instrides[0]; const ubyte *ui = in[1]; const ubyte *vi = in[2]; vector unsigned char *oute, *outo; instrides_scl[0] = instrides[0] * 2 - w; instrides_scl[1] = instrides[1] - w / 2; instrides_scl[2] = instrides[2] - w / 2; for (i = 0; i < h / 2; i++) { oute = (vector unsigned char *)(oplanes[0] + outstrides[0] * (srcSliceY + i * 2)); outo = oute + (outstrides[0] >> 4); vec_dstst(outo, (0x02000002 | (((w * 3 + 32) / 32) << 16)), 0); vec_dstst(oute, (0x02000002 | (((w * 3 + 32) / 32) << 16)), 1); for (j = 0; j < w / 16; j++) { y0 = vec_xl(0, y1i); y1 = vec_xl(0, y2i); u = (vector signed char) vec_xl(0, ui); v = (vector signed char) vec_xl(0, vi); u = (vector signed char) vec_sub(u, (vector signed char) vec_splat((vector signed char) { 128 }, 0)); v = (vector signed char) vec_sub(v, (vector signed char) vec_splat((vector signed char) { 128 }, 0)); U = vec_unpackh(u); V = vec_unpackh(v); Y0 = vec_unh(y0); Y1 = vec_unl(y0); Y2 = vec_unh(y1); Y3 = vec_unl(y1); Y0 = vec_mradds(Y0, lCY, lOY); Y1 = vec_mradds(Y1, lCY, lOY); Y2 = vec_mradds(Y2, lCY, lOY); Y3 = vec_mradds(Y3, lCY, lOY); ux = vec_sl(U, lCSHIFT); ux = vec_mradds(ux, lCBU, (vector signed short) { 0 }); ux0 = vec_mergeh(ux, ux); ux1 = vec_mergel(ux, ux); vx = vec_sl(V, lCSHIFT); vx = vec_mradds(vx, lCRV, (vector signed short) { 0 }); vx0 = vec_mergeh(vx, vx); vx1 = vec_mergel(vx, vx); uvx = vec_mradds(U, lCGU, (vector signed short) { 0 }); uvx = vec_mradds(V, lCGV, uvx); uvx0 = vec_mergeh(uvx, uvx); uvx1 = vec_mergel(uvx, uvx); R0 = vec_add(Y0, vx0); G0 = vec_add(Y0, uvx0); B0 = vec_add(Y0, ux0); R1 = vec_add(Y1, vx1); G1 = vec_add(Y1, uvx1); B1 = vec_add(Y1, ux1); R = vec_packclp(R0, R1); G = vec_packclp(G0, G1); B = vec_packclp(B0, B1); out_pixels(R, G, B, oute); R0 = vec_add(Y2, vx0); G0 = vec_add(Y2, uvx0); B0 = vec_add(Y2, ux0); R1 = vec_add(Y3, vx1); G1 = vec_add(Y3, uvx1); B1 = vec_add(Y3, ux1); R = vec_packclp(R0, R1); G = vec_packclp(G0, G1); B = vec_packclp(B0, B1); out_pixels(R, G, B, outo); y1i += 16; y2i += 16; ui += 8; vi += 8; } ui += instrides_scl[1]; vi += instrides_scl[2]; y1i += instrides_scl[0]; y2i += instrides_scl[0]; } return srcSliceH; }
#define out_abgr(a, b, c, ptr) vec_mstrgb32(__typeof__(a), ((__typeof__(a)) { 255 }), c, b, a, ptr)
#define out_bgra(a, b, c, ptr) vec_mstrgb32(__typeof__(a), c, b, a, ((__typeof__(a)) { 255 }), ptr)
#define out_rgba(a, b, c, ptr) vec_mstrgb32(__typeof__(a), a, b, c, ((__typeof__(a)) { 255 }), ptr)
#define out_argb(a, b, c, ptr) vec_mstrgb32(__typeof__(a), ((__typeof__(a)) { 255 }), a, b, c, ptr)
#define out_rgb24(a, b, c, ptr) vec_mstrgb24(a, b, c, ptr)
#define out_bgr24(a, b, c, ptr) vec_mstbgr24(a, b, c, ptr)
DEFCSP420_CVT(yuv2_abgr, out_abgr)
DEFCSP420_CVT(yuv2_bgra, out_bgra)
DEFCSP420_CVT(yuv2_rgba, out_rgba)
DEFCSP420_CVT(yuv2_argb, out_argb)
DEFCSP420_CVT(yuv2_rgb24, out_rgb24)
DEFCSP420_CVT(yuv2_bgr24, out_bgr24)
static const vector unsigned char
demux_u = { 0x10, 0x00, 0x10, 0x00,
0x10, 0x04, 0x10, 0x04,
0x10, 0x08, 0x10, 0x08,
0x10, 0x0c, 0x10, 0x0c },
demux_v = { 0x10, 0x02, 0x10, 0x02,
0x10, 0x06, 0x10, 0x06,
0x10, 0x0A, 0x10, 0x0A,
0x10, 0x0E, 0x10, 0x0E },
demux_y = { 0x10, 0x01, 0x10, 0x03,
0x10, 0x05, 0x10, 0x07,
0x10, 0x09, 0x10, 0x0B,
0x10, 0x0D, 0x10, 0x0F };
static int altivec_uyvy_rgb32(SwsContext *c, const unsigned char **in,
int *instrides, int srcSliceY, int srcSliceH,
unsigned char **oplanes, int *outstrides)
{
int w = c->srcW;
int h = srcSliceH;
int i, j;
vector unsigned char uyvy;
vector signed short Y, U, V;
vector signed short R0, G0, B0, R1, G1, B1;
vector unsigned char R, G, B;
vector unsigned char *out;
const ubyte *img;
img = in[0];
out = (vector unsigned char *) (oplanes[0] + srcSliceY * outstrides[0]);
for (i = 0; i < h; i++)
for (j = 0; j < w / 16; j++) {
uyvy = vec_ld(0, img);
U = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_u);
V = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_v);
Y = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_y);
cvtyuvtoRGB(c, Y, U, V, &R0, &G0, &B0);
uyvy = vec_ld(16, img);
U = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_u);
V = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_v);
Y = (vector signed short)
vec_perm(uyvy, (vector unsigned char) { 0 }, demux_y);
cvtyuvtoRGB(c, Y, U, V, &R1, &G1, &B1);
R = vec_packclp(R0, R1);
G = vec_packclp(G0, G1);
B = vec_packclp(B0, B1);
out_rgba(R, G, B, out);
img += 32;
}
return srcSliceH;
}
#endif 
av_cold SwsFunc ff_yuv2rgb_init_ppc(SwsContext *c)
{
#if HAVE_ALTIVEC
if (!(av_get_cpu_flags() & AV_CPU_FLAG_ALTIVEC))
return NULL;
if ((c->srcW & 0xf) != 0)
return NULL;
switch (c->srcFormat) {
case AV_PIX_FMT_YUV410P:
case AV_PIX_FMT_YUV420P:
case AV_PIX_FMT_GRAY8:
case AV_PIX_FMT_NV12:
case AV_PIX_FMT_NV21:
if ((c->srcH & 0x1) != 0)
return NULL;
switch (c->dstFormat) {
case AV_PIX_FMT_RGB24:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space RGB24\n");
return altivec_yuv2_rgb24;
case AV_PIX_FMT_BGR24:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space BGR24\n");
return altivec_yuv2_bgr24;
case AV_PIX_FMT_ARGB:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space ARGB\n");
return altivec_yuv2_argb;
case AV_PIX_FMT_ABGR:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space ABGR\n");
return altivec_yuv2_abgr;
case AV_PIX_FMT_RGBA:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space RGBA\n");
return altivec_yuv2_rgba;
case AV_PIX_FMT_BGRA:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space BGRA\n");
return altivec_yuv2_bgra;
default: return NULL;
}
break;
case AV_PIX_FMT_UYVY422:
switch (c->dstFormat) {
case AV_PIX_FMT_BGR32:
av_log(c, AV_LOG_WARNING, "ALTIVEC: Color Space UYVY -> RGB32\n");
return altivec_uyvy_rgb32;
default: return NULL;
}
break;
}
#endif 
return NULL;
}
av_cold void ff_yuv2rgb_init_tables_ppc(SwsContext *c,
const int inv_table[4],
int brightness,
int contrast,
int saturation)
{
#if HAVE_ALTIVEC
union {
DECLARE_ALIGNED(16, signed short, tmp)[8];
vector signed short vec;
} buf;
if (!(av_get_cpu_flags() & AV_CPU_FLAG_ALTIVEC))
return;
buf.tmp[0] = ((0xffffLL) * contrast >> 8) >> 9; 
buf.tmp[1] = -256 * brightness; 
buf.tmp[2] = (inv_table[0] >> 3) * (contrast >> 16) * (saturation >> 16); 
buf.tmp[3] = (inv_table[1] >> 3) * (contrast >> 16) * (saturation >> 16); 
buf.tmp[4] = -((inv_table[2] >> 1) * (contrast >> 16) * (saturation >> 16)); 
buf.tmp[5] = -((inv_table[3] >> 1) * (contrast >> 16) * (saturation >> 16)); 
c->CSHIFT = (vector unsigned short) vec_splat_u16(2);
c->CY = vec_splat((vector signed short) buf.vec, 0);
c->OY = vec_splat((vector signed short) buf.vec, 1);
c->CRV = vec_splat((vector signed short) buf.vec, 2);
c->CBU = vec_splat((vector signed short) buf.vec, 3);
c->CGU = vec_splat((vector signed short) buf.vec, 4);
c->CGV = vec_splat((vector signed short) buf.vec, 5);
return;
#endif 
}
#if HAVE_ALTIVEC
static av_always_inline void yuv2packedX_altivec(SwsContext *c,
const int16_t *lumFilter,
const int16_t **lumSrc,
int lumFilterSize,
const int16_t *chrFilter,
const int16_t **chrUSrc,
const int16_t **chrVSrc,
int chrFilterSize,
const int16_t **alpSrc,
uint8_t *dest,
int dstW, int dstY,
enum AVPixelFormat target)
{
int i, j;
vector signed short X, X0, X1, Y0, U0, V0, Y1, U1, V1, U, V;
vector signed short R0, G0, B0, R1, G1, B1;
vector unsigned char R, G, B;
vector unsigned char *out, *nout;
vector signed short RND = vec_splat_s16(1 << 3);
vector unsigned short SCL = vec_splat_u16(4);
DECLARE_ALIGNED(16, unsigned int, scratch)[16];
vector signed short *YCoeffs, *CCoeffs;
YCoeffs = c->vYCoeffsBank + dstY * lumFilterSize;
CCoeffs = c->vCCoeffsBank + dstY * chrFilterSize;
out = (vector unsigned char *) dest;
for (i = 0; i < dstW; i += 16) {
Y0 = RND;
Y1 = RND;
for (j = 0; j < lumFilterSize; j++) {
X0 = vec_ld(0, &lumSrc[j][i]);
X1 = vec_ld(16, &lumSrc[j][i]);
Y0 = vec_mradds(X0, YCoeffs[j], Y0);
Y1 = vec_mradds(X1, YCoeffs[j], Y1);
}
U = RND;
V = RND;
for (j = 0; j < chrFilterSize; j++) {
X = vec_ld(0, &chrUSrc[j][i / 2]);
U = vec_mradds(X, CCoeffs[j], U);
X = vec_ld(0, &chrVSrc[j][i / 2]);
V = vec_mradds(X, CCoeffs[j], V);
}
Y0 = vec_sra(Y0, SCL);
Y1 = vec_sra(Y1, SCL);
U = vec_sra(U, SCL);
V = vec_sra(V, SCL);
Y0 = vec_clip_s16(Y0);
Y1 = vec_clip_s16(Y1);
U = vec_clip_s16(U);
V = vec_clip_s16(V);
U0 = vec_mergeh(U, U);
V0 = vec_mergeh(V, V);
U1 = vec_mergel(U, U);
V1 = vec_mergel(V, V);
cvtyuvtoRGB(c, Y0, U0, V0, &R0, &G0, &B0);
cvtyuvtoRGB(c, Y1, U1, V1, &R1, &G1, &B1);
R = vec_packclp(R0, R1);
G = vec_packclp(G0, G1);
B = vec_packclp(B0, B1);
switch (target) {
case AV_PIX_FMT_ABGR:
out_abgr(R, G, B, out);
break;
case AV_PIX_FMT_BGRA:
out_bgra(R, G, B, out);
break;
case AV_PIX_FMT_RGBA:
out_rgba(R, G, B, out);
break;
case AV_PIX_FMT_ARGB:
out_argb(R, G, B, out);
break;
case AV_PIX_FMT_RGB24:
out_rgb24(R, G, B, out);
break;
case AV_PIX_FMT_BGR24:
out_bgr24(R, G, B, out);
break;
default:
{
static int printed_error_message;
if (!printed_error_message) {
av_log(c, AV_LOG_ERROR,
"altivec_yuv2packedX doesn't support %s output\n",
av_get_pix_fmt_name(c->dstFormat));
printed_error_message = 1;
}
return;
}
}
}
if (i < dstW) {
i -= 16;
Y0 = RND;
Y1 = RND;
for (j = 0; j < lumFilterSize; j++) {
X0 = vec_ld(0, &lumSrc[j][i]);
X1 = vec_ld(16, &lumSrc[j][i]);
Y0 = vec_mradds(X0, YCoeffs[j], Y0);
Y1 = vec_mradds(X1, YCoeffs[j], Y1);
}
U = RND;
V = RND;
for (j = 0; j < chrFilterSize; j++) {
X = vec_ld(0, &chrUSrc[j][i / 2]);
U = vec_mradds(X, CCoeffs[j], U);
X = vec_ld(0, &chrVSrc[j][i / 2]);
V = vec_mradds(X, CCoeffs[j], V);
}
Y0 = vec_sra(Y0, SCL);
Y1 = vec_sra(Y1, SCL);
U = vec_sra(U, SCL);
V = vec_sra(V, SCL);
Y0 = vec_clip_s16(Y0);
Y1 = vec_clip_s16(Y1);
U = vec_clip_s16(U);
V = vec_clip_s16(V);
U0 = vec_mergeh(U, U);
V0 = vec_mergeh(V, V);
U1 = vec_mergel(U, U);
V1 = vec_mergel(V, V);
cvtyuvtoRGB(c, Y0, U0, V0, &R0, &G0, &B0);
cvtyuvtoRGB(c, Y1, U1, V1, &R1, &G1, &B1);
R = vec_packclp(R0, R1);
G = vec_packclp(G0, G1);
B = vec_packclp(B0, B1);
nout = (vector unsigned char *) scratch;
switch (target) {
case AV_PIX_FMT_ABGR:
out_abgr(R, G, B, nout);
break;
case AV_PIX_FMT_BGRA:
out_bgra(R, G, B, nout);
break;
case AV_PIX_FMT_RGBA:
out_rgba(R, G, B, nout);
break;
case AV_PIX_FMT_ARGB:
out_argb(R, G, B, nout);
break;
case AV_PIX_FMT_RGB24:
out_rgb24(R, G, B, nout);
break;
case AV_PIX_FMT_BGR24:
out_bgr24(R, G, B, nout);
break;
default:
av_log(c, AV_LOG_ERROR,
"altivec_yuv2packedX doesn't support %s output\n",
av_get_pix_fmt_name(c->dstFormat));
return;
}
memcpy(&((uint32_t *) dest)[i], scratch, (dstW - i) / 4);
}
}
#define YUV2PACKEDX_WRAPPER(suffix, pixfmt) void ff_yuv2 ##suffix ##_X_altivec(SwsContext *c, const int16_t *lumFilter, const int16_t **lumSrc, int lumFilterSize, const int16_t *chrFilter, const int16_t **chrUSrc, const int16_t **chrVSrc, int chrFilterSize, const int16_t **alpSrc, uint8_t *dest, int dstW, int dstY) { yuv2packedX_altivec(c, lumFilter, lumSrc, lumFilterSize, chrFilter, chrUSrc, chrVSrc, chrFilterSize, alpSrc, dest, dstW, dstY, pixfmt); }
YUV2PACKEDX_WRAPPER(abgr, AV_PIX_FMT_ABGR);
YUV2PACKEDX_WRAPPER(bgra, AV_PIX_FMT_BGRA);
YUV2PACKEDX_WRAPPER(argb, AV_PIX_FMT_ARGB);
YUV2PACKEDX_WRAPPER(rgba, AV_PIX_FMT_RGBA);
YUV2PACKEDX_WRAPPER(rgb24, AV_PIX_FMT_RGB24);
YUV2PACKEDX_WRAPPER(bgr24, AV_PIX_FMT_BGR24);
#endif 
