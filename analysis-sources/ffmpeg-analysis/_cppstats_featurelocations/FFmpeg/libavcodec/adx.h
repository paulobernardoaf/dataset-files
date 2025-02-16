





























#if !defined(AVCODEC_ADX_H)
#define AVCODEC_ADX_H

#include <stdint.h>

#include "avcodec.h"

typedef struct ADXChannelState {
int s1,s2;
} ADXChannelState;

typedef struct ADXContext {
int channels;
ADXChannelState prev[2];
int header_parsed;
int eof;
int cutoff;
int coeff[2];
} ADXContext;

#define COEFF_BITS 12

#define BLOCK_SIZE 18
#define BLOCK_SAMPLES 32









void ff_adx_calculate_coeffs(int cutoff, int sample_rate, int bits, int *coeff);












int ff_adx_decode_header(AVCodecContext *avctx, const uint8_t *buf,
int bufsize, int *header_size, int *coeff);

#endif 
