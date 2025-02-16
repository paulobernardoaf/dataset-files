





















#if !defined(AVFILTER_BOXBLUR_H)
#define AVFILTER_BOXBLUR_H

#include "libavutil/eval.h"
#include "libavutil/pixdesc.h"
#include "libavutil/mem.h"

#include "avfilter.h"

typedef struct FilterParam {
int radius;
int power;
char *radius_expr;
} FilterParam;

#define Y 0
#define U 1
#define V 2
#define A 3

int ff_boxblur_eval_filter_params(AVFilterLink *inlink,
FilterParam *luma_param,
FilterParam *chroma_param,
FilterParam *alpha_param);

#endif 
