

















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/pixblockdsp.h"

void ff_get_pixels_armv6(int16_t *block, const uint8_t *pixels,
ptrdiff_t stride);
void ff_diff_pixels_armv6(int16_t *block, const uint8_t *s1,
const uint8_t *s2, ptrdiff_t stride);

av_cold void ff_pixblockdsp_init_arm(PixblockDSPContext *c,
AVCodecContext *avctx,
unsigned high_bit_depth)
{
int cpu_flags = av_get_cpu_flags();

if (have_armv6(cpu_flags)) {
if (!high_bit_depth)
c->get_pixels = ff_get_pixels_armv6;
c->diff_pixels = ff_diff_pixels_armv6;
}
}
