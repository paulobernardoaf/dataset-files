#include <stdint.h>
#include "libavutil/x86/asm.h"
#include "libswscale/swscale_internal.h"
#define YUV2RGB_LOOP(depth) h_size = (c->dstW + 7) & ~7; if (h_size * depth > FFABS(dstStride[0])) h_size -= 8; vshift = c->srcFormat != AV_PIX_FMT_YUV422P; for (y = 0; y < srcSliceH; y++) { uint8_t *image = dst[0] + (y + srcSliceY) * dstStride[0]; const uint8_t *py = src[0] + y * srcStride[0]; const uint8_t *pu = src[1] + (y >> vshift) * srcStride[1]; const uint8_t *pv = src[2] + (y >> vshift) * srcStride[2]; x86_reg index = -h_size / 2; 
extern void RENAME(ff_yuv_420_rgb24)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
extern void RENAME(ff_yuv_420_bgr24)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
#if !COMPILE_TEMPLATE_MMXEXT
extern void RENAME(ff_yuv_420_rgb15)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
extern void RENAME(ff_yuv_420_rgb16)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
extern void RENAME(ff_yuv_420_rgb32)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
extern void RENAME(ff_yuv_420_bgr32)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index);
extern void RENAME(ff_yuva_420_rgb32)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index, const uint8_t *pa_2index);
extern void RENAME(ff_yuva_420_bgr32)(x86_reg index, uint8_t *image, const uint8_t *pu_index,
const uint8_t *pv_index, const uint64_t *pointer_c_dither,
const uint8_t *py_2index, const uint8_t *pa_2index);
static inline int RENAME(yuv420_rgb15)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(2)
#if defined(DITHER1XBPP)
c->blueDither = ff_dither8[y & 1];
c->greenDither = ff_dither8[y & 1];
c->redDither = ff_dither8[(y + 1) & 1];
#endif
RENAME(ff_yuv_420_rgb15)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuv420_rgb16)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(2)
#if defined(DITHER1XBPP)
c->blueDither = ff_dither8[y & 1];
c->greenDither = ff_dither4[y & 1];
c->redDither = ff_dither8[(y + 1) & 1];
#endif
RENAME(ff_yuv_420_rgb16)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuv420_rgb32)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(4)
RENAME(ff_yuv_420_rgb32)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuv420_bgr32)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(4)
RENAME(ff_yuv_420_bgr32)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuva420_rgb32)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(4)
const uint8_t *pa = src[3] + y * srcStride[3];
RENAME(ff_yuva_420_rgb32)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index, pa - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuva420_bgr32)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(4)
const uint8_t *pa = src[3] + y * srcStride[3];
RENAME(ff_yuva_420_bgr32)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index, pa - 2 * index);
}
return srcSliceH;
}
#endif
static inline int RENAME(yuv420_rgb24)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(3)
RENAME(ff_yuv_420_rgb24)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
static inline int RENAME(yuv420_bgr24)(SwsContext *c, const uint8_t *src[],
int srcStride[],
int srcSliceY, int srcSliceH,
uint8_t *dst[], int dstStride[])
{
int y, h_size, vshift;
YUV2RGB_LOOP(3)
RENAME(ff_yuv_420_bgr24)(index, image, pu - index, pv - index, &(c->redDither), py - 2 * index);
}
return srcSliceH;
}
