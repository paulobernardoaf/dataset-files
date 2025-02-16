#if defined(__cplusplus) && !defined(__STDC_CONSTANT_MACROS) && !defined(UINT64_C)
#error missing -D__STDC_CONSTANT_MACROS / #define __STDC_CONSTANT_MACROS
#endif
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "attributes.h"
#include "macros.h"
#include "version.h"
#include "libavutil/avconfig.h"
#if AV_HAVE_BIGENDIAN
#define AV_NE(be, le) (be)
#else
#define AV_NE(be, le) (le)
#endif
#define RSHIFT(a,b) ((a) > 0 ? ((a) + ((1<<(b))>>1))>>(b) : ((a) + ((1<<(b))>>1)-1)>>(b))
#define ROUNDED_DIV(a,b) (((a)>=0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define AV_CEIL_RSHIFT(a,b) (!av_builtin_constant_p(b) ? -((-(a)) >> (b)) : ((a) + (1<<(b)) - 1) >> (b))
#define FF_CEIL_RSHIFT AV_CEIL_RSHIFT
#define FFUDIV(a,b) (((a)>0 ?(a):(a)-(b)+1) / (b))
#define FFUMOD(a,b) ((a)-(b)*FFUDIV(a,b))
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)
#define FFNABS(a) ((a) <= 0 ? (a) : (-(a)))
#define FFDIFFSIGN(x,y) (((x)>(y)) - ((x)<(y)))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)
#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#if defined(HAVE_AV_CONFIG_H)
#include "config.h"
#include "intmath.h"
#endif
#include "common.h"
#if !defined(av_log2)
av_const int av_log2(unsigned v);
#endif
#if !defined(av_log2_16bit)
av_const int av_log2_16bit(unsigned v);
#endif
static av_always_inline av_const int av_clip_c(int a, int amin, int amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
if (amin > amax) abort();
#endif
if (a < amin) return amin;
else if (a > amax) return amax;
else return a;
}
static av_always_inline av_const int64_t av_clip64_c(int64_t a, int64_t amin, int64_t amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
if (amin > amax) abort();
#endif
if (a < amin) return amin;
else if (a > amax) return amax;
else return a;
}
static av_always_inline av_const uint8_t av_clip_uint8_c(int a)
{
if (a&(~0xFF)) return (~a)>>31;
else return a;
}
static av_always_inline av_const int8_t av_clip_int8_c(int a)
{
if ((a+0x80U) & ~0xFF) return (a>>31) ^ 0x7F;
else return a;
}
static av_always_inline av_const uint16_t av_clip_uint16_c(int a)
{
if (a&(~0xFFFF)) return (~a)>>31;
else return a;
}
static av_always_inline av_const int16_t av_clip_int16_c(int a)
{
if ((a+0x8000U) & ~0xFFFF) return (a>>31) ^ 0x7FFF;
else return a;
}
static av_always_inline av_const int32_t av_clipl_int32_c(int64_t a)
{
if ((a+0x80000000u) & ~UINT64_C(0xFFFFFFFF)) return (int32_t)((a>>63) ^ 0x7FFFFFFF);
else return (int32_t)a;
}
static av_always_inline av_const int av_clip_intp2_c(int a, int p)
{
if (((unsigned)a + (1 << p)) & ~((2 << p) - 1))
return (a >> 31) ^ ((1 << p) - 1);
else
return a;
}
static av_always_inline av_const unsigned av_clip_uintp2_c(int a, int p)
{
if (a & ~((1<<p) - 1)) return (~a) >> 31 & ((1<<p) - 1);
else return a;
}
static av_always_inline av_const unsigned av_mod_uintp2_c(unsigned a, unsigned p)
{
return a & ((1U << p) - 1);
}
static av_always_inline int av_sat_add32_c(int a, int b)
{
return av_clipl_int32((int64_t)a + b);
}
static av_always_inline int av_sat_dadd32_c(int a, int b)
{
return av_sat_add32(a, av_sat_add32(b, b));
}
static av_always_inline int av_sat_sub32_c(int a, int b)
{
return av_clipl_int32((int64_t)a - b);
}
static av_always_inline int av_sat_dsub32_c(int a, int b)
{
return av_sat_sub32(a, av_sat_add32(b, b));
}
static av_always_inline av_const float av_clipf_c(float a, float amin, float amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
if (amin > amax) abort();
#endif
if (a < amin) return amin;
else if (a > amax) return amax;
else return a;
}
static av_always_inline av_const double av_clipd_c(double a, double amin, double amax)
{
#if defined(HAVE_AV_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
if (amin > amax) abort();
#endif
if (a < amin) return amin;
else if (a > amax) return amax;
else return a;
}
static av_always_inline av_const int av_ceil_log2_c(int x)
{
return av_log2((x - 1) << 1);
}
static av_always_inline av_const int av_popcount_c(uint32_t x)
{
x -= (x >> 1) & 0x55555555;
x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
x = (x + (x >> 4)) & 0x0F0F0F0F;
x += x >> 8;
return (x + (x >> 16)) & 0x3F;
}
static av_always_inline av_const int av_popcount64_c(uint64_t x)
{
return av_popcount((uint32_t)x) + av_popcount((uint32_t)(x >> 32));
}
static av_always_inline av_const int av_parity_c(uint32_t v)
{
return av_popcount(v) & 1;
}
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#define GET_UTF8(val, GET_BYTE, ERROR)val= (GET_BYTE);{uint32_t top = (val & 128) >> 1;if ((val & 0xc0) == 0x80 || val >= 0xFE){ERROR}while (val & top) {unsigned int tmp = (GET_BYTE) - 128;if(tmp>>6){ERROR}val= (val<<6) + tmp;top <<= 5;}val &= (top << 1) - 1;}
#define GET_UTF16(val, GET_16BIT, ERROR)val = (GET_16BIT);{unsigned int hi = val - 0xD800;if (hi < 0x800) {val = (GET_16BIT) - 0xDC00;if (val > 0x3FFU || hi > 0x3FFU){ERROR}val += (hi<<10) + 0x10000;}}
#define PUT_UTF8(val, tmp, PUT_BYTE){int bytes, shift;uint32_t in = val;if (in < 0x80) {tmp = in;PUT_BYTE} else {bytes = (av_log2(in) + 4) / 5;shift = (bytes - 1) * 6;tmp = (256 - (256 >> bytes)) | (in >> shift);PUT_BYTEwhile (shift >= 6) {shift -= 6;tmp = 0x80 | ((in >> shift) & 0x3f);PUT_BYTE}}}
#define PUT_UTF16(val, tmp, PUT_16BIT){uint32_t in = val;if (in < 0x10000) {tmp = in;PUT_16BIT} else {tmp = 0xD800 | ((in - 0x10000) >> 10);PUT_16BITtmp = 0xDC00 | ((in - 0x10000) & 0x3FF);PUT_16BIT}}
#include "mem.h"
#if defined(HAVE_AV_CONFIG_H)
#include "internal.h"
#endif 
#if !defined(av_ceil_log2)
#define av_ceil_log2 av_ceil_log2_c
#endif
#if !defined(av_clip)
#define av_clip av_clip_c
#endif
#if !defined(av_clip64)
#define av_clip64 av_clip64_c
#endif
#if !defined(av_clip_uint8)
#define av_clip_uint8 av_clip_uint8_c
#endif
#if !defined(av_clip_int8)
#define av_clip_int8 av_clip_int8_c
#endif
#if !defined(av_clip_uint16)
#define av_clip_uint16 av_clip_uint16_c
#endif
#if !defined(av_clip_int16)
#define av_clip_int16 av_clip_int16_c
#endif
#if !defined(av_clipl_int32)
#define av_clipl_int32 av_clipl_int32_c
#endif
#if !defined(av_clip_intp2)
#define av_clip_intp2 av_clip_intp2_c
#endif
#if !defined(av_clip_uintp2)
#define av_clip_uintp2 av_clip_uintp2_c
#endif
#if !defined(av_mod_uintp2)
#define av_mod_uintp2 av_mod_uintp2_c
#endif
#if !defined(av_sat_add32)
#define av_sat_add32 av_sat_add32_c
#endif
#if !defined(av_sat_dadd32)
#define av_sat_dadd32 av_sat_dadd32_c
#endif
#if !defined(av_sat_sub32)
#define av_sat_sub32 av_sat_sub32_c
#endif
#if !defined(av_sat_dsub32)
#define av_sat_dsub32 av_sat_dsub32_c
#endif
#if !defined(av_clipf)
#define av_clipf av_clipf_c
#endif
#if !defined(av_clipd)
#define av_clipd av_clipd_c
#endif
#if !defined(av_popcount)
#define av_popcount av_popcount_c
#endif
#if !defined(av_popcount64)
#define av_popcount64 av_popcount64_c
#endif
#if !defined(av_parity)
#define av_parity av_parity_c
#endif
