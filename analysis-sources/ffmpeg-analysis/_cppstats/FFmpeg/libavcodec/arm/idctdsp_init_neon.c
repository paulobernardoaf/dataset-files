#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/idctdsp.h"
#include "idct.h"
#include "idctdsp_arm.h"
void ff_add_pixels_clamped_neon(const int16_t *, uint8_t *, ptrdiff_t);
void ff_put_pixels_clamped_neon(const int16_t *, uint8_t *, ptrdiff_t);
void ff_put_signed_pixels_clamped_neon(const int16_t *, uint8_t *, ptrdiff_t);
av_cold void ff_idctdsp_init_neon(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if (!avctx->lowres && !high_bit_depth) {
if (avctx->idct_algo == FF_IDCT_AUTO ||
avctx->idct_algo == FF_IDCT_SIMPLEAUTO ||
avctx->idct_algo == FF_IDCT_SIMPLENEON) {
c->idct_put = ff_simple_idct_put_neon;
c->idct_add = ff_simple_idct_add_neon;
c->idct = ff_simple_idct_neon;
c->perm_type = FF_IDCT_PERM_PARTTRANS;
}
}
c->add_pixels_clamped = ff_add_pixels_clamped_neon;
c->put_pixels_clamped = ff_put_pixels_clamped_neon;
c->put_signed_pixels_clamped = ff_put_signed_pixels_clamped_neon;
}
