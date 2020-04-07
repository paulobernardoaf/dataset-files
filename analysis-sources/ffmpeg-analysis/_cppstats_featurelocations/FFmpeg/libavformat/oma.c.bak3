



















#include "internal.h"
#include "oma.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"

const uint16_t ff_oma_srate_tab[8] = { 320, 441, 480, 882, 960, 0 };

const AVCodecTag ff_oma_codec_tags[] = {
{ AV_CODEC_ID_ATRAC3, OMA_CODECID_ATRAC3 },
{ AV_CODEC_ID_ATRAC3P, OMA_CODECID_ATRAC3P },
{ AV_CODEC_ID_MP3, OMA_CODECID_MP3 },
{ AV_CODEC_ID_PCM_S16BE, OMA_CODECID_LPCM },
{ AV_CODEC_ID_ATRAC3PAL, OMA_CODECID_ATRAC3PAL },
{ AV_CODEC_ID_ATRAC3AL, OMA_CODECID_ATRAC3AL },
{ 0 },
};


const uint64_t ff_oma_chid_to_native_layout[7] = {
AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_4POINT0,
AV_CH_LAYOUT_5POINT1_BACK,
AV_CH_LAYOUT_6POINT1_BACK,
AV_CH_LAYOUT_7POINT1
};


const int ff_oma_chid_to_num_channels[7] = {1, 2, 3, 4, 6, 7, 8};
