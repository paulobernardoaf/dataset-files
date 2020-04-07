



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/ppc/cpu.h"
#include "libavutil/ppc/util_altivec.h"
#include "libavcodec/fmtconvert.h"

#if HAVE_ALTIVEC

static void int32_to_float_fmul_scalar_altivec(float *dst, const int32_t *src,
float mul, int len)
{
union {
vector float v;
float s[4];
} mul_u;
int i;
vector float src1, src2, dst1, dst2, mul_v, zero;

zero = (vector float)vec_splat_u32(0);
mul_u.s[0] = mul;
mul_v = vec_splat(mul_u.v, 0);

for (i = 0; i < len; i += 8) {
src1 = vec_ctf(vec_ld(0, src+i), 0);
src2 = vec_ctf(vec_ld(16, src+i), 0);
dst1 = vec_madd(src1, mul_v, zero);
dst2 = vec_madd(src2, mul_v, zero);
vec_st(dst1, 0, dst+i);
vec_st(dst2, 16, dst+i);
}
}

#endif 

av_cold void ff_fmt_convert_init_ppc(FmtConvertContext *c,
AVCodecContext *avctx)
{
#if HAVE_ALTIVEC
if (!PPC_ALTIVEC(av_get_cpu_flags()))
return;

c->int32_to_float_fmul_scalar = int32_to_float_fmul_scalar_altivec;
#endif 
}
