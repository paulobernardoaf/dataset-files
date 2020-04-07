#include <stdint.h>
#include "libavutil/attributes.h"
#include "avcodec.h"
#include "vp56dsp.h"
#include "libavutil/common.h"
#define VP56_EDGE_FILTER(pfx, suf, pix_inc, line_inc) static void pfx ##_edge_filter_ ##suf(uint8_t *yuv, ptrdiff_t stride, int t) { int pix2_inc = 2 * pix_inc; int i, v; for (i=0; i<12; i++) { v = (yuv[-pix2_inc] + 3*(yuv[0]-yuv[-pix_inc]) - yuv[pix_inc] + 4)>>3;v = pfx##_adjust(v, t); yuv[-pix_inc] = av_clip_uint8(yuv[-pix_inc] + v); yuv[0] = av_clip_uint8(yuv[0] - v); yuv += line_inc; } }
#if CONFIG_VP5_DECODER
static int vp5_adjust(int v, int t)
{
int s2, s1 = v >> 31;
v ^= s1;
v -= s1;
v *= v < 2*t;
v -= t;
s2 = v >> 31;
v ^= s2;
v -= s2;
v = t - v;
v += s1;
v ^= s1;
return v;
}
VP56_EDGE_FILTER(vp5, hor, 1, stride)
VP56_EDGE_FILTER(vp5, ver, stride, 1)
av_cold void ff_vp5dsp_init(VP56DSPContext *s)
{
s->edge_filter_hor = vp5_edge_filter_hor;
s->edge_filter_ver = vp5_edge_filter_ver;
}
#endif 
#if CONFIG_VP6_DECODER
av_cold void ff_vp6dsp_init(VP56DSPContext *s)
{
s->vp6_filter_diag4 = ff_vp6_filter_diag4_c;
if (ARCH_ARM)
ff_vp6dsp_init_arm(s);
if (ARCH_X86)
ff_vp6dsp_init_x86(s);
}
#endif 
