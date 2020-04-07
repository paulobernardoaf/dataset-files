#include <vid.stab/libvidstab.h>
#include "avfilter.h"
VSPixelFormat ff_av2vs_pixfmt(AVFilterContext *ctx, enum AVPixelFormat pf);
void ff_vs_init(void);
