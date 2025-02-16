

















#if !defined(AVFILTER_LIMITER_H)
#define AVFILTER_LIMITER_H

#include <stddef.h>
#include <stdint.h>

typedef struct LimiterDSPContext {
void (*limiter)(const uint8_t *src, uint8_t *dst,
ptrdiff_t slinesize, ptrdiff_t dlinesize,
int w, int h, int min, int max);
} LimiterDSPContext;

void ff_limiter_init_x86(LimiterDSPContext *dsp, int bpp);

#endif 
