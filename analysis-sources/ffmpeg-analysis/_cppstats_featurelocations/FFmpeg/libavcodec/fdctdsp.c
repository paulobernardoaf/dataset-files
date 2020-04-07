

















#include "libavutil/attributes.h"
#include "avcodec.h"
#include "dct.h"
#include "faandct.h"
#include "fdctdsp.h"
#include "config.h"

av_cold void ff_fdctdsp_init(FDCTDSPContext *c, AVCodecContext *avctx)
{
const unsigned high_bit_depth = avctx->bits_per_raw_sample > 8;

if (avctx->bits_per_raw_sample == 10 || avctx->bits_per_raw_sample == 9) {
c->fdct = ff_jpeg_fdct_islow_10;
c->fdct248 = ff_fdct248_islow_10;
} else if (avctx->dct_algo == FF_DCT_FASTINT) {
c->fdct = ff_fdct_ifast;
c->fdct248 = ff_fdct_ifast248;
#if CONFIG_FAANDCT
} else if (avctx->dct_algo == FF_DCT_FAAN) {
c->fdct = ff_faandct;
c->fdct248 = ff_faandct248;
#endif 
} else {
c->fdct = ff_jpeg_fdct_islow_8; 
c->fdct248 = ff_fdct248_islow_8;
}

if (ARCH_PPC)
ff_fdctdsp_init_ppc(c, avctx, high_bit_depth);
if (ARCH_X86)
ff_fdctdsp_init_x86(c, avctx, high_bit_depth);
}
