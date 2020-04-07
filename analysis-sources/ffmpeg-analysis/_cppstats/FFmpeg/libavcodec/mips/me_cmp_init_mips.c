#include "me_cmp_mips.h"
#if HAVE_MSA
static av_cold void me_cmp_msa(MECmpContext *c, AVCodecContext *avctx)
{
#if BIT_DEPTH == 8
c->pix_abs[0][0] = ff_pix_abs16_msa;
c->pix_abs[0][1] = ff_pix_abs16_x2_msa;
c->pix_abs[0][2] = ff_pix_abs16_y2_msa;
c->pix_abs[0][3] = ff_pix_abs16_xy2_msa;
c->pix_abs[1][0] = ff_pix_abs8_msa;
c->pix_abs[1][1] = ff_pix_abs8_x2_msa;
c->pix_abs[1][2] = ff_pix_abs8_y2_msa;
c->pix_abs[1][3] = ff_pix_abs8_xy2_msa;
c->hadamard8_diff[0] = ff_hadamard8_diff16_msa;
c->hadamard8_diff[1] = ff_hadamard8_diff8x8_msa;
c->hadamard8_diff[4] = ff_hadamard8_intra16_msa;
c->hadamard8_diff[5] = ff_hadamard8_intra8x8_msa;
c->sad[0] = ff_pix_abs16_msa;
c->sad[1] = ff_pix_abs8_msa;
c->sse[0] = ff_sse16_msa;
c->sse[1] = ff_sse8_msa;
c->sse[2] = ff_sse4_msa;
#endif
}
#endif 
av_cold void ff_me_cmp_init_mips(MECmpContext *c, AVCodecContext *avctx)
{
#if HAVE_MSA
me_cmp_msa(c, avctx);
#endif 
}
