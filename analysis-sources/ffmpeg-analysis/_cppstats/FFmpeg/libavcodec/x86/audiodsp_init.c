#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/audiodsp.h"
int32_t ff_scalarproduct_int16_mmxext(const int16_t *v1, const int16_t *v2,
int order);
int32_t ff_scalarproduct_int16_sse2(const int16_t *v1, const int16_t *v2,
int order);
void ff_vector_clip_int32_mmx(int32_t *dst, const int32_t *src,
int32_t min, int32_t max, unsigned int len);
void ff_vector_clip_int32_sse2(int32_t *dst, const int32_t *src,
int32_t min, int32_t max, unsigned int len);
void ff_vector_clip_int32_int_sse2(int32_t *dst, const int32_t *src,
int32_t min, int32_t max, unsigned int len);
void ff_vector_clip_int32_sse4(int32_t *dst, const int32_t *src,
int32_t min, int32_t max, unsigned int len);
void ff_vector_clipf_sse(float *dst, const float *src,
int len, float min, float max);
av_cold void ff_audiodsp_init_x86(AudioDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_MMX(cpu_flags))
c->vector_clip_int32 = ff_vector_clip_int32_mmx;
if (EXTERNAL_MMXEXT(cpu_flags))
c->scalarproduct_int16 = ff_scalarproduct_int16_mmxext;
if (EXTERNAL_SSE(cpu_flags))
c->vector_clipf = ff_vector_clipf_sse;
if (EXTERNAL_SSE2(cpu_flags)) {
c->scalarproduct_int16 = ff_scalarproduct_int16_sse2;
if (cpu_flags & AV_CPU_FLAG_ATOM)
c->vector_clip_int32 = ff_vector_clip_int32_int_sse2;
else
c->vector_clip_int32 = ff_vector_clip_int32_sse2;
}
if (EXTERNAL_SSE4(cpu_flags))
c->vector_clip_int32 = ff_vector_clip_int32_sse4;
}
