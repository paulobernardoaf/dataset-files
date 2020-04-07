#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/synth_filter.h"
#define SYNTH_FILTER_FUNC(opt) void ff_synth_filter_inner_##opt(float *synth_buf_ptr, float synth_buf2[32], const float window[512], float out[32], intptr_t offset, float scale); static void synth_filter_##opt(FFTContext *imdct, float *synth_buf_ptr, int *synth_buf_offset, float synth_buf2[32], const float window[512], float out[32], const float in[32], float scale) { float *synth_buf= synth_buf_ptr + *synth_buf_offset; imdct->imdct_half(imdct, synth_buf, in); ff_synth_filter_inner_##opt(synth_buf, synth_buf2, window, out, *synth_buf_offset, scale); *synth_buf_offset = (*synth_buf_offset - 32) & 511; } 
#if HAVE_X86ASM
#if ARCH_X86_32
SYNTH_FILTER_FUNC(sse)
#endif
SYNTH_FILTER_FUNC(sse2)
SYNTH_FILTER_FUNC(avx)
SYNTH_FILTER_FUNC(fma3)
#endif 
av_cold void ff_synth_filter_init_x86(SynthFilterContext *s)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();
#if ARCH_X86_32
if (EXTERNAL_SSE(cpu_flags)) {
s->synth_filter_float = synth_filter_sse;
}
#endif
if (EXTERNAL_SSE2(cpu_flags)) {
s->synth_filter_float = synth_filter_sse2;
}
if (EXTERNAL_AVX_FAST(cpu_flags)) {
s->synth_filter_float = synth_filter_avx;
}
if (EXTERNAL_FMA3_FAST(cpu_flags)) {
s->synth_filter_float = synth_filter_fma3;
}
#endif 
}
