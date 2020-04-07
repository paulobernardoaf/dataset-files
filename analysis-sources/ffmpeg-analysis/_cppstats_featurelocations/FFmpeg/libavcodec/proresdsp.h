





















#if !defined(AVCODEC_PRORESDSP_H)
#define AVCODEC_PRORESDSP_H

#include <stddef.h>
#include <stdint.h>
#include "avcodec.h"

typedef struct ProresDSPContext {
int idct_permutation_type;
uint8_t idct_permutation[64];
void (*idct_put)(uint16_t *out, ptrdiff_t linesize, int16_t *block, const int16_t *qmat);
} ProresDSPContext;

int ff_proresdsp_init(ProresDSPContext *dsp, AVCodecContext *avctx);

void ff_proresdsp_init_x86(ProresDSPContext *dsp, AVCodecContext *avctx);

#endif 
