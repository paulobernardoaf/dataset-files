




















#include "pixblockdsp_mips.h"

#if HAVE_MSA
static av_cold void pixblockdsp_init_msa(PixblockDSPContext *c,
AVCodecContext *avctx,
unsigned high_bit_depth)
{
c->diff_pixels = ff_diff_pixels_msa;

switch (avctx->bits_per_raw_sample) {
case 9:
case 10:
case 12:
case 14:
c->get_pixels = ff_get_pixels_16_msa;
break;
default:
if (avctx->bits_per_raw_sample <= 8 || avctx->codec_type !=
AVMEDIA_TYPE_VIDEO) {
c->get_pixels = ff_get_pixels_8_msa;
}
break;
}
}
#endif 

#if HAVE_MMI
static av_cold void pixblockdsp_init_mmi(PixblockDSPContext *c,
AVCodecContext *avctx, unsigned high_bit_depth)
{
c->diff_pixels = ff_diff_pixels_mmi;

if (!high_bit_depth || avctx->codec_type != AVMEDIA_TYPE_VIDEO) {
c->get_pixels = ff_get_pixels_8_mmi;
}
}
#endif 

void ff_pixblockdsp_init_mips(PixblockDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
#if HAVE_MMI
pixblockdsp_init_mmi(c, avctx, high_bit_depth);
#endif 
#if HAVE_MSA
pixblockdsp_init_msa(c, avctx, high_bit_depth);
#endif 
}
