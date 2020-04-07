



















#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/atadenoise.h"

void ff_atadenoise_filter_row8_sse4(const uint8_t *src, uint8_t *dst,
const uint8_t **srcf,
int w, int mid, int size,
int thra, int thrb);

void ff_atadenoise_filter_row8_serial_sse4(const uint8_t *src, uint8_t *dst,
const uint8_t **srcf,
int w, int mid, int size,
int thra, int thrb);

av_cold void ff_atadenoise_init_x86(ATADenoiseDSPContext *dsp, int depth, int algorithm)
{
int cpu_flags = av_get_cpu_flags();

if (ARCH_X86_64 && EXTERNAL_SSE4(cpu_flags) && depth <= 8 && algorithm == PARALLEL) {
dsp->filter_row = ff_atadenoise_filter_row8_sse4;
}

if (ARCH_X86_64 && EXTERNAL_SSE4(cpu_flags) && depth <= 8 && algorithm == SERIAL) {
dsp->filter_row = ff_atadenoise_filter_row8_serial_sse4;
}
}
