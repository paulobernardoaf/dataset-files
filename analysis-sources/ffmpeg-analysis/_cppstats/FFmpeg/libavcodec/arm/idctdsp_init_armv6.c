#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/idctdsp.h"
#include "idct.h"
#include "idctdsp_arm.h"
void ff_add_pixels_clamped_armv6(const int16_t *block, uint8_t *pixels,
ptrdiff_t line_size);
av_cold void ff_idctdsp_init_armv6(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if (!avctx->lowres && !high_bit_depth) {
if ((avctx->idct_algo == FF_IDCT_AUTO && !(avctx->flags & AV_CODEC_FLAG_BITEXACT)) ||
avctx->idct_algo == FF_IDCT_SIMPLEARMV6) {
c->idct_put = ff_simple_idct_put_armv6;
c->idct_add = ff_simple_idct_add_armv6;
c->idct = ff_simple_idct_armv6;
c->perm_type = FF_IDCT_PERM_LIBMPEG2;
}
}
c->add_pixels_clamped = ff_add_pixels_clamped_armv6;
}
