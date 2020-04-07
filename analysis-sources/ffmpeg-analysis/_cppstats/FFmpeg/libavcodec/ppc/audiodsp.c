#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"
#include "libavcodec/audiodsp.h"
#if HAVE_ALTIVEC
static int32_t scalarproduct_int16_altivec(const int16_t *v1, const int16_t *v2,
int order)
{
int i;
LOAD_ZERO;
register vec_s16 vec1;
register vec_s32 res = vec_splat_s32(0), t;
int32_t ires;
for (i = 0; i < order; i += 8) {
vec1 = vec_unaligned_load(v1);
t = vec_msum(vec1, vec_ld(0, v2), zero_s32v);
res = vec_sums(t, res);
v1 += 8;
v2 += 8;
}
res = vec_splat(res, 3);
vec_ste(res, 0, &ires);
return ires;
}
#endif 
#if HAVE_VSX
static int32_t scalarproduct_int16_vsx(const int16_t *v1, const int16_t *v2, int order)
{
int i;
LOAD_ZERO;
register vec_s16 vec1;
register vec_s32 res = vec_splat_s32(0), t;
int32_t ires;
for (i = 0; i < order; i += 8) {
vec1 = vec_vsx_ld(0, v1);
t = vec_msum(vec1, vec_ld(0, v2), zero_s32v);
res = vec_sums(t, res);
v1 += 8;
v2 += 8;
}
res = vec_splat(res, 3);
vec_ste(res, 0, &ires);
return ires;
}
#endif 
av_cold void ff_audiodsp_init_ppc(AudioDSPContext *c)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;
c->scalarproduct_int16 = scalarproduct_int16_altivec;
#endif 
#if HAVE_VSX
if (!PPC_VSX(av_get_cpu_flags()))
return;
c->scalarproduct_int16 = scalarproduct_int16_vsx;
#endif 
}
