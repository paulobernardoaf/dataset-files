#include "config.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/mdct15.h"
void ff_mdct15_postreindex_sse3(FFTComplex *out, FFTComplex *in, FFTComplex *exp, int *lut, ptrdiff_t len8);
void ff_mdct15_postreindex_avx2(FFTComplex *out, FFTComplex *in, FFTComplex *exp, int *lut, ptrdiff_t len8);
void ff_fft15_avx(FFTComplex *out, FFTComplex *in, FFTComplex *exptab, ptrdiff_t stride);
static void perm_twiddles(MDCT15Context *s)
{
int k;
FFTComplex tmp[30];
s->exptab[60].re = s->exptab[60].im = s->exptab[19].re;
s->exptab[61].re = s->exptab[61].im = s->exptab[19].im;
s->exptab[62].re = s->exptab[62].im = s->exptab[20].re;
s->exptab[63].re = s->exptab[63].im = s->exptab[20].im;
for (k = 0; k < 5; k++) {
tmp[6*k + 0] = s->exptab[k + 0];
tmp[6*k + 2] = s->exptab[k + 5];
tmp[6*k + 4] = s->exptab[k + 10];
tmp[6*k + 1] = s->exptab[2 * (k + 0)];
tmp[6*k + 3] = s->exptab[2 * (k + 5)];
tmp[6*k + 5] = s->exptab[2 * k + 5 ];
}
for (k = 0; k < 6; k++) {
FFTComplex ac_exp[] = {
{ tmp[6*1 + k].re, tmp[6*1 + k].re },
{ tmp[6*2 + k].re, tmp[6*2 + k].re },
{ tmp[6*3 + k].re, tmp[6*3 + k].re },
{ tmp[6*4 + k].re, tmp[6*4 + k].re },
{ tmp[6*1 + k].im, -tmp[6*1 + k].im },
{ tmp[6*2 + k].im, -tmp[6*2 + k].im },
{ tmp[6*3 + k].im, -tmp[6*3 + k].im },
{ tmp[6*4 + k].im, -tmp[6*4 + k].im },
};
memcpy(s->exptab + 8*k, ac_exp, 8*sizeof(FFTComplex));
}
for (k = 0; k < 3; k++) {
FFTComplex dc_exp[] = {
{ tmp[2*k + 0].re, -tmp[2*k + 0].im },
{ tmp[2*k + 0].im, tmp[2*k + 0].re },
{ tmp[2*k + 1].re, -tmp[2*k + 1].im },
{ tmp[2*k + 1].im, tmp[2*k + 1].re },
};
memcpy(s->exptab + 8*6 + 4*k, dc_exp, 4*sizeof(FFTComplex));
}
}
av_cold void ff_mdct15_init_x86(MDCT15Context *s)
{
int adjust_twiddles = 0;
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE3(cpu_flags))
s->postreindex = ff_mdct15_postreindex_sse3;
if (ARCH_X86_64 && EXTERNAL_AVX(cpu_flags)) {
s->fft15 = ff_fft15_avx;
adjust_twiddles = 1;
}
if (ARCH_X86_64 && EXTERNAL_AVX2_FAST(cpu_flags))
s->postreindex = ff_mdct15_postreindex_avx2;
if (adjust_twiddles)
perm_twiddles(s);
}
