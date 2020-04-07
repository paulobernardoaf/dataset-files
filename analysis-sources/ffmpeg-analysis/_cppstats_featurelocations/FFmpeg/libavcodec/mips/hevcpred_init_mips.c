



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavcodec/mips/hevcpred_mips.h"

#if HAVE_MSA
static av_cold void hevc_pred_init_msa(HEVCPredContext *c, const int bit_depth)
{
if (8 == bit_depth) {
c->intra_pred[2] = ff_intra_pred_8_16x16_msa;
c->intra_pred[3] = ff_intra_pred_8_32x32_msa;
c->pred_planar[0] = ff_hevc_intra_pred_planar_0_msa;
c->pred_planar[1] = ff_hevc_intra_pred_planar_1_msa;
c->pred_planar[2] = ff_hevc_intra_pred_planar_2_msa;
c->pred_planar[3] = ff_hevc_intra_pred_planar_3_msa;
c->pred_dc = ff_hevc_intra_pred_dc_msa;
c->pred_angular[0] = ff_pred_intra_pred_angular_0_msa;
c->pred_angular[1] = ff_pred_intra_pred_angular_1_msa;
c->pred_angular[2] = ff_pred_intra_pred_angular_2_msa;
c->pred_angular[3] = ff_pred_intra_pred_angular_3_msa;
}
}
#endif 

void ff_hevc_pred_init_mips(HEVCPredContext *c, const int bit_depth)
{
#if HAVE_MSA
hevc_pred_init_msa(c, bit_depth);
#endif 
}
