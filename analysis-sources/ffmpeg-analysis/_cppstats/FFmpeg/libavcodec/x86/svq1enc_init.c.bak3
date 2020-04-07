



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/svq1enc.h"

int ff_ssd_int8_vs_int16_mmx(const int8_t *pix1, const int16_t *pix2,
intptr_t size);
int ff_ssd_int8_vs_int16_sse2(const int8_t *pix1, const int16_t *pix2,
intptr_t size);

av_cold void ff_svq1enc_init_x86(SVQ1EncContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_MMX(cpu_flags)) {
c->ssd_int8_vs_int16 = ff_ssd_int8_vs_int16_mmx;
}
if (EXTERNAL_SSE2(cpu_flags)) {
c->ssd_int8_vs_int16 = ff_ssd_int8_vs_int16_sse2;
}
}
