#include "libavcodec/vp8dsp.h"
#define VP8_LF_Y(hv, inner, opt) void ff_vp8_##hv##_loop_filter16##inner##_##opt(uint8_t *dst, ptrdiff_t stride, int flim_E, int flim_I, int hev_thresh)
#define VP8_LF_UV(hv, inner, opt) void ff_vp8_##hv##_loop_filter8uv##inner##_##opt(uint8_t *dstU, uint8_t *dstV, ptrdiff_t stride, int flim_E, int flim_I, int hev_thresh)
#define VP8_LF_SIMPLE(hv, opt) void ff_vp8_##hv##_loop_filter16_simple_##opt(uint8_t *dst, ptrdiff_t stride, int flim)
#define VP8_LF_HV(inner, opt) VP8_LF_Y(h, inner, opt); VP8_LF_Y(v, inner, opt); VP8_LF_UV(h, inner, opt); VP8_LF_UV(v, inner, opt)
#define VP8_LF(opt) VP8_LF_HV(, opt); VP8_LF_HV(_inner, opt); VP8_LF_SIMPLE(h, opt); VP8_LF_SIMPLE(v, opt)
#define VP8_MC(n, opt) void ff_put_vp8_##n##_##opt(uint8_t *dst, ptrdiff_t dststride, uint8_t *src, ptrdiff_t srcstride, int h, int x, int y)
#define VP8_EPEL(w, opt) VP8_MC(pixels ##w, opt); VP8_MC(epel ##w ##_h4, opt); VP8_MC(epel ##w ##_h6, opt); VP8_MC(epel ##w ##_v4, opt); VP8_MC(epel ##w ##_h4v4, opt); VP8_MC(epel ##w ##_h6v4, opt); VP8_MC(epel ##w ##_v6, opt); VP8_MC(epel ##w ##_h4v6, opt); VP8_MC(epel ##w ##_h6v6, opt)
#define VP8_BILIN(w, opt) VP8_MC(bilin ##w ##_h, opt); VP8_MC(bilin ##w ##_v, opt); VP8_MC(bilin ##w ##_hv, opt)
