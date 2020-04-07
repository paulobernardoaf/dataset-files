#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/bwdif.h"
void ff_bwdif_filter_line_mmxext(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
void ff_bwdif_filter_line_sse2(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
void ff_bwdif_filter_line_ssse3(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
void ff_bwdif_filter_line_12bit_mmxext(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
void ff_bwdif_filter_line_12bit_sse2(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
void ff_bwdif_filter_line_12bit_ssse3(void *dst, void *prev, void *cur, void *next,
int w, int prefs, int mrefs, int prefs2,
int mrefs2, int prefs3, int mrefs3, int prefs4,
int mrefs4, int parity, int clip_max);
av_cold void ff_bwdif_init_x86(BWDIFContext *bwdif)
{
YADIFContext *yadif = &bwdif->yadif;
int cpu_flags = av_get_cpu_flags();
int bit_depth = (!yadif->csp) ? 8 : yadif->csp->comp[0].depth;
if (bit_depth <= 8) {
#if ARCH_X86_32
if (EXTERNAL_MMXEXT(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_mmxext;
#endif 
if (EXTERNAL_SSE2(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_ssse3;
} else if (bit_depth <= 12) {
#if ARCH_X86_32
if (EXTERNAL_MMXEXT(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_12bit_mmxext;
#endif 
if (EXTERNAL_SSE2(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_12bit_sse2;
if (EXTERNAL_SSSE3(cpu_flags))
bwdif->filter_line = ff_bwdif_filter_line_12bit_ssse3;
}
}
