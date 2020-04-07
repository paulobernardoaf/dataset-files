#include "h263dsp_mips.h"
#include "mpegvideo_mips.h"
#if HAVE_MSA
static av_cold void dct_unquantize_init_msa(MpegEncContext *s)
{
s->dct_unquantize_h263_intra = ff_dct_unquantize_h263_intra_msa;
s->dct_unquantize_h263_inter = ff_dct_unquantize_h263_inter_msa;
if (!s->q_scale_type)
s->dct_unquantize_mpeg2_inter = ff_dct_unquantize_mpeg2_inter_msa;
}
#endif 
#if HAVE_MMI
static av_cold void dct_unquantize_init_mmi(MpegEncContext *s)
{
s->dct_unquantize_h263_intra = ff_dct_unquantize_h263_intra_mmi;
s->dct_unquantize_h263_inter = ff_dct_unquantize_h263_inter_mmi;
s->dct_unquantize_mpeg1_intra = ff_dct_unquantize_mpeg1_intra_mmi;
s->dct_unquantize_mpeg1_inter = ff_dct_unquantize_mpeg1_inter_mmi;
if (!(s->avctx->flags & AV_CODEC_FLAG_BITEXACT))
if (!s->q_scale_type)
s->dct_unquantize_mpeg2_intra = ff_dct_unquantize_mpeg2_intra_mmi;
s->denoise_dct= ff_denoise_dct_mmi;
}
#endif 
av_cold void ff_mpv_common_init_mips(MpegEncContext *s)
{
#if HAVE_MMI
dct_unquantize_init_mmi(s);
#endif 
#if HAVE_MSA
dct_unquantize_init_msa(s);
#endif 
}
