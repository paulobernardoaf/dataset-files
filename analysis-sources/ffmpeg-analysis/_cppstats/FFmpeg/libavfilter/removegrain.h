#include "avfilter.h"
typedef struct RemoveGrainContext {
const AVClass *class;
int mode[4];
int nb_planes;
int planewidth[4];
int planeheight[4];
int skip_even;
int skip_odd;
int (*rg[4])(int c, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);
void (*fl[4])(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
} RemoveGrainContext;
void ff_removegrain_init_x86(RemoveGrainContext *rg);
