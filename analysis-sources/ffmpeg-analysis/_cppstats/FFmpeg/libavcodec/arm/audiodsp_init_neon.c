#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavcodec/audiodsp.h"
#include "audiodsp_arm.h"
void ff_vector_clipf_neon(float *dst, const float *src, int len, float min, float max);
void ff_vector_clip_int32_neon(int32_t *dst, const int32_t *src, int32_t min,
int32_t max, unsigned int len);
int32_t ff_scalarproduct_int16_neon(const int16_t *v1, const int16_t *v2, int len);
av_cold void ff_audiodsp_init_neon(AudioDSPContext *c)
{
c->vector_clip_int32 = ff_vector_clip_int32_neon;
c->vector_clipf = ff_vector_clipf_neon;
c->scalarproduct_int16 = ff_scalarproduct_int16_neon;
}
