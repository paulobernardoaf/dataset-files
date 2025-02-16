




















#if !defined(AVCODEC_MLPDSP_H)
#define AVCODEC_MLPDSP_H

#include <stdint.h>
#include "mlp.h"

void ff_mlp_rematrix_channel(int32_t *samples,
const int32_t *coeffs,
const uint8_t *bypassed_lsbs,
const int8_t *noise_buffer,
int index,
unsigned int dest_ch,
uint16_t blockpos,
unsigned int maxchan,
int matrix_noise_shift,
int access_unit_size_pow2,
int32_t mask);

int32_t ff_mlp_pack_output(int32_t lossless_check_data,
uint16_t blockpos,
int32_t (*sample_buffer)[MAX_CHANNELS],
void *data,
uint8_t *ch_assign,
int8_t *output_shift,
uint8_t max_matrix_channel,
int is32);

typedef struct MLPDSPContext {
void (*mlp_filter_channel)(int32_t *state, const int32_t *coeff,
int firorder, int iirorder,
unsigned int filter_shift, int32_t mask,
int blocksize, int32_t *sample_buffer);
void (*mlp_rematrix_channel)(int32_t *samples,
const int32_t *coeffs,
const uint8_t *bypassed_lsbs,
const int8_t *noise_buffer,
int index,
unsigned int dest_ch,
uint16_t blockpos,
unsigned int maxchan,
int matrix_noise_shift,
int access_unit_size_pow2,
int32_t mask);
int32_t (*(*mlp_select_pack_output)(uint8_t *ch_assign,
int8_t *output_shift,
uint8_t max_matrix_channel,
int is32))(int32_t, uint16_t, int32_t (*)[], void *, uint8_t*, int8_t *, uint8_t, int);
int32_t (*mlp_pack_output)(int32_t lossless_check_data,
uint16_t blockpos,
int32_t (*sample_buffer)[MAX_CHANNELS],
void *data,
uint8_t *ch_assign,
int8_t *output_shift,
uint8_t max_matrix_channel,
int is32);
} MLPDSPContext;

void ff_mlpdsp_init(MLPDSPContext *c);
void ff_mlpdsp_init_arm(MLPDSPContext *c);
void ff_mlpdsp_init_x86(MLPDSPContext *c);

#endif 
