

















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/fdctdsp.h"
#include "fdct.h"

av_cold void ff_fdctdsp_init_x86(FDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
int cpu_flags = av_get_cpu_flags();
const int dct_algo = avctx->dct_algo;

if (!high_bit_depth) {
if ((dct_algo == FF_DCT_AUTO || dct_algo == FF_DCT_MMX)) {
if (INLINE_MMX(cpu_flags))
c->fdct = ff_fdct_mmx;

if (INLINE_MMXEXT(cpu_flags))
c->fdct = ff_fdct_mmxext;

if (INLINE_SSE2(cpu_flags))
c->fdct = ff_fdct_sse2;
}
}
}
