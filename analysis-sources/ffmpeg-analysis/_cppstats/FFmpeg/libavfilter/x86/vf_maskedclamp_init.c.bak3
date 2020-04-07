



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/maskedclamp.h"

void ff_maskedclamp8_sse2(const uint8_t *bsrc, uint8_t *dst,
const uint8_t *darksrc, const uint8_t *brightsrc,
int w, int undershoot, int overshoot);

void ff_maskedclamp16_sse4(const uint8_t *bsrc, uint8_t *dst,
const uint8_t *darksrc, const uint8_t *brightsrc,
int w, int undershoot, int overshoot);

av_cold void ff_maskedclamp_init_x86(MaskedClampDSPContext *dsp, int depth)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags) && depth <= 8) {
dsp->maskedclamp = ff_maskedclamp8_sse2;
}

if (EXTERNAL_SSE4(cpu_flags) && depth > 8) {
dsp->maskedclamp = ff_maskedclamp16_sse4;
}
}
