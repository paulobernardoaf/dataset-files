



















#if !defined(AVCODEC_DCADCT_H)
#define AVCODEC_DCADCT_H

#include "libavutil/common.h"

typedef struct DCADCTContext {
void (*imdct_half[2])(int32_t *output, const int32_t *input);
} DCADCTContext;

av_cold void ff_dcadct_init(DCADCTContext *c);

#endif
