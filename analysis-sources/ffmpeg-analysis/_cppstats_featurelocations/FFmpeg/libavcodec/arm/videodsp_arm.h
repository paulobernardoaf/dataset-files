



















#if !defined(AVCODEC_ARM_VIDEODSP_ARM_H)
#define AVCODEC_ARM_VIDEODSP_ARM_H

#include "libavcodec/avcodec.h"
#include "libavcodec/videodsp.h"

void ff_videodsp_init_armv5te(VideoDSPContext* ctx, int bpc);

#endif 
