



















#if !defined(AVUTIL_ARM_FLOAT_DSP_ARM_H)
#define AVUTIL_ARM_FLOAT_DSP_ARM_H

#include "libavutil/float_dsp.h"

void ff_float_dsp_init_vfp(AVFloatDSPContext *fdsp, int cpu_flags);
void ff_float_dsp_init_neon(AVFloatDSPContext *fdsp);

#endif 
