



















#if !defined(AVCODEC_ARM_VP9DSP_INIT_H)
#define AVCODEC_ARM_VP9DSP_INIT_H

#include "libavcodec/vp9dsp.h"

void ff_vp9dsp_init_10bpp_arm(VP9DSPContext *dsp);
void ff_vp9dsp_init_12bpp_arm(VP9DSPContext *dsp);

#endif 
