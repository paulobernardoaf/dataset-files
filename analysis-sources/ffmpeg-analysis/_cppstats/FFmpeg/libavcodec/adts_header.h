#include "get_bits.h"
typedef struct AACADTSHeaderInfo {
uint32_t sample_rate;
uint32_t samples;
uint32_t bit_rate;
uint8_t crc_absent;
uint8_t object_type;
uint8_t sampling_index;
uint8_t chan_config;
uint8_t num_aac_frames;
} AACADTSHeaderInfo;
int ff_adts_header_parse(GetBitContext *gbc, AACADTSHeaderInfo *hdr);
