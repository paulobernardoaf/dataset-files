#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/tinterlace.h"
void ff_lowpass_line_sse2(uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_avx (uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_avx2 (uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_16_sse2(uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_16_avx (uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_16_avx2 (uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_complex_sse2(uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
void ff_lowpass_line_complex_12_sse2(uint8_t *dstp, ptrdiff_t linesize,
const uint8_t *srcp, ptrdiff_t mref,
ptrdiff_t pref, int clip_max);
av_cold void ff_tinterlace_init_x86(TInterlaceContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (s->csp->comp[0].depth > 8) {
if (EXTERNAL_SSE2(cpu_flags)) {
if (!(s->flags & TINTERLACE_FLAG_CVLPF))
s->lowpass_line = ff_lowpass_line_16_sse2;
else
s->lowpass_line = ff_lowpass_line_complex_12_sse2;
}
if (EXTERNAL_AVX(cpu_flags))
if (!(s->flags & TINTERLACE_FLAG_CVLPF))
s->lowpass_line = ff_lowpass_line_16_avx;
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
if (!(s->flags & TINTERLACE_FLAG_CVLPF)) {
s->lowpass_line = ff_lowpass_line_16_avx2;
}
}
} else {
if (EXTERNAL_SSE2(cpu_flags)) {
if (!(s->flags & TINTERLACE_FLAG_CVLPF))
s->lowpass_line = ff_lowpass_line_sse2;
else
s->lowpass_line = ff_lowpass_line_complex_sse2;
}
if (EXTERNAL_AVX(cpu_flags))
if (!(s->flags & TINTERLACE_FLAG_CVLPF))
s->lowpass_line = ff_lowpass_line_avx;
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
if (!(s->flags & TINTERLACE_FLAG_CVLPF)) {
s->lowpass_line = ff_lowpass_line_avx2;
}
}
}
}
