




























#if !defined(AVCODEC_AACDECTAB_H)
#define AVCODEC_AACDECTAB_H

#include "libavutil/channel_layout.h"
#include "aac.h"

#include <stdint.h>

static const int8_t tags_per_config[16] = { 0, 1, 1, 2, 3, 3, 4, 5, 0, 0, 0, 4, 5, 0, 5, 0 };

static const uint8_t aac_channel_layout_map[16][5][3] = {
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, },
{ { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_SCE, 1, AAC_CHANNEL_BACK }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 1, AAC_CHANNEL_BACK }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 1, AAC_CHANNEL_BACK }, { TYPE_LFE, 0, AAC_CHANNEL_LFE }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 1, AAC_CHANNEL_FRONT }, { TYPE_CPE, 2, AAC_CHANNEL_BACK }, { TYPE_LFE, 0, AAC_CHANNEL_LFE }, },
{ { 0, } },
{ { 0, } },
{ { 0, } },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 1, AAC_CHANNEL_BACK }, { TYPE_SCE, 1, AAC_CHANNEL_BACK }, { TYPE_LFE, 0, AAC_CHANNEL_LFE }, },
{ { TYPE_SCE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 0, AAC_CHANNEL_FRONT }, { TYPE_CPE, 1, AAC_CHANNEL_SIDE }, { TYPE_CPE, 2, AAC_CHANNEL_BACK }, { TYPE_LFE, 0, AAC_CHANNEL_LFE }, },
{ { 0, } },

};

static const uint64_t aac_channel_layout[16] = {
AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_4POINT0,
AV_CH_LAYOUT_5POINT0_BACK,
AV_CH_LAYOUT_5POINT1_BACK,
AV_CH_LAYOUT_7POINT1_WIDE_BACK,
0,
0,
0,
AV_CH_LAYOUT_6POINT1,
AV_CH_LAYOUT_7POINT1,
0,

};

#endif 
