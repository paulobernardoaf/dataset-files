#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "dirac_dwt.h"
#define TEMPLATE_8bit
#include "dirac_dwt_template.c"
#define TEMPLATE_10bit
#include "dirac_dwt_template.c"
#define TEMPLATE_12bit
#include "dirac_dwt_template.c"
int ff_spatial_idwt_init(DWTContext *d, DWTPlane *p, enum dwt_type type,
int decomposition_count, int bit_depth)
{
int ret = 0;
d->buffer = p->buf;
d->width = p->width;
d->height = p->height;
d->stride = p->stride;
d->temp = p->tmp;
d->decomposition_count = decomposition_count;
if (bit_depth == 8)
ret = ff_spatial_idwt_init_8bit(d, type);
else if (bit_depth == 10)
ret = ff_spatial_idwt_init_10bit(d, type);
else if (bit_depth == 12)
ret = ff_spatial_idwt_init_12bit(d, type);
else
av_log(NULL, AV_LOG_WARNING, "Unsupported bit depth = %i\n", bit_depth);
if (ret) {
av_log(NULL, AV_LOG_ERROR, "Unknown wavelet type %d\n", type);
return AVERROR_INVALIDDATA;
}
if (ARCH_X86 && bit_depth == 8)
ff_spatial_idwt_init_x86(d, type);
return 0;
}
void ff_spatial_idwt_slice2(DWTContext *d, int y)
{
int level, support = d->support;
for (level = d->decomposition_count-1; level >= 0; level--) {
int wl = d->width >> level;
int hl = d->height >> level;
int stride_l = d->stride << level;
while (d->cs[level].y <= FFMIN((y>>level)+support, hl))
d->spatial_compose(d, level, wl, hl, stride_l);
}
}
