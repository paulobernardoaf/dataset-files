





















#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/idctdsp.h"
#include "libavcodec/proresdsp.h"

void ff_prores_idct_put_10_sse2(uint16_t *dst, ptrdiff_t linesize,
int16_t *block, const int16_t *qmat);
void ff_prores_idct_put_10_avx (uint16_t *dst, ptrdiff_t linesize,
int16_t *block, const int16_t *qmat);

av_cold void ff_proresdsp_init_x86(ProresDSPContext *dsp, AVCodecContext *avctx)
{
#if ARCH_X86_64
int cpu_flags = av_get_cpu_flags();

if (avctx->bits_per_raw_sample == 10){
if (EXTERNAL_SSE2(cpu_flags)) {
dsp->idct_permutation_type = FF_IDCT_PERM_TRANSPOSE;
dsp->idct_put = ff_prores_idct_put_10_sse2;
}

if (EXTERNAL_AVX(cpu_flags)) {
dsp->idct_permutation_type = FF_IDCT_PERM_TRANSPOSE;
dsp->idct_put = ff_prores_idct_put_10_avx;
}
}
#endif 
}
