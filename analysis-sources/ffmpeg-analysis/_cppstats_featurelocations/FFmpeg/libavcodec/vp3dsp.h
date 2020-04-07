

















#if !defined(AVCODEC_VP3DSP_H)
#define AVCODEC_VP3DSP_H

#include <stddef.h>
#include <stdint.h>

typedef struct VP3DSPContext {










void (*put_no_rnd_pixels_l2)(uint8_t *dst,
const uint8_t *a,
const uint8_t *b,
ptrdiff_t stride, int h);

void (*idct_put)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*idct_add)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*idct_dc_add)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*v_loop_filter)(uint8_t *src, ptrdiff_t stride, int *bounding_values);
void (*h_loop_filter)(uint8_t *src, ptrdiff_t stride, int *bounding_values);
void (*v_loop_filter_unaligned)(uint8_t *src, ptrdiff_t stride, int *bounding_values);
void (*h_loop_filter_unaligned)(uint8_t *src, ptrdiff_t stride, int *bounding_values);
} VP3DSPContext;

void ff_vp3dsp_v_loop_filter_12(uint8_t *first_pixel, ptrdiff_t stride, int *bounding_values);
void ff_vp3dsp_h_loop_filter_12(uint8_t *first_pixel, ptrdiff_t stride, int *bounding_values);

void ff_vp3dsp_idct10_put(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void ff_vp3dsp_idct10_add(uint8_t *dest, ptrdiff_t stride, int16_t *block);

void ff_vp3dsp_init(VP3DSPContext *c, int flags);
void ff_vp3dsp_init_arm(VP3DSPContext *c, int flags);
void ff_vp3dsp_init_ppc(VP3DSPContext *c, int flags);
void ff_vp3dsp_init_x86(VP3DSPContext *c, int flags);
void ff_vp3dsp_init_mips(VP3DSPContext *c, int flags);

void ff_vp3dsp_set_bounding_values(int * bound_values_array, int filter_limit);

#endif 
