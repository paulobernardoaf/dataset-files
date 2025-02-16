





















#include "hevcdec.h"

#include "hevcpred.h"

#define BIT_DEPTH 8
#include "hevcpred_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 9
#include "hevcpred_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 10
#include "hevcpred_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 12
#include "hevcpred_template.c"
#undef BIT_DEPTH

void ff_hevc_pred_init(HEVCPredContext *hpc, int bit_depth)
{
#undef FUNC
#define FUNC(a, depth) a ##_ ##depth

#define HEVC_PRED(depth) hpc->intra_pred[0] = FUNC(intra_pred_2, depth); hpc->intra_pred[1] = FUNC(intra_pred_3, depth); hpc->intra_pred[2] = FUNC(intra_pred_4, depth); hpc->intra_pred[3] = FUNC(intra_pred_5, depth); hpc->pred_planar[0] = FUNC(pred_planar_0, depth); hpc->pred_planar[1] = FUNC(pred_planar_1, depth); hpc->pred_planar[2] = FUNC(pred_planar_2, depth); hpc->pred_planar[3] = FUNC(pred_planar_3, depth); hpc->pred_dc = FUNC(pred_dc, depth); hpc->pred_angular[0] = FUNC(pred_angular_0, depth); hpc->pred_angular[1] = FUNC(pred_angular_1, depth); hpc->pred_angular[2] = FUNC(pred_angular_2, depth); hpc->pred_angular[3] = FUNC(pred_angular_3, depth);














switch (bit_depth) {
case 9:
HEVC_PRED(9);
break;
case 10:
HEVC_PRED(10);
break;
case 12:
HEVC_PRED(12);
break;
default:
HEVC_PRED(8);
break;
}

if (ARCH_MIPS)
ff_hevc_pred_init_mips(hpc, bit_depth);
}
