#include <stddef.h>
#include <stdint.h>
typedef struct MSS2DSPContext {
void (*mss2_blit_wmv9)(uint8_t *dst, ptrdiff_t dst_stride,
const uint8_t *srcy, ptrdiff_t srcy_stride,
const uint8_t *srcu, const uint8_t *srcv,
ptrdiff_t srcuv_stride, int w, int h);
void (*mss2_blit_wmv9_masked)(uint8_t *dst, ptrdiff_t dst_stride,
int maskcolor, const uint8_t *mask,
ptrdiff_t mask_stride,
const uint8_t *srcy, ptrdiff_t srcy_stride,
const uint8_t *srcu, const uint8_t *srcv,
ptrdiff_t srcuv_stride, int w, int h);
void (*mss2_gray_fill_masked)(uint8_t *dst, ptrdiff_t dst_stride,
int maskcolor, const uint8_t *mask,
ptrdiff_t mask_stride, int w, int h);
void (*upsample_plane)(uint8_t *plane, ptrdiff_t plane_stride,
int w, int h);
} MSS2DSPContext;
void ff_mss2dsp_init(MSS2DSPContext *dsp);
