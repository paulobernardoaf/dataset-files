

















#include <stdint.h>

#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/mpegvideoencdsp.h"

int ff_pix_norm1_armv6(uint8_t *pix, int line_size);
int ff_pix_sum_armv6(uint8_t *pix, int line_size);

av_cold void ff_mpegvideoencdsp_init_arm(MpegvideoEncDSPContext *c,
AVCodecContext *avctx)
{
int cpu_flags = av_get_cpu_flags();

if (have_armv6(cpu_flags)) {
c->pix_norm1 = ff_pix_norm1_armv6;
c->pix_sum = ff_pix_sum_armv6;
}
}
