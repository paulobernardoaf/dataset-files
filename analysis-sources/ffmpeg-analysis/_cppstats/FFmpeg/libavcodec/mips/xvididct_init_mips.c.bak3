



















#include "xvididct_mips.h"

#if HAVE_MMI
static av_cold void xvid_idct_init_mmi(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
if (!high_bit_depth) {
if (avctx->idct_algo == FF_IDCT_AUTO ||
avctx->idct_algo == FF_IDCT_XVID) {
c->idct_put = ff_xvid_idct_put_mmi;
c->idct_add = ff_xvid_idct_add_mmi;
c->idct = ff_xvid_idct_mmi;
c->perm_type = FF_IDCT_PERM_NONE;
}
}
}
#endif 

av_cold void ff_xvid_idct_init_mips(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
#if HAVE_MMI
xvid_idct_init_mmi(c, avctx, high_bit_depth);
#endif 
}
