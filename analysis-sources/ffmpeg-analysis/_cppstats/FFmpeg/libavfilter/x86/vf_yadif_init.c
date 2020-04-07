#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/yadif.h"
void ff_yadif_filter_line_mmxext(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_sse2(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_ssse3(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_16bit_mmxext(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_16bit_sse2(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_16bit_ssse3(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_16bit_sse4(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_10bit_mmxext(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_10bit_sse2(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
void ff_yadif_filter_line_10bit_ssse3(void *dst, void *prev, void *cur,
void *next, int w, int prefs,
int mrefs, int parity, int mode);
av_cold void ff_yadif_init_x86(YADIFContext *yadif)
{
int cpu_flags = av_get_cpu_flags();
int bit_depth = (!yadif->csp) ? 8
: yadif->csp->comp[0].depth;
if (bit_depth >= 15) {
#if ARCH_X86_32
if (EXTERNAL_MMXEXT(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_16bit_mmxext;
#endif 
if (EXTERNAL_SSE2(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_16bit_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_16bit_ssse3;
if (EXTERNAL_SSE4(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_16bit_sse4;
} else if ( bit_depth >= 9 && bit_depth <= 14) {
#if ARCH_X86_32
if (EXTERNAL_MMXEXT(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_10bit_mmxext;
#endif 
if (EXTERNAL_SSE2(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_10bit_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_10bit_ssse3;
} else {
#if ARCH_X86_32
if (EXTERNAL_MMXEXT(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_mmxext;
#endif 
if (EXTERNAL_SSE2(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
yadif->filter_line = ff_yadif_filter_line_ssse3;
}
}
