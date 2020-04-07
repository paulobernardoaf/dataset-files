



















#include "libavcodec/takdsp.h"
#include "libavutil/x86/cpu.h"
#include "config.h"

void ff_tak_decorrelate_ls_sse2(int32_t *p1, int32_t *p2, int length);
void ff_tak_decorrelate_sr_sse2(int32_t *p1, int32_t *p2, int length);
void ff_tak_decorrelate_sm_sse2(int32_t *p1, int32_t *p2, int length);
void ff_tak_decorrelate_sf_sse4(int32_t *p1, int32_t *p2, int length, int dshift, int dfactor);

av_cold void ff_takdsp_init_x86(TAKDSPContext *c)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE2(cpu_flags)) {
c->decorrelate_ls = ff_tak_decorrelate_ls_sse2;
c->decorrelate_sr = ff_tak_decorrelate_sr_sse2;
c->decorrelate_sm = ff_tak_decorrelate_sm_sse2;
}

if (EXTERNAL_SSE4(cpu_flags)) {
c->decorrelate_sf = ff_tak_decorrelate_sf_sse4;
}
#endif
}
