#include "avfilter.h"
int ff_scale_eval_dimensions(void *ctx,
const char *w_expr, const char *h_expr,
AVFilterLink *inlink, AVFilterLink *outlink,
int *ret_w, int *ret_h);
int ff_scale_adjust_dimensions(AVFilterLink *inlink,
int *ret_w, int *ret_h,
int force_original_aspect_ratio, int force_divisible_by);
