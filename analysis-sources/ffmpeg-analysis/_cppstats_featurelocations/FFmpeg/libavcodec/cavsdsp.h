




















#if !defined(AVCODEC_CAVSDSP_H)
#define AVCODEC_CAVSDSP_H

#include <stdint.h>

#include "avcodec.h"
#include "qpeldsp.h"

typedef struct CAVSDSPContext {
qpel_mc_func put_cavs_qpel_pixels_tab[2][16];
qpel_mc_func avg_cavs_qpel_pixels_tab[2][16];
void (*cavs_filter_lv)(uint8_t *pix, ptrdiff_t stride, int alpha, int beta, int tc, int bs1, int bs2);
void (*cavs_filter_lh)(uint8_t *pix, ptrdiff_t stride, int alpha, int beta, int tc, int bs1, int bs2);
void (*cavs_filter_cv)(uint8_t *pix, ptrdiff_t stride, int alpha, int beta, int tc, int bs1, int bs2);
void (*cavs_filter_ch)(uint8_t *pix, ptrdiff_t stride, int alpha, int beta, int tc, int bs1, int bs2);
void (*cavs_idct8_add)(uint8_t *dst, int16_t *block, ptrdiff_t stride);
int idct_perm;
} CAVSDSPContext;

void ff_cavsdsp_init(CAVSDSPContext* c, AVCodecContext *avctx);
void ff_cavsdsp_init_x86(CAVSDSPContext* c, AVCodecContext *avctx);

#endif 
