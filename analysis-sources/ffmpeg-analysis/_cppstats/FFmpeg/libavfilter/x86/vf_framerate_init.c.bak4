

















#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/framerate.h"

void ff_blend_frames_ssse3(BLEND_FUNC_PARAMS);
void ff_blend_frames_avx2(BLEND_FUNC_PARAMS);
void ff_blend_frames16_sse4(BLEND_FUNC_PARAMS);
void ff_blend_frames16_avx2(BLEND_FUNC_PARAMS);

void ff_framerate_init_x86(FrameRateContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (s->bitdepth == 8) {
if (EXTERNAL_AVX2_FAST(cpu_flags))
s->blend = ff_blend_frames_avx2;
else if (EXTERNAL_SSSE3(cpu_flags))
s->blend = ff_blend_frames_ssse3;
} else {
if (EXTERNAL_AVX2_FAST(cpu_flags))
s->blend = ff_blend_frames16_avx2;
else if (EXTERNAL_SSE4(cpu_flags))
s->blend = ff_blend_frames16_sse4;
}
}
