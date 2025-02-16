

















#if !defined(AVCODEC_ALACDSP_H)
#define AVCODEC_ALACDSP_H

#include <stdint.h>

typedef struct ALACDSPContext {
void (*decorrelate_stereo)(int32_t *buffer[2], int nb_samples,
int decorr_shift, int decorr_left_weight);
void (*append_extra_bits[2])(int32_t *buffer[2], int32_t *extra_bits_buffer[2],
int extra_bits, int channels, int nb_samples);
} ALACDSPContext;

void ff_alacdsp_init(ALACDSPContext *c);
void ff_alacdsp_init_x86(ALACDSPContext *c);

#endif 
