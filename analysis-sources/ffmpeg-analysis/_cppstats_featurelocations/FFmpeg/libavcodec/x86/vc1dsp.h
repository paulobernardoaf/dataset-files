



















#if !defined(AVCODEC_X86_VC1DSP_H)
#define AVCODEC_X86_VC1DSP_H

#include "libavcodec/vc1dsp.h"

void ff_vc1dsp_init_mmx(VC1DSPContext *dsp);
void ff_vc1dsp_init_mmxext(VC1DSPContext *dsp);

#endif 
