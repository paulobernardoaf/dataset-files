#include <stdint.h>
static const uint8_t ff_adpcm_ima_block_sizes[4] = { 4, 12, 4, 20 };
static const uint8_t ff_adpcm_ima_block_samples[4] = { 16, 32, 8, 32 };
extern const int8_t * const ff_adpcm_index_tables[4];
extern const int8_t ff_adpcm_index_table[16];
extern const int16_t ff_adpcm_step_table[89];
extern const int16_t ff_adpcm_oki_step_table[49];
extern const int16_t ff_adpcm_AdaptationTable[];
extern const uint8_t ff_adpcm_AdaptCoeff1[];
extern const int8_t ff_adpcm_AdaptCoeff2[];
extern const int16_t ff_adpcm_yamaha_indexscale[];
extern const int8_t ff_adpcm_yamaha_difflookup[];
extern const int16_t ff_adpcm_afc_coeffs[2][16];
extern const int16_t ff_adpcm_mtaf_stepsize[32][16];
