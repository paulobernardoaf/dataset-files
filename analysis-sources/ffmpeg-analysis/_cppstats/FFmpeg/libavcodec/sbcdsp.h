#include "sbc.h"
#include "sbcdsp_data.h"
#define SCALE_OUT_BITS 15
#define SBC_X_BUFFER_SIZE 328
typedef struct sbc_dsp_context SBCDSPContext;
struct sbc_dsp_context {
int position;
uint8_t increment;
DECLARE_ALIGNED(SBC_ALIGN, int16_t, X)[2][SBC_X_BUFFER_SIZE];
void (*sbc_analyze_4)(const int16_t *in, int32_t *out, const int16_t *consts);
void (*sbc_analyze_8)(const int16_t *in, int32_t *out, const int16_t *consts);
void (*sbc_analyze_4s)(SBCDSPContext *s,
int16_t *x, int32_t *out, int out_stride);
void (*sbc_analyze_8s)(SBCDSPContext *s,
int16_t *x, int32_t *out, int out_stride);
int (*sbc_enc_process_input_4s)(int position, const uint8_t *pcm,
int16_t X[2][SBC_X_BUFFER_SIZE],
int nsamples, int nchannels);
int (*sbc_enc_process_input_8s)(int position, const uint8_t *pcm,
int16_t X[2][SBC_X_BUFFER_SIZE],
int nsamples, int nchannels);
void (*sbc_calc_scalefactors)(int32_t sb_sample_f[16][2][8],
uint32_t scale_factor[2][8],
int blocks, int channels, int subbands);
int (*sbc_calc_scalefactors_j)(int32_t sb_sample_f[16][2][8],
uint32_t scale_factor[2][8],
int blocks, int subbands);
};
void ff_sbcdsp_init(SBCDSPContext *s);
void ff_sbcdsp_init_arm(SBCDSPContext *s);
void ff_sbcdsp_init_x86(SBCDSPContext *s);
