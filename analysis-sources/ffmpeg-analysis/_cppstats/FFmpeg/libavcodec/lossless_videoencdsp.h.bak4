

















#if !defined(AVCODEC_LOSSLESS_VIDEOENCDSP_H)
#define AVCODEC_LOSSLESS_VIDEOENCDSP_H

#include <stdint.h>

#include "avcodec.h"

typedef struct LLVidEncDSPContext {
void (*diff_bytes)(uint8_t *dst ,
const uint8_t *src1 ,
const uint8_t *src2 ,
intptr_t w);




void (*sub_median_pred)(uint8_t *dst, const uint8_t *src1,
const uint8_t *src2, intptr_t w,
int *left, int *left_top);

void (*sub_left_predict)(uint8_t *dst, uint8_t *src,
ptrdiff_t stride, ptrdiff_t width, int height);
} LLVidEncDSPContext;

void ff_llvidencdsp_init(LLVidEncDSPContext *c);
void ff_llvidencdsp_init_x86(LLVidEncDSPContext *c);

#endif 
