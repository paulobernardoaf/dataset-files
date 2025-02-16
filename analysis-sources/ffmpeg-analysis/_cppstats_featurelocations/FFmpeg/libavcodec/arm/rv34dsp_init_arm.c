



















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/rv34dsp.h"
#include "libavutil/arm/cpu.h"

void ff_rv34_inv_transform_noround_neon(int16_t *block);

void ff_rv34_inv_transform_noround_dc_neon(int16_t *block);

void ff_rv34_idct_add_neon(uint8_t *dst, ptrdiff_t stride, int16_t *block);
void ff_rv34_idct_dc_add_neon(uint8_t *dst, ptrdiff_t stride, int dc);

av_cold void ff_rv34dsp_init_arm(RV34DSPContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags)) {
c->rv34_inv_transform = ff_rv34_inv_transform_noround_neon;
c->rv34_inv_transform_dc = ff_rv34_inv_transform_noround_dc_neon;

c->rv34_idct_add = ff_rv34_idct_add_neon;
c->rv34_idct_dc_add = ff_rv34_idct_dc_add_neon;
}
}
