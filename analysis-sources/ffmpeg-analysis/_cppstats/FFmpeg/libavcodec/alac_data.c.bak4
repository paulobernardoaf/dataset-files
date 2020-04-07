



















#include "libavutil/channel_layout.h"
#include "alac_data.h"

const uint8_t ff_alac_channel_layout_offsets[ALAC_MAX_CHANNELS][ALAC_MAX_CHANNELS] = {
{ 0 },
{ 0, 1 },
{ 2, 0, 1 },
{ 2, 0, 1, 3 },
{ 2, 0, 1, 3, 4 },
{ 2, 0, 1, 4, 5, 3 },
{ 2, 0, 1, 4, 5, 6, 3 },
{ 2, 6, 7, 0, 1, 4, 5, 3 }
};

const uint64_t ff_alac_channel_layouts[ALAC_MAX_CHANNELS + 1] = {
AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_4POINT0,
AV_CH_LAYOUT_5POINT0_BACK,
AV_CH_LAYOUT_5POINT1_BACK,
AV_CH_LAYOUT_6POINT1_BACK,
AV_CH_LAYOUT_7POINT1_WIDE_BACK,
0
};

const enum AlacRawDataBlockType ff_alac_channel_elements[ALAC_MAX_CHANNELS][5] = {
{ TYPE_SCE, },
{ TYPE_CPE, },
{ TYPE_SCE, TYPE_CPE, },
{ TYPE_SCE, TYPE_CPE, TYPE_SCE },
{ TYPE_SCE, TYPE_CPE, TYPE_CPE, },
{ TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_SCE, },
{ TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_SCE, TYPE_SCE, },
{ TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_CPE, TYPE_SCE, },
};
