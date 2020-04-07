























#if !defined(AVCODEC_G722_H)
#define AVCODEC_G722_H

#include <stdint.h>
#include "avcodec.h"
#include "g722dsp.h"

#define PREV_SAMPLES_BUF_SIZE 1024

typedef struct G722Context {
const AVClass *class;
int bits_per_codeword;
int16_t prev_samples[PREV_SAMPLES_BUF_SIZE]; 
int prev_samples_pos; 




struct G722Band {
int16_t s_predictor; 
int32_t s_zero; 
int8_t part_reconst_mem[2]; 
int16_t prev_qtzd_reconst; 
int16_t pole_mem[2]; 
int32_t diff_mem[6]; 
int16_t zero_mem[6]; 
int16_t log_factor; 
int16_t scale_factor; 
} band[2];

struct TrellisNode {
struct G722Band state;
uint32_t ssd;
int path;
} *node_buf[2], **nodep_buf[2];

struct TrellisPath {
int value;
int prev;
} *paths[2];

G722DSPContext dsp;
} G722Context;

extern const int16_t ff_g722_high_inv_quant[4];
extern const int16_t ff_g722_low_inv_quant4[16];
extern const int16_t ff_g722_low_inv_quant6[64];

void ff_g722_update_low_predictor(struct G722Band *band, const int ilow);

void ff_g722_update_high_predictor(struct G722Band *band, const int dhigh,
const int ihigh);

#endif 
