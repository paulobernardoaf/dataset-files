#include "idctdsp_mips.h"
#include "xvididct_mips.h"
#if HAVE_MSA
static av_cold void idctdsp_init_msa(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if ((avctx->lowres != 1) && (avctx->lowres != 2) && (avctx->lowres != 3) &&
(avctx->bits_per_raw_sample != 10) &&
(avctx->bits_per_raw_sample != 12) &&
(avctx->idct_algo == FF_IDCT_AUTO)) {
c->idct_put = ff_simple_idct_put_msa;
c->idct_add = ff_simple_idct_add_msa;
c->idct = ff_simple_idct_msa;
c->perm_type = FF_IDCT_PERM_NONE;
}
c->put_pixels_clamped = ff_put_pixels_clamped_msa;
c->put_signed_pixels_clamped = ff_put_signed_pixels_clamped_msa;
c->add_pixels_clamped = ff_add_pixels_clamped_msa;
}
#endif 
#if HAVE_MMI
static av_cold void idctdsp_init_mmi(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if ((avctx->lowres != 1) && (avctx->lowres != 2) && (avctx->lowres != 3) &&
(avctx->bits_per_raw_sample != 10) &&
(avctx->bits_per_raw_sample != 12) &&
((avctx->idct_algo == FF_IDCT_AUTO) || (avctx->idct_algo == FF_IDCT_SIMPLE))) {
c->idct_put = ff_simple_idct_put_8_mmi;
c->idct_add = ff_simple_idct_add_8_mmi;
c->idct = ff_simple_idct_8_mmi;
c->perm_type = FF_IDCT_PERM_NONE;
}
c->put_pixels_clamped = ff_put_pixels_clamped_mmi;
c->add_pixels_clamped = ff_add_pixels_clamped_mmi;
c->put_signed_pixels_clamped = ff_put_signed_pixels_clamped_mmi;
}
#endif 
av_cold void ff_idctdsp_init_mips(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
#if HAVE_MMI
idctdsp_init_mmi(c, avctx, high_bit_depth);
#endif 
#if HAVE_MSA
idctdsp_init_msa(c, avctx, high_bit_depth);
#endif 
}
