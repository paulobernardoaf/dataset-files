#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/lossless_audiodsp.h"
int32_t ff_scalarproduct_and_madd_int16_neon(int16_t *v1, const int16_t *v2,
const int16_t *v3, int len, int mul);
av_cold void ff_llauddsp_init_arm(LLAudDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags)) {
c->scalarproduct_and_madd_int16 = ff_scalarproduct_and_madd_int16_neon;
}
}
