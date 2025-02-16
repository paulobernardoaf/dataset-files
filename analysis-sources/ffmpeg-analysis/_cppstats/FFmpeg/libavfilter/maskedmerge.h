#include "avfilter.h"
#include "framesync.h"
typedef struct MaskedMergeContext {
const AVClass *class;
int width[4], height[4];
int linesize[4];
int nb_planes;
int planes;
int half, depth;
FFFrameSync fs;
void (*maskedmerge)(const uint8_t *bsrc, const uint8_t *osrc,
const uint8_t *msrc, uint8_t *dst,
ptrdiff_t blinesize, ptrdiff_t olinesize,
ptrdiff_t mlinesize, ptrdiff_t dlinesize,
int w, int h,
int half, int shift);
} MaskedMergeContext;
void ff_maskedmerge_init_x86(MaskedMergeContext *s);
