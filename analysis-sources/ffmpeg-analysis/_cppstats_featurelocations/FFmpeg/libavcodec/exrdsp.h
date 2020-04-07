

















#if !defined(AVCODEC_EXRDSP_H)
#define AVCODEC_EXRDSP_H

#include <stdint.h>
#include "libavutil/common.h"

typedef struct ExrDSPContext {
void (*reorder_pixels)(uint8_t *dst, const uint8_t *src, ptrdiff_t size);
void (*predictor)(uint8_t *src, ptrdiff_t size);
} ExrDSPContext;

void ff_exrdsp_init(ExrDSPContext *c);
void ff_exrdsp_init_x86(ExrDSPContext *c);

#endif 
