#include "libavcodec/vp3dsp.h"
#include <string.h>
void ff_vp3_idct_add_msa(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_vp3_idct_put_msa(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_vp3_idct_dc_add_msa(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_vp3_v_loop_filter_msa(uint8_t *first_pixel, ptrdiff_t stride,
int *bounding_values);
void ff_put_no_rnd_pixels_l2_msa(uint8_t *dst, const uint8_t *src1,
const uint8_t *src2, ptrdiff_t stride, int h);
void ff_vp3_h_loop_filter_msa(uint8_t *first_pixel, ptrdiff_t stride,
int *bounding_values);
void ff_vp3_idct_add_mmi(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_vp3_idct_put_mmi(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_vp3_idct_dc_add_mmi(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_put_no_rnd_pixels_l2_mmi(uint8_t *dst, const uint8_t *src1,
const uint8_t *src2, ptrdiff_t stride, int h);
