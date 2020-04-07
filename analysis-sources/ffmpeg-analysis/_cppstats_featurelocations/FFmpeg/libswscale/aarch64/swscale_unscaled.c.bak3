

















#include "config.h"
#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"
#include "libavutil/aarch64/cpu.h"

#define YUV_TO_RGB_TABLE c->yuv2rgb_v2r_coeff, c->yuv2rgb_u2g_coeff, c->yuv2rgb_v2g_coeff, c->yuv2rgb_u2b_coeff, 





#define DECLARE_FF_YUVX_TO_RGBX_FUNCS(ifmt, ofmt) int ff_##ifmt##_to_##ofmt##_neon(int w, int h, uint8_t *dst, int linesize, const uint8_t *srcY, int linesizeY, const uint8_t *srcU, int linesizeU, const uint8_t *srcV, int linesizeV, const int16_t *table, int y_offset, int y_coeff); static int ifmt##_to_##ofmt##_neon_wrapper(SwsContext *c, const uint8_t *src[], int srcStride[], int srcSliceY, int srcSliceH, uint8_t *dst[], int dstStride[]) { const int16_t yuv2rgb_table[] = { YUV_TO_RGB_TABLE }; ff_##ifmt##_to_##ofmt##_neon(c->srcW, srcSliceH, dst[0] + srcSliceY * dstStride[0], dstStride[0], src[0], srcStride[0], src[1], srcStride[1], src[2], srcStride[2], yuv2rgb_table, c->yuv2rgb_y_offset >> 6, c->yuv2rgb_y_coeff); return 0; } 

























#define DECLARE_FF_YUVX_TO_ALL_RGBX_FUNCS(yuvx) DECLARE_FF_YUVX_TO_RGBX_FUNCS(yuvx, argb) DECLARE_FF_YUVX_TO_RGBX_FUNCS(yuvx, rgba) DECLARE_FF_YUVX_TO_RGBX_FUNCS(yuvx, abgr) DECLARE_FF_YUVX_TO_RGBX_FUNCS(yuvx, bgra) 





DECLARE_FF_YUVX_TO_ALL_RGBX_FUNCS(yuv420p)
DECLARE_FF_YUVX_TO_ALL_RGBX_FUNCS(yuv422p)

#define DECLARE_FF_NVX_TO_RGBX_FUNCS(ifmt, ofmt) int ff_##ifmt##_to_##ofmt##_neon(int w, int h, uint8_t *dst, int linesize, const uint8_t *srcY, int linesizeY, const uint8_t *srcC, int linesizeC, const int16_t *table, int y_offset, int y_coeff); static int ifmt##_to_##ofmt##_neon_wrapper(SwsContext *c, const uint8_t *src[], int srcStride[], int srcSliceY, int srcSliceH, uint8_t *dst[], int dstStride[]) { const int16_t yuv2rgb_table[] = { YUV_TO_RGB_TABLE }; ff_##ifmt##_to_##ofmt##_neon(c->srcW, srcSliceH, dst[0] + srcSliceY * dstStride[0], dstStride[0], src[0], srcStride[0], src[1], srcStride[1], yuv2rgb_table, c->yuv2rgb_y_offset >> 6, c->yuv2rgb_y_coeff); return 0; } 























#define DECLARE_FF_NVX_TO_ALL_RGBX_FUNCS(nvx) DECLARE_FF_NVX_TO_RGBX_FUNCS(nvx, argb) DECLARE_FF_NVX_TO_RGBX_FUNCS(nvx, rgba) DECLARE_FF_NVX_TO_RGBX_FUNCS(nvx, abgr) DECLARE_FF_NVX_TO_RGBX_FUNCS(nvx, bgra) 





DECLARE_FF_NVX_TO_ALL_RGBX_FUNCS(nv12)
DECLARE_FF_NVX_TO_ALL_RGBX_FUNCS(nv21)





#define SET_FF_NVX_TO_RGBX_FUNC(ifmt, IFMT, ofmt, OFMT, accurate_rnd) do { if (c->srcFormat == AV_PIX_FMT_##IFMT && c->dstFormat == AV_PIX_FMT_##OFMT && !(c->srcH & 1) && !(c->srcW & 15) && !accurate_rnd) c->swscale = ifmt##_to_##ofmt##_neon_wrapper; } while (0)








#define SET_FF_NVX_TO_ALL_RGBX_FUNC(nvx, NVX, accurate_rnd) do { SET_FF_NVX_TO_RGBX_FUNC(nvx, NVX, argb, ARGB, accurate_rnd); SET_FF_NVX_TO_RGBX_FUNC(nvx, NVX, rgba, RGBA, accurate_rnd); SET_FF_NVX_TO_RGBX_FUNC(nvx, NVX, abgr, ABGR, accurate_rnd); SET_FF_NVX_TO_RGBX_FUNC(nvx, NVX, bgra, BGRA, accurate_rnd); } while (0)






static void get_unscaled_swscale_neon(SwsContext *c) {
int accurate_rnd = c->flags & SWS_ACCURATE_RND;

SET_FF_NVX_TO_ALL_RGBX_FUNC(nv12, NV12, accurate_rnd);
SET_FF_NVX_TO_ALL_RGBX_FUNC(nv21, NV21, accurate_rnd);
SET_FF_NVX_TO_ALL_RGBX_FUNC(yuv420p, YUV420P, accurate_rnd);
SET_FF_NVX_TO_ALL_RGBX_FUNC(yuv422p, YUV422P, accurate_rnd);
}

void ff_get_unscaled_swscale_aarch64(SwsContext *c)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags))
get_unscaled_swscale_neon(c);
}
