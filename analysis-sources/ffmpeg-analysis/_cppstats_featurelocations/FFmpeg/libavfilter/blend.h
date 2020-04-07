



















#if !defined(AVFILTER_BLEND_H)
#define AVFILTER_BLEND_H

#include "libavutil/eval.h"
#include "avfilter.h"

enum BlendMode {
BLEND_UNSET = -1,
BLEND_NORMAL,
BLEND_ADDITION,
BLEND_AND,
BLEND_AVERAGE,
BLEND_BURN,
BLEND_DARKEN,
BLEND_DIFFERENCE,
BLEND_GRAINEXTRACT,
BLEND_DIVIDE,
BLEND_DODGE,
BLEND_EXCLUSION,
BLEND_HARDLIGHT,
BLEND_LIGHTEN,
BLEND_MULTIPLY,
BLEND_NEGATION,
BLEND_OR,
BLEND_OVERLAY,
BLEND_PHOENIX,
BLEND_PINLIGHT,
BLEND_REFLECT,
BLEND_SCREEN,
BLEND_SOFTLIGHT,
BLEND_SUBTRACT,
BLEND_VIVIDLIGHT,
BLEND_XOR,
BLEND_HARDMIX,
BLEND_LINEARLIGHT,
BLEND_GLOW,
BLEND_GRAINMERGE,
BLEND_MULTIPLY128,
BLEND_HEAT,
BLEND_FREEZE,
BLEND_EXTREMITY,
BLEND_NB
};

typedef struct FilterParams {
enum BlendMode mode;
double opacity;
AVExpr *e;
char *expr_str;
void (*blend)(const uint8_t *top, ptrdiff_t top_linesize,
const uint8_t *bottom, ptrdiff_t bottom_linesize,
uint8_t *dst, ptrdiff_t dst_linesize,
ptrdiff_t width, ptrdiff_t height,
struct FilterParams *param, double *values, int starty);
} FilterParams;

void ff_blend_init(FilterParams *param, int depth);
void ff_blend_init_x86(FilterParams *param, int depth);

#endif 
