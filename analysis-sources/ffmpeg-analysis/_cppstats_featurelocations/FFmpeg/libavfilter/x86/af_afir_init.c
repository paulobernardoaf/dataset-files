

















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/af_afir.h"

void ff_fcmul_add_sse3(float *sum, const float *t, const float *c,
ptrdiff_t len);
void ff_fcmul_add_avx(float *sum, const float *t, const float *c,
ptrdiff_t len);

av_cold void ff_afir_init_x86(AudioFIRDSPContext *s)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE3(cpu_flags)) {
s->fcmul_add = ff_fcmul_add_sse3;
}
if (EXTERNAL_AVX_FAST(cpu_flags)) {
s->fcmul_add = ff_fcmul_add_avx;
}
}
