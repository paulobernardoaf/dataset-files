#include <stdint.h>
#include <stddef.h>
typedef void (*h264_weight_func)(uint8_t *block, ptrdiff_t stride, int height,
int log2_denom, int weight, int offset);
typedef void (*h264_biweight_func)(uint8_t *dst, uint8_t *src,
ptrdiff_t stride, int height, int log2_denom,
int weightd, int weights, int offset);
typedef struct H264DSPContext {
h264_weight_func weight_h264_pixels_tab[4];
h264_biweight_func biweight_h264_pixels_tab[4];
void (*h264_v_loop_filter_luma)(uint8_t *pix , ptrdiff_t stride,
int alpha, int beta, int8_t *tc0);
void (*h264_h_loop_filter_luma)(uint8_t *pix , ptrdiff_t stride,
int alpha, int beta, int8_t *tc0);
void (*h264_h_loop_filter_luma_mbaff)(uint8_t *pix , ptrdiff_t stride,
int alpha, int beta, int8_t *tc0);
void (*h264_v_loop_filter_luma_intra)(uint8_t *pix, ptrdiff_t stride,
int alpha, int beta);
void (*h264_h_loop_filter_luma_intra)(uint8_t *pix, ptrdiff_t stride,
int alpha, int beta);
void (*h264_h_loop_filter_luma_mbaff_intra)(uint8_t *pix ,
ptrdiff_t stride, int alpha, int beta);
void (*h264_v_loop_filter_chroma)(uint8_t *pix , ptrdiff_t stride,
int alpha, int beta, int8_t *tc0);
void (*h264_h_loop_filter_chroma)(uint8_t *pix , ptrdiff_t stride,
int alpha, int beta, int8_t *tc0);
void (*h264_h_loop_filter_chroma_mbaff)(uint8_t *pix ,
ptrdiff_t stride, int alpha, int beta,
int8_t *tc0);
void (*h264_v_loop_filter_chroma_intra)(uint8_t *pix ,
ptrdiff_t stride, int alpha, int beta);
void (*h264_h_loop_filter_chroma_intra)(uint8_t *pix ,
ptrdiff_t stride, int alpha, int beta);
void (*h264_h_loop_filter_chroma_mbaff_intra)(uint8_t *pix ,
ptrdiff_t stride, int alpha, int beta);
void (*h264_loop_filter_strength)(int16_t bS[2][4][4], uint8_t nnz[40],
int8_t ref[2][40], int16_t mv[2][40][2],
int bidir, int edges, int step,
int mask_mv0, int mask_mv1, int field);
void (*h264_idct_add)(uint8_t *dst ,
int16_t *block , int stride);
void (*h264_idct8_add)(uint8_t *dst ,
int16_t *block , int stride);
void (*h264_idct_dc_add)(uint8_t *dst ,
int16_t *block , int stride);
void (*h264_idct8_dc_add)(uint8_t *dst ,
int16_t *block , int stride);
void (*h264_idct_add16)(uint8_t *dst , const int *blockoffset,
int16_t *block , int stride,
const uint8_t nnzc[15 * 8]);
void (*h264_idct8_add4)(uint8_t *dst , const int *blockoffset,
int16_t *block , int stride,
const uint8_t nnzc[15 * 8]);
void (*h264_idct_add8)(uint8_t **dst , const int *blockoffset,
int16_t *block , int stride,
const uint8_t nnzc[15 * 8]);
void (*h264_idct_add16intra)(uint8_t *dst , const int *blockoffset,
int16_t *block ,
int stride, const uint8_t nnzc[15 * 8]);
void (*h264_luma_dc_dequant_idct)(int16_t *output,
int16_t *input , int qmul);
void (*h264_chroma_dc_dequant_idct)(int16_t *block, int qmul);
void (*h264_add_pixels8_clear)(uint8_t *dst, int16_t *block, int stride);
void (*h264_add_pixels4_clear)(uint8_t *dst, int16_t *block, int stride);
int (*startcode_find_candidate)(const uint8_t *buf, int size);
} H264DSPContext;
void ff_h264dsp_init(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
void ff_h264dsp_init_aarch64(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
void ff_h264dsp_init_arm(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
void ff_h264dsp_init_ppc(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
void ff_h264dsp_init_x86(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
void ff_h264dsp_init_mips(H264DSPContext *c, const int bit_depth,
const int chroma_format_idc);
