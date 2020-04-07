

























#include "utvideo.h"

#if FF_API_PRIVATE_OPT
const int ff_ut_pred_order[5] = {
PRED_LEFT, PRED_MEDIAN, PRED_MEDIAN, PRED_NONE, PRED_GRADIENT
};
#endif

const int ff_ut_rgb_order[4] = { 1, 2, 0, 3 }; 

int ff_ut_huff_cmp_len(const void *a, const void *b)
{
const HuffEntry *aa = a, *bb = b;
return (aa->len - bb->len)*256 + aa->sym - bb->sym;
}

int ff_ut10_huff_cmp_len(const void *a, const void *b)
{
const HuffEntry *aa = a, *bb = b;
return (aa->len - bb->len)*1024 + aa->sym - bb->sym;
}
