#include "libavutil/attributes.h"
#include "libavcodec/videodsp.h"
static void prefetch_ppc(uint8_t *mem, ptrdiff_t stride, int h)
{
register const uint8_t *p = mem;
do {
__asm__ volatile ("dcbt 0,%0" : : "r" (p));
p += stride;
} while(--h);
}
av_cold void ff_videodsp_init_ppc(VideoDSPContext *ctx, int bpc)
{
ctx->prefetch = prefetch_ppc;
}
