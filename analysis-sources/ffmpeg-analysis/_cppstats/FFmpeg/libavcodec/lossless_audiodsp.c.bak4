





















#include "avcodec.h"
#include "lossless_audiodsp.h"

static int32_t scalarproduct_and_madd_int16_c(int16_t *v1, const int16_t *v2,
const int16_t *v3,
int order, int mul)
{
int res = 0;

do {
res += *v1 * *v2++;
*v1++ += mul * *v3++;
res += *v1 * *v2++;
*v1++ += mul * *v3++;
} while (order-=2);
return res;
}

static int32_t scalarproduct_and_madd_int32_c(int16_t *v1, const int32_t *v2,
const int16_t *v3,
int order, int mul)
{
int res = 0;

do {
res += *v1 * (uint32_t)*v2++;
*v1++ += mul * *v3++;
res += *v1 * (uint32_t)*v2++;
*v1++ += mul * *v3++;
} while (order-=2);
return res;
}

av_cold void ff_llauddsp_init(LLAudDSPContext *c)
{
c->scalarproduct_and_madd_int16 = scalarproduct_and_madd_int16_c;
c->scalarproduct_and_madd_int32 = scalarproduct_and_madd_int32_c;

if (ARCH_ARM)
ff_llauddsp_init_arm(c);
if (ARCH_PPC)
ff_llauddsp_init_ppc(c);
if (ARCH_X86)
ff_llauddsp_init_x86(c);
}
