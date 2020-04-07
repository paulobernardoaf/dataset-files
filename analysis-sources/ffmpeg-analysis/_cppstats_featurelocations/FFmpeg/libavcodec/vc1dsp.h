

























#if !defined(AVCODEC_VC1DSP_H)
#define AVCODEC_VC1DSP_H

#include "hpeldsp.h"
#include "h264chroma.h"

typedef void (*vc1op_pixels_func)(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size, int h);

typedef struct VC1DSPContext {

void (*vc1_inv_trans_8x8)(int16_t *b);
void (*vc1_inv_trans_8x4)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_4x8)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_4x4)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_8x8_dc)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_8x4_dc)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_4x8_dc)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_inv_trans_4x4_dc)(uint8_t *dest, ptrdiff_t stride, int16_t *block);
void (*vc1_v_overlap)(uint8_t *src, int stride);
void (*vc1_h_overlap)(uint8_t *src, int stride);
void (*vc1_v_s_overlap)(int16_t *top, int16_t *bottom);
void (*vc1_h_s_overlap)(int16_t *left, int16_t *right, int left_stride, int right_stride, int flags);
void (*vc1_v_loop_filter4)(uint8_t *src, int stride, int pq);
void (*vc1_h_loop_filter4)(uint8_t *src, int stride, int pq);
void (*vc1_v_loop_filter8)(uint8_t *src, int stride, int pq);
void (*vc1_h_loop_filter8)(uint8_t *src, int stride, int pq);
void (*vc1_v_loop_filter16)(uint8_t *src, int stride, int pq);
void (*vc1_h_loop_filter16)(uint8_t *src, int stride, int pq);




vc1op_pixels_func put_vc1_mspel_pixels_tab[2][16];
vc1op_pixels_func avg_vc1_mspel_pixels_tab[2][16];


h264_chroma_mc_func put_no_rnd_vc1_chroma_pixels_tab[3];
h264_chroma_mc_func avg_no_rnd_vc1_chroma_pixels_tab[3];


void (*sprite_h)(uint8_t *dst, const uint8_t *src, int offset, int advance, int count);
void (*sprite_v_single)(uint8_t *dst, const uint8_t *src1a, const uint8_t *src1b, int offset, int width);
void (*sprite_v_double_noscale)(uint8_t *dst, const uint8_t *src1a, const uint8_t *src2a, int alpha, int width);
void (*sprite_v_double_onescale)(uint8_t *dst, const uint8_t *src1a, const uint8_t *src1b, int offset1,
const uint8_t *src2a, int alpha, int width);
void (*sprite_v_double_twoscale)(uint8_t *dst, const uint8_t *src1a, const uint8_t *src1b, int offset1,
const uint8_t *src2a, const uint8_t *src2b, int offset2,
int alpha, int width);







int (*startcode_find_candidate)(const uint8_t *buf, int size);
} VC1DSPContext;

void ff_vc1dsp_init(VC1DSPContext* c);
void ff_vc1dsp_init_aarch64(VC1DSPContext* dsp);
void ff_vc1dsp_init_arm(VC1DSPContext* dsp);
void ff_vc1dsp_init_ppc(VC1DSPContext *c);
void ff_vc1dsp_init_x86(VC1DSPContext* dsp);
void ff_vc1dsp_init_mips(VC1DSPContext* dsp);

#endif 
