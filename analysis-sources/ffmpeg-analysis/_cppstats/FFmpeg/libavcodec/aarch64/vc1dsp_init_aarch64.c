#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/aarch64/cpu.h"
#include "libavcodec/vc1dsp.h"
#include "config.h"
void ff_put_vc1_chroma_mc8_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
void ff_avg_vc1_chroma_mc8_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
void ff_put_vc1_chroma_mc4_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
void ff_avg_vc1_chroma_mc4_neon(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
int h, int x, int y);
av_cold void ff_vc1dsp_init_aarch64(VC1DSPContext *dsp)
{
int cpu_flags = av_get_cpu_flags();
if (have_neon(cpu_flags)) {
dsp->put_no_rnd_vc1_chroma_pixels_tab[0] = ff_put_vc1_chroma_mc8_neon;
dsp->avg_no_rnd_vc1_chroma_pixels_tab[0] = ff_avg_vc1_chroma_mc8_neon;
dsp->put_no_rnd_vc1_chroma_pixels_tab[1] = ff_put_vc1_chroma_mc4_neon;
dsp->avg_no_rnd_vc1_chroma_pixels_tab[1] = ff_avg_vc1_chroma_mc4_neon;
}
}
