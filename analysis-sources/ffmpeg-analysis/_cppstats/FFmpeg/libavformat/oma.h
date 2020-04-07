#include <stdint.h>
#include "internal.h"
#define EA3_HEADER_SIZE 96
#define ID3v2_EA3_MAGIC "ea3"
#define OMA_ENC_HEADER_SIZE 16
enum {
OMA_CODECID_ATRAC3 = 0,
OMA_CODECID_ATRAC3P = 1,
OMA_CODECID_MP3 = 3,
OMA_CODECID_LPCM = 4,
OMA_CODECID_WMA = 5,
OMA_CODECID_ATRAC3PAL = 33,
OMA_CODECID_ATRAC3AL = 34,
};
extern const uint16_t ff_oma_srate_tab[8];
extern const AVCodecTag ff_oma_codec_tags[];
extern const uint64_t ff_oma_chid_to_native_layout[7];
extern const int ff_oma_chid_to_num_channels[7];
