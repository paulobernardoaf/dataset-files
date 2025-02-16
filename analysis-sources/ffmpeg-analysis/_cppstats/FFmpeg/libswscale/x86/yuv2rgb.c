#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "libswscale/rgb2rgb.h"
#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"
#include "libavutil/attributes.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavutil/cpu.h"
#if HAVE_X86ASM
#define DITHER1XBPP 
DECLARE_ASM_CONST(8, uint64_t, mmx_00ffw) = 0x00ff00ff00ff00ffULL;
DECLARE_ASM_CONST(8, uint64_t, mmx_redmask) = 0xf8f8f8f8f8f8f8f8ULL;
DECLARE_ASM_CONST(8, uint64_t, mmx_grnmask) = 0xfcfcfcfcfcfcfcfcULL;
DECLARE_ASM_CONST(8, uint64_t, pb_e0) = 0xe0e0e0e0e0e0e0e0ULL;
DECLARE_ASM_CONST(8, uint64_t, pb_03) = 0x0303030303030303ULL;
DECLARE_ASM_CONST(8, uint64_t, pb_07) = 0x0707070707070707ULL;
#if HAVE_MMX
#undef RENAME
#undef COMPILE_TEMPLATE_MMXEXT
#define COMPILE_TEMPLATE_MMXEXT 0
#define RENAME(a) a ##_mmx
#include "yuv2rgb_template.c"
#endif 
#if HAVE_MMXEXT
#undef RENAME
#undef COMPILE_TEMPLATE_MMXEXT
#define COMPILE_TEMPLATE_MMXEXT 1
#define RENAME(a) a ##_mmxext
#include "yuv2rgb_template.c"
#endif 
#if HAVE_SSSE3
#undef RENAME
#undef COMPILE_TEMPLATE_MMXEXT
#define COMPILE_TEMPLATE_MMXEXT 0
#define RENAME(a) a ##_ssse3
#include "yuv2rgb_template.c"
#endif
#endif 
av_cold SwsFunc ff_yuv2rgb_init_x86(SwsContext *c)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSSE3(cpu_flags)) {
switch (c->dstFormat) {
case AV_PIX_FMT_RGB32:
if (c->srcFormat == AV_PIX_FMT_YUVA420P) {
#if CONFIG_SWSCALE_ALPHA
return yuva420_rgb32_ssse3;
#endif
break;
} else
return yuv420_rgb32_ssse3;
case AV_PIX_FMT_BGR32:
if (c->srcFormat == AV_PIX_FMT_YUVA420P) {
#if CONFIG_SWSCALE_ALPHA
return yuva420_bgr32_ssse3;
#endif
break;
} else
return yuv420_bgr32_ssse3;
case AV_PIX_FMT_RGB24:
return yuv420_rgb24_ssse3;
case AV_PIX_FMT_BGR24:
return yuv420_bgr24_ssse3;
case AV_PIX_FMT_RGB565:
return yuv420_rgb16_ssse3;
case AV_PIX_FMT_RGB555:
return yuv420_rgb15_ssse3;
}
}
if (EXTERNAL_MMXEXT(cpu_flags)) {
switch (c->dstFormat) {
case AV_PIX_FMT_RGB24:
return yuv420_rgb24_mmxext;
case AV_PIX_FMT_BGR24:
return yuv420_bgr24_mmxext;
}
}
if (EXTERNAL_MMX(cpu_flags)) {
switch (c->dstFormat) {
case AV_PIX_FMT_RGB32:
if (c->srcFormat == AV_PIX_FMT_YUVA420P) {
#if CONFIG_SWSCALE_ALPHA
return yuva420_rgb32_mmx;
#endif
break;
} else
return yuv420_rgb32_mmx;
case AV_PIX_FMT_BGR32:
if (c->srcFormat == AV_PIX_FMT_YUVA420P) {
#if CONFIG_SWSCALE_ALPHA
return yuva420_bgr32_mmx;
#endif
break;
} else
return yuv420_bgr32_mmx;
case AV_PIX_FMT_RGB24:
return yuv420_rgb24_mmx;
case AV_PIX_FMT_BGR24:
return yuv420_bgr24_mmx;
case AV_PIX_FMT_RGB565:
return yuv420_rgb16_mmx;
case AV_PIX_FMT_RGB555:
return yuv420_rgb15_mmx;
}
}
#endif 
return NULL;
}
