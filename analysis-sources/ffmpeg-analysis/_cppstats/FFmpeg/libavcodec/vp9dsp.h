#include <stddef.h>
#include <stdint.h>
#include "libavcodec/vp9.h"
typedef void (*vp9_mc_func)(uint8_t *dst, ptrdiff_t dst_stride,
const uint8_t *ref, ptrdiff_t ref_stride,
int h, int mx, int my);
typedef void (*vp9_scaled_mc_func)(uint8_t *dst, ptrdiff_t dst_stride,
const uint8_t *ref, ptrdiff_t ref_stride,
int h, int mx, int my, int dx, int dy);
typedef struct VP9DSPContext {
void (*intra_pred[N_TXFM_SIZES][N_INTRA_PRED_MODES])(uint8_t *dst,
ptrdiff_t stride,
const uint8_t *left,
const uint8_t *top);
void (*itxfm_add[N_TXFM_SIZES + 1][N_TXFM_TYPES])(uint8_t *dst,
ptrdiff_t stride,
int16_t *block, int eob);
void (*loop_filter_8[3][2])(uint8_t *dst, ptrdiff_t stride,
int mb_lim, int lim, int hev_thr);
void (*loop_filter_16[2])(uint8_t *dst, ptrdiff_t stride,
int mb_lim, int lim, int hev_thr);
void (*loop_filter_mix2[2][2][2])(uint8_t *dst, ptrdiff_t stride,
int mb_lim, int lim, int hev_thr);
vp9_mc_func mc[5][N_FILTERS][2][2][2];
vp9_scaled_mc_func smc[5][N_FILTERS][2];
} VP9DSPContext;
extern const int16_t ff_vp9_subpel_filters[3][16][8];
void ff_vp9dsp_init(VP9DSPContext *dsp, int bpp, int bitexact);
void ff_vp9dsp_init_8(VP9DSPContext *dsp);
void ff_vp9dsp_init_10(VP9DSPContext *dsp);
void ff_vp9dsp_init_12(VP9DSPContext *dsp);
void ff_vp9dsp_init_aarch64(VP9DSPContext *dsp, int bpp);
void ff_vp9dsp_init_arm(VP9DSPContext *dsp, int bpp);
void ff_vp9dsp_init_x86(VP9DSPContext *dsp, int bpp, int bitexact);
void ff_vp9dsp_init_mips(VP9DSPContext *dsp, int bpp);
