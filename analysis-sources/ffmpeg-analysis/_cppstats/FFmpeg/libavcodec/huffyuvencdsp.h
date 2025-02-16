#include <stdint.h>
#include "avcodec.h"
typedef struct HuffYUVEncDSPContext {
void (*diff_int16)(uint16_t *dst ,
const uint16_t *src1 ,
const uint16_t *src2 ,
unsigned mask, int w);
void (*sub_hfyu_median_pred_int16)(uint16_t *dst, const uint16_t *src1,
const uint16_t *src2, unsigned mask,
int w, int *left, int *left_top);
} HuffYUVEncDSPContext;
void ff_huffyuvencdsp_init(HuffYUVEncDSPContext *c, AVCodecContext *avctx);
void ff_huffyuvencdsp_init_x86(HuffYUVEncDSPContext *c, AVCodecContext *avctx);
