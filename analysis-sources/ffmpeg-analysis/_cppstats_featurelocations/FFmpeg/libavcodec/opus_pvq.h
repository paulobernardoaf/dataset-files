





















#if !defined(AVCODEC_OPUS_PVQ_H)
#define AVCODEC_OPUS_PVQ_H

#include "opus_celt.h"

#define QUANT_FN(name) uint32_t (name)(struct CeltPVQ *pvq, CeltFrame *f, OpusRangeCoder *rc, const int band, float *X, float *Y, int N, int b, uint32_t blocks, float *lowband, int duration, float *lowband_out, int level, float gain, float *lowband_scratch, int fill)






struct CeltPVQ {
DECLARE_ALIGNED(32, int, qcoeff )[256];
DECLARE_ALIGNED(32, float, hadamard_tmp)[256];

float (*pvq_search)(float *X, int *y, int K, int N);
QUANT_FN(*quant_band);
};

void ff_celt_pvq_init_x86(struct CeltPVQ *s);

int ff_celt_pvq_init(struct CeltPVQ **pvq, int encode);
void ff_celt_pvq_uninit(struct CeltPVQ **pvq);

#endif 
