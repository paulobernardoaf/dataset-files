

















#if !defined(AVCODEC_ARM_HEVCDSP_ARM_H)
#define AVCODEC_ARM_HEVCDSP_ARM_H

#include "libavcodec/hevcdsp.h"

void ff_hevc_dsp_init_neon(HEVCDSPContext *c, const int bit_depth);

#endif 
