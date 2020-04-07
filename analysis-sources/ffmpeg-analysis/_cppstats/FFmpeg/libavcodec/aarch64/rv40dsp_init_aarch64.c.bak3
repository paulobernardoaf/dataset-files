

















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/aarch64/cpu.h"
#include "libavcodec/rv34dsp.h"

#include "config.h"

void ff_put_rv40_chroma_mc8_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
void ff_put_rv40_chroma_mc4_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);

void ff_avg_rv40_chroma_mc8_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
void ff_avg_rv40_chroma_mc4_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);

av_cold void ff_rv40dsp_init_aarch64(RV34DSPContext *c)
{
int cpu_flags = av_get_cpu_flags();

if (have_neon(cpu_flags)) {
c->put_chroma_pixels_tab[0] = ff_put_rv40_chroma_mc8_neon;
c->put_chroma_pixels_tab[1] = ff_put_rv40_chroma_mc4_neon;
c->avg_chroma_pixels_tab[0] = ff_avg_rv40_chroma_mc8_neon;
c->avg_chroma_pixels_tab[1] = ff_avg_rv40_chroma_mc4_neon;
}
}
