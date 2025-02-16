#include <stddef.h>
#include <stdint.h>
typedef struct VP56DSPContext {
void (*edge_filter_hor)(uint8_t *yuv, ptrdiff_t stride, int t);
void (*edge_filter_ver)(uint8_t *yuv, ptrdiff_t stride, int t);
void (*vp6_filter_diag4)(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
const int16_t *h_weights,const int16_t *v_weights);
} VP56DSPContext;
void ff_vp6_filter_diag4_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
const int16_t *h_weights, const int16_t *v_weights);
void ff_vp5dsp_init(VP56DSPContext *s);
void ff_vp6dsp_init(VP56DSPContext *s);
void ff_vp6dsp_init_arm(VP56DSPContext *s);
void ff_vp6dsp_init_x86(VP56DSPContext *s);
