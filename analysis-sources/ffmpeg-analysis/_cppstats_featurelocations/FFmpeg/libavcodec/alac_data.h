



















#if !defined(AVCODEC_ALAC_DATA_H)
#define AVCODEC_ALAC_DATA_H

#include <stdint.h>

enum AlacRawDataBlockType {

TYPE_SCE,
TYPE_CPE,
TYPE_CCE,
TYPE_LFE,
TYPE_DSE,
TYPE_PCE,
TYPE_FIL,
TYPE_END
};

#define ALAC_MAX_CHANNELS 8

extern const uint8_t ff_alac_channel_layout_offsets[ALAC_MAX_CHANNELS][ALAC_MAX_CHANNELS];

extern const uint64_t ff_alac_channel_layouts[ALAC_MAX_CHANNELS + 1];

extern const enum AlacRawDataBlockType ff_alac_channel_elements[ALAC_MAX_CHANNELS][5];

#endif 
