

















#if !defined(AVCODEC_OPUSDSP_H)
#define AVCODEC_OPUSDSP_H

#include "libavutil/common.h"

#define CELT_EMPH_COEFF 0.8500061035f

typedef struct OpusDSP {
void (*postfilter)(float *data, int period, float *gains, int len);
float (*deemphasis)(float *out, float *in, float coeff, int len);
} OpusDSP;

void ff_opus_dsp_init(OpusDSP *ctx);

void ff_opus_dsp_init_x86(OpusDSP *ctx);
void ff_opus_dsp_init_aarch64(OpusDSP *ctx);

#endif 
