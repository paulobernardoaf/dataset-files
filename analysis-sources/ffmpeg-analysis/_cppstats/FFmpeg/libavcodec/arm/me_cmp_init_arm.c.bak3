

















#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/me_cmp.h"
#include "libavcodec/mpegvideo.h"

int ff_pix_abs16_armv6(MpegEncContext *s, uint8_t *blk1, uint8_t *blk2,
ptrdiff_t stride, int h);
int ff_pix_abs16_x2_armv6(MpegEncContext *s, uint8_t *blk1, uint8_t *blk2,
ptrdiff_t stride, int h);
int ff_pix_abs16_y2_armv6(MpegEncContext *s, uint8_t *blk1, uint8_t *blk2,
ptrdiff_t stride, int h);

int ff_pix_abs8_armv6(MpegEncContext *s, uint8_t *blk1, uint8_t *blk2,
ptrdiff_t stride, int h);

int ff_sse16_armv6(MpegEncContext *s, uint8_t *blk1, uint8_t *blk2,
ptrdiff_t stride, int h);

av_cold void ff_me_cmp_init_arm(MECmpContext *c, AVCodecContext *avctx)
{
int cpu_flags = av_get_cpu_flags();

if (have_armv6(cpu_flags)) {
c->pix_abs[0][0] = ff_pix_abs16_armv6;
c->pix_abs[0][1] = ff_pix_abs16_x2_armv6;
c->pix_abs[0][2] = ff_pix_abs16_y2_armv6;

c->pix_abs[1][0] = ff_pix_abs8_armv6;

c->sad[0] = ff_pix_abs16_armv6;
c->sad[1] = ff_pix_abs8_armv6;

c->sse[0] = ff_sse16_armv6;
}
}
