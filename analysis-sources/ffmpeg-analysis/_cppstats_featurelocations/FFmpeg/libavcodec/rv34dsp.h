

























#if !defined(AVCODEC_RV34DSP_H)
#define AVCODEC_RV34DSP_H

#include "h264chroma.h"
#include "qpeldsp.h"

typedef void (*rv40_weight_func)(uint8_t *dst,
uint8_t *src1,
uint8_t *src2,
int w1, int w2, ptrdiff_t stride);

typedef void (*rv34_inv_transform_func)(int16_t *block);

typedef void (*rv34_idct_add_func)(uint8_t *dst, ptrdiff_t stride, int16_t *block);
typedef void (*rv34_idct_dc_add_func)(uint8_t *dst, ptrdiff_t stride,
int dc);

typedef void (*rv40_weak_loop_filter_func)(uint8_t *src, ptrdiff_t stride,
int filter_p1, int filter_q1,
int alpha, int beta,
int lims, int lim_q1, int lim_p1);

typedef void (*rv40_strong_loop_filter_func)(uint8_t *src, ptrdiff_t stride,
int alpha, int lims,
int dmode, int chroma);

typedef int (*rv40_loop_filter_strength_func)(uint8_t *src, ptrdiff_t stride,
int beta, int beta2, int edge,
int *p1, int *q1);

typedef struct RV34DSPContext {
qpel_mc_func put_pixels_tab[4][16];
qpel_mc_func avg_pixels_tab[4][16];
h264_chroma_mc_func put_chroma_pixels_tab[3];
h264_chroma_mc_func avg_chroma_pixels_tab[3];





rv40_weight_func rv40_weight_pixels_tab[2][2];
rv34_inv_transform_func rv34_inv_transform;
rv34_inv_transform_func rv34_inv_transform_dc;
rv34_idct_add_func rv34_idct_add;
rv34_idct_dc_add_func rv34_idct_dc_add;
rv40_weak_loop_filter_func rv40_weak_loop_filter[2];
rv40_strong_loop_filter_func rv40_strong_loop_filter[2];
rv40_loop_filter_strength_func rv40_loop_filter_strength[2];
} RV34DSPContext;

void ff_rv30dsp_init(RV34DSPContext *c);
void ff_rv34dsp_init(RV34DSPContext *c);
void ff_rv40dsp_init(RV34DSPContext *c);

void ff_rv34dsp_init_arm(RV34DSPContext *c);
void ff_rv34dsp_init_x86(RV34DSPContext *c);

void ff_rv40dsp_init_aarch64(RV34DSPContext *c);
void ff_rv40dsp_init_x86(RV34DSPContext *c);
void ff_rv40dsp_init_arm(RV34DSPContext *c);

#endif 
