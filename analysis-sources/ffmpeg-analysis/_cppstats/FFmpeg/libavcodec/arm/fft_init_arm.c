#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/fft.h"
void ff_fft_calc_vfp(FFTContext *s, FFTComplex *z);
void ff_fft_permute_neon(FFTContext *s, FFTComplex *z);
void ff_fft_calc_neon(FFTContext *s, FFTComplex *z);
void ff_imdct_half_vfp(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_imdct_calc_neon(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_imdct_half_neon(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_mdct_calc_neon(FFTContext *s, FFTSample *output, const FFTSample *input);
av_cold void ff_fft_init_arm(FFTContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (have_vfp_vm(cpu_flags)) {
s->fft_calc = ff_fft_calc_vfp;
#if CONFIG_MDCT
s->imdct_half = ff_imdct_half_vfp;
#endif
}
if (have_neon(cpu_flags)) {
#if CONFIG_FFT
s->fft_permute = ff_fft_permute_neon;
s->fft_calc = ff_fft_calc_neon;
#endif
#if CONFIG_MDCT
s->imdct_calc = ff_imdct_calc_neon;
s->imdct_half = ff_imdct_half_neon;
s->mdct_calc = ff_mdct_calc_neon;
s->mdct_permutation = FF_MDCT_PERM_INTERLEAVE;
#endif
}
}
