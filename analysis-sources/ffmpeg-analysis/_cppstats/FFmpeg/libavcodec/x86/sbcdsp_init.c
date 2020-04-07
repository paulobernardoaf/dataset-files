#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/sbcdsp.h"
void ff_sbc_analyze_4_mmx(const int16_t *in, int32_t *out, const int16_t *consts);
void ff_sbc_analyze_8_mmx(const int16_t *in, int32_t *out, const int16_t *consts);
void ff_sbc_calc_scalefactors_mmx(int32_t sb_sample_f[16][2][8],
uint32_t scale_factor[2][8],
int blocks, int channels, int subbands);
av_cold void ff_sbcdsp_init_x86(SBCDSPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_MMX(cpu_flags)) {
s->sbc_analyze_4 = ff_sbc_analyze_4_mmx;
s->sbc_analyze_8 = ff_sbc_analyze_8_mmx;
s->sbc_calc_scalefactors = ff_sbc_calc_scalefactors_mmx;
}
}
