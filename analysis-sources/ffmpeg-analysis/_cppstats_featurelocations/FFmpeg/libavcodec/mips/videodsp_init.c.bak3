



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/mips/asmdefs.h"
#include "libavcodec/videodsp.h"

#if HAVE_MSA
static void prefetch_mips(uint8_t *mem, ptrdiff_t stride, int h)
{
register const uint8_t *p = mem;

__asm__ volatile (
"1: \n\t"
"pref 4, 0(%[p]) \n\t"
"pref 4, 32(%[p]) \n\t"
PTR_ADDIU" %[h], %[h], -1 \n\t"
PTR_ADDU " %[p], %[p], %[stride] \n\t"

"bnez %[h], 1b \n\t"

: [p] "+r" (p), [h] "+r" (h)
: [stride] "r" (stride)
);
}
#endif 

av_cold void ff_videodsp_init_mips(VideoDSPContext *ctx, int bpc)
{
#if HAVE_MSA
ctx->prefetch = prefetch_mips;
#endif 
}
