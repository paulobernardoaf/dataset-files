#include "libavcodec/bit_depth_template.c"
#include "h263dsp_mips.h"
#if HAVE_MSA
static av_cold void mpegvideoencdsp_init_msa(MpegvideoEncDSPContext *c,
AVCodecContext *avctx)
{
#if BIT_DEPTH == 8
c->pix_sum = ff_pix_sum_msa;
#endif
}
#endif 
av_cold void ff_mpegvideoencdsp_init_mips(MpegvideoEncDSPContext *c,
AVCodecContext *avctx)
{
#if HAVE_MSA
mpegvideoencdsp_init_msa(c, avctx);
#endif 
}
