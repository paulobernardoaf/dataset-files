























#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/fmtconvert.h"

#if HAVE_X86ASM

void ff_int32_to_float_fmul_scalar_sse (float *dst, const int32_t *src, float mul, int len);
void ff_int32_to_float_fmul_scalar_sse2(float *dst, const int32_t *src, float mul, int len);
void ff_int32_to_float_fmul_array8_sse (FmtConvertContext *c, float *dst, const int32_t *src,
const float *mul, int len);
void ff_int32_to_float_fmul_array8_sse2(FmtConvertContext *c, float *dst, const int32_t *src,
const float *mul, int len);

#endif 

av_cold void ff_fmt_convert_init_x86(FmtConvertContext *c, AVCodecContext *avctx)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_SSE(cpu_flags)) {
c->int32_to_float_fmul_scalar = ff_int32_to_float_fmul_scalar_sse;
c->int32_to_float_fmul_array8 = ff_int32_to_float_fmul_array8_sse;
}
if (EXTERNAL_SSE2(cpu_flags)) {
c->int32_to_float_fmul_scalar = ff_int32_to_float_fmul_scalar_sse2;
c->int32_to_float_fmul_array8 = ff_int32_to_float_fmul_array8_sse2;
}
#endif 
}
