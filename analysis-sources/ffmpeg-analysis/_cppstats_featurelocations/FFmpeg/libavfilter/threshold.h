



















#if !defined(AVFILTER_THRESHOLD_H)
#define AVFILTER_THRESHOLD_H

#include "avfilter.h"
#include "framesync.h"

typedef struct ThresholdContext {
const AVClass *class;

int depth;
int planes;
int bpc;

int nb_planes;
int width[4], height[4];

void (*threshold)(const uint8_t *in, const uint8_t *threshold,
const uint8_t *min, const uint8_t *max,
uint8_t *out,
ptrdiff_t ilinesize, ptrdiff_t tlinesize,
ptrdiff_t flinesize, ptrdiff_t slinesize,
ptrdiff_t olinesize,
int w, int h);

AVFrame *frames[4];
FFFrameSync fs;
} ThresholdContext;

void ff_threshold_init(ThresholdContext *s);
void ff_threshold_init_x86(ThresholdContext *s);

#endif 
