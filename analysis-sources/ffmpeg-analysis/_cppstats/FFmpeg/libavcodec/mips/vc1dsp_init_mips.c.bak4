



















#include "libavutil/attributes.h"
#include "libavcodec/vc1dsp.h"
#include "vc1dsp_mips.h"
#include "config.h"

#define FN_ASSIGN(OP, X, Y, INSN) dsp->OP##vc1_mspel_pixels_tab[1][X+4*Y] = ff_##OP##vc1_mspel_mc##X##Y##INSN; dsp->OP##vc1_mspel_pixels_tab[0][X+4*Y] = ff_##OP##vc1_mspel_mc##X##Y##_16##INSN



#if HAVE_MMI
static av_cold void vc1dsp_init_mmi(VC1DSPContext *dsp)
{
#if _MIPS_SIM != _ABIO32
dsp->vc1_inv_trans_8x8 = ff_vc1_inv_trans_8x8_mmi;
dsp->vc1_inv_trans_4x8 = ff_vc1_inv_trans_4x8_mmi;
dsp->vc1_inv_trans_8x4 = ff_vc1_inv_trans_8x4_mmi;
#endif
dsp->vc1_inv_trans_4x4 = ff_vc1_inv_trans_4x4_mmi;
dsp->vc1_inv_trans_8x8_dc = ff_vc1_inv_trans_8x8_dc_mmi;
dsp->vc1_inv_trans_4x8_dc = ff_vc1_inv_trans_4x8_dc_mmi;
dsp->vc1_inv_trans_8x4_dc = ff_vc1_inv_trans_8x4_dc_mmi;
dsp->vc1_inv_trans_4x4_dc = ff_vc1_inv_trans_4x4_dc_mmi;

dsp->vc1_h_overlap = ff_vc1_h_overlap_mmi;
dsp->vc1_v_overlap = ff_vc1_v_overlap_mmi;
dsp->vc1_h_s_overlap = ff_vc1_h_s_overlap_mmi;
dsp->vc1_v_s_overlap = ff_vc1_v_s_overlap_mmi;

dsp->vc1_v_loop_filter4 = ff_vc1_v_loop_filter4_mmi;
dsp->vc1_h_loop_filter4 = ff_vc1_h_loop_filter4_mmi;
dsp->vc1_v_loop_filter8 = ff_vc1_v_loop_filter8_mmi;
dsp->vc1_h_loop_filter8 = ff_vc1_h_loop_filter8_mmi;
dsp->vc1_v_loop_filter16 = ff_vc1_v_loop_filter16_mmi;
dsp->vc1_h_loop_filter16 = ff_vc1_h_loop_filter16_mmi;

FN_ASSIGN(put_, 0, 0, _mmi);
FN_ASSIGN(put_, 0, 1, _mmi);
FN_ASSIGN(put_, 0, 2, _mmi);
FN_ASSIGN(put_, 0, 3, _mmi);

FN_ASSIGN(put_, 1, 0, _mmi);




FN_ASSIGN(put_, 2, 0, _mmi);




FN_ASSIGN(put_, 3, 0, _mmi);




FN_ASSIGN(avg_, 0, 0, _mmi);
FN_ASSIGN(avg_, 0, 1, _mmi);
FN_ASSIGN(avg_, 0, 2, _mmi);
FN_ASSIGN(avg_, 0, 3, _mmi);

FN_ASSIGN(avg_, 1, 0, _mmi);




FN_ASSIGN(avg_, 2, 0, _mmi);




FN_ASSIGN(avg_, 3, 0, _mmi);




dsp->put_no_rnd_vc1_chroma_pixels_tab[0] = ff_put_no_rnd_vc1_chroma_mc8_mmi;
dsp->avg_no_rnd_vc1_chroma_pixels_tab[0] = ff_avg_no_rnd_vc1_chroma_mc8_mmi;
dsp->put_no_rnd_vc1_chroma_pixels_tab[1] = ff_put_no_rnd_vc1_chroma_mc4_mmi;
dsp->avg_no_rnd_vc1_chroma_pixels_tab[1] = ff_avg_no_rnd_vc1_chroma_mc4_mmi;
}
#endif 

#if HAVE_MSA
static av_cold void vc1dsp_init_msa(VC1DSPContext *dsp)
{
dsp->vc1_inv_trans_8x8 = ff_vc1_inv_trans_8x8_msa;
dsp->vc1_inv_trans_4x8 = ff_vc1_inv_trans_4x8_msa;
dsp->vc1_inv_trans_8x4 = ff_vc1_inv_trans_8x4_msa;

FN_ASSIGN(put_, 1, 1, _msa);
FN_ASSIGN(put_, 1, 2, _msa);
FN_ASSIGN(put_, 1, 3, _msa);
FN_ASSIGN(put_, 2, 1, _msa);
FN_ASSIGN(put_, 2, 2, _msa);
FN_ASSIGN(put_, 2, 3, _msa);
FN_ASSIGN(put_, 3, 1, _msa);
FN_ASSIGN(put_, 3, 2, _msa);
FN_ASSIGN(put_, 3, 3, _msa);
}
#endif 

av_cold void ff_vc1dsp_init_mips(VC1DSPContext *dsp)
{
#if HAVE_MMI
vc1dsp_init_mmi(dsp);
#endif 
#if HAVE_MSA
vc1dsp_init_msa(dsp);
#endif 
}
