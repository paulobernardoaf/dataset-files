
























#include <stdint.h>

#include "libavutil/channel_layout.h"
#include "libavcodec/avcodec.h"
#include "mov_chan.h"
















enum MovChannelLayoutTag {
#define MOV_CH_LAYOUT_UNKNOWN 0xFFFF0000
MOV_CH_LAYOUT_USE_DESCRIPTIONS = ( 0 << 16) | 0,
MOV_CH_LAYOUT_USE_BITMAP = ( 1 << 16) | 0,
MOV_CH_LAYOUT_DISCRETEINORDER = (147 << 16) | 0,
MOV_CH_LAYOUT_MONO = (100 << 16) | 1,
MOV_CH_LAYOUT_STEREO = (101 << 16) | 2,
MOV_CH_LAYOUT_STEREOHEADPHONES = (102 << 16) | 2,
MOV_CH_LAYOUT_MATRIXSTEREO = (103 << 16) | 2,
MOV_CH_LAYOUT_MIDSIDE = (104 << 16) | 2,
MOV_CH_LAYOUT_XY = (105 << 16) | 2,
MOV_CH_LAYOUT_BINAURAL = (106 << 16) | 2,
MOV_CH_LAYOUT_AMBISONIC_B_FORMAT = (107 << 16) | 4,
MOV_CH_LAYOUT_QUADRAPHONIC = (108 << 16) | 4,
MOV_CH_LAYOUT_PENTAGONAL = (109 << 16) | 5,
MOV_CH_LAYOUT_HEXAGONAL = (110 << 16) | 6,
MOV_CH_LAYOUT_OCTAGONAL = (111 << 16) | 8,
MOV_CH_LAYOUT_CUBE = (112 << 16) | 8,
MOV_CH_LAYOUT_MPEG_3_0_A = (113 << 16) | 3,
MOV_CH_LAYOUT_MPEG_3_0_B = (114 << 16) | 3,
MOV_CH_LAYOUT_MPEG_4_0_A = (115 << 16) | 4,
MOV_CH_LAYOUT_MPEG_4_0_B = (116 << 16) | 4,
MOV_CH_LAYOUT_MPEG_5_0_A = (117 << 16) | 5,
MOV_CH_LAYOUT_MPEG_5_0_B = (118 << 16) | 5,
MOV_CH_LAYOUT_MPEG_5_0_C = (119 << 16) | 5,
MOV_CH_LAYOUT_MPEG_5_0_D = (120 << 16) | 5,
MOV_CH_LAYOUT_MPEG_5_1_A = (121 << 16) | 6,
MOV_CH_LAYOUT_MPEG_5_1_B = (122 << 16) | 6,
MOV_CH_LAYOUT_MPEG_5_1_C = (123 << 16) | 6,
MOV_CH_LAYOUT_MPEG_5_1_D = (124 << 16) | 6,
MOV_CH_LAYOUT_MPEG_6_1_A = (125 << 16) | 7,
MOV_CH_LAYOUT_MPEG_7_1_A = (126 << 16) | 8,
MOV_CH_LAYOUT_MPEG_7_1_B = (127 << 16) | 8,
MOV_CH_LAYOUT_MPEG_7_1_C = (128 << 16) | 8,
MOV_CH_LAYOUT_EMAGIC_DEFAULT_7_1 = (129 << 16) | 8,
MOV_CH_LAYOUT_SMPTE_DTV = (130 << 16) | 8,
MOV_CH_LAYOUT_ITU_2_1 = (131 << 16) | 3,
MOV_CH_LAYOUT_ITU_2_2 = (132 << 16) | 4,
MOV_CH_LAYOUT_DVD_4 = (133 << 16) | 3,
MOV_CH_LAYOUT_DVD_5 = (134 << 16) | 4,
MOV_CH_LAYOUT_DVD_6 = (135 << 16) | 5,
MOV_CH_LAYOUT_DVD_10 = (136 << 16) | 4,
MOV_CH_LAYOUT_DVD_11 = (137 << 16) | 5,
MOV_CH_LAYOUT_DVD_18 = (138 << 16) | 5,
MOV_CH_LAYOUT_AUDIOUNIT_6_0 = (139 << 16) | 6,
MOV_CH_LAYOUT_AUDIOUNIT_7_0 = (140 << 16) | 7,
MOV_CH_LAYOUT_AUDIOUNIT_7_0_FRONT = (148 << 16) | 7,
MOV_CH_LAYOUT_AAC_6_0 = (141 << 16) | 6,
MOV_CH_LAYOUT_AAC_6_1 = (142 << 16) | 7,
MOV_CH_LAYOUT_AAC_7_0 = (143 << 16) | 7,
MOV_CH_LAYOUT_AAC_OCTAGONAL = (144 << 16) | 8,
MOV_CH_LAYOUT_TMH_10_2_STD = (145 << 16) | 16,
MOV_CH_LAYOUT_TMH_10_2_FULL = (146 << 16) | 21,
MOV_CH_LAYOUT_AC3_1_0_1 = (149 << 16) | 2,
MOV_CH_LAYOUT_AC3_3_0 = (150 << 16) | 3,
MOV_CH_LAYOUT_AC3_3_1 = (151 << 16) | 4,
MOV_CH_LAYOUT_AC3_3_0_1 = (152 << 16) | 4,
MOV_CH_LAYOUT_AC3_2_1_1 = (153 << 16) | 4,
MOV_CH_LAYOUT_AC3_3_1_1 = (154 << 16) | 5,
MOV_CH_LAYOUT_EAC3_6_0_A = (155 << 16) | 6,
MOV_CH_LAYOUT_EAC3_7_0_A = (156 << 16) | 7,
MOV_CH_LAYOUT_EAC3_6_1_A = (157 << 16) | 7,
MOV_CH_LAYOUT_EAC3_6_1_B = (158 << 16) | 7,
MOV_CH_LAYOUT_EAC3_6_1_C = (159 << 16) | 7,
MOV_CH_LAYOUT_EAC3_7_1_A = (160 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_B = (161 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_C = (162 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_D = (163 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_E = (164 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_F = (165 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_G = (166 << 16) | 8,
MOV_CH_LAYOUT_EAC3_7_1_H = (167 << 16) | 8,
MOV_CH_LAYOUT_DTS_3_1 = (168 << 16) | 4,
MOV_CH_LAYOUT_DTS_4_1 = (169 << 16) | 5,
MOV_CH_LAYOUT_DTS_6_0_A = (170 << 16) | 6,
MOV_CH_LAYOUT_DTS_6_0_B = (171 << 16) | 6,
MOV_CH_LAYOUT_DTS_6_0_C = (172 << 16) | 6,
MOV_CH_LAYOUT_DTS_6_1_A = (173 << 16) | 7,
MOV_CH_LAYOUT_DTS_6_1_B = (174 << 16) | 7,
MOV_CH_LAYOUT_DTS_6_1_C = (175 << 16) | 7,
MOV_CH_LAYOUT_DTS_6_1_D = (182 << 16) | 7,
MOV_CH_LAYOUT_DTS_7_0 = (176 << 16) | 7,
MOV_CH_LAYOUT_DTS_7_1 = (177 << 16) | 8,
MOV_CH_LAYOUT_DTS_8_0_A = (178 << 16) | 8,
MOV_CH_LAYOUT_DTS_8_0_B = (179 << 16) | 8,
MOV_CH_LAYOUT_DTS_8_1_A = (180 << 16) | 9,
MOV_CH_LAYOUT_DTS_8_1_B = (181 << 16) | 9,
};

struct MovChannelLayoutMap {
uint32_t tag;
uint64_t layout;
};

static const struct MovChannelLayoutMap mov_ch_layout_map_misc[] = {
{ MOV_CH_LAYOUT_USE_DESCRIPTIONS, 0 },
{ MOV_CH_LAYOUT_USE_BITMAP, 0 },
{ MOV_CH_LAYOUT_DISCRETEINORDER, 0 },
{ MOV_CH_LAYOUT_UNKNOWN, 0 },
{ MOV_CH_LAYOUT_TMH_10_2_STD, 0 }, 


{ MOV_CH_LAYOUT_TMH_10_2_FULL, 0 }, 



{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_1ch[] = {
{ MOV_CH_LAYOUT_MONO, AV_CH_LAYOUT_MONO }, 
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_2ch[] = {
{ MOV_CH_LAYOUT_STEREO, AV_CH_LAYOUT_STEREO }, 
{ MOV_CH_LAYOUT_STEREOHEADPHONES, AV_CH_LAYOUT_STEREO }, 
{ MOV_CH_LAYOUT_BINAURAL, AV_CH_LAYOUT_STEREO }, 
{ MOV_CH_LAYOUT_MIDSIDE, AV_CH_LAYOUT_STEREO }, 
{ MOV_CH_LAYOUT_XY, AV_CH_LAYOUT_STEREO }, 

{ MOV_CH_LAYOUT_MATRIXSTEREO, AV_CH_LAYOUT_STEREO_DOWNMIX }, 

{ MOV_CH_LAYOUT_AC3_1_0_1, AV_CH_LAYOUT_MONO | 
AV_CH_LOW_FREQUENCY },
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_3ch[] = {
{ MOV_CH_LAYOUT_MPEG_3_0_A, AV_CH_LAYOUT_SURROUND }, 
{ MOV_CH_LAYOUT_MPEG_3_0_B, AV_CH_LAYOUT_SURROUND }, 
{ MOV_CH_LAYOUT_AC3_3_0, AV_CH_LAYOUT_SURROUND }, 

{ MOV_CH_LAYOUT_ITU_2_1, AV_CH_LAYOUT_2_1 }, 

{ MOV_CH_LAYOUT_DVD_4, AV_CH_LAYOUT_2POINT1 }, 
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_4ch[] = {
{ MOV_CH_LAYOUT_AMBISONIC_B_FORMAT, 0 }, 

{ MOV_CH_LAYOUT_QUADRAPHONIC, AV_CH_LAYOUT_QUAD }, 

{ MOV_CH_LAYOUT_MPEG_4_0_A, AV_CH_LAYOUT_4POINT0 }, 
{ MOV_CH_LAYOUT_MPEG_4_0_B, AV_CH_LAYOUT_4POINT0 }, 
{ MOV_CH_LAYOUT_AC3_3_1, AV_CH_LAYOUT_4POINT0 }, 

{ MOV_CH_LAYOUT_ITU_2_2, AV_CH_LAYOUT_2_2 }, 

{ MOV_CH_LAYOUT_DVD_5, AV_CH_LAYOUT_2_1 | 
AV_CH_LOW_FREQUENCY },
{ MOV_CH_LAYOUT_AC3_2_1_1, AV_CH_LAYOUT_2_1 | 
AV_CH_LOW_FREQUENCY },

{ MOV_CH_LAYOUT_DVD_10, AV_CH_LAYOUT_3POINT1 }, 
{ MOV_CH_LAYOUT_AC3_3_0_1, AV_CH_LAYOUT_3POINT1 }, 
{ MOV_CH_LAYOUT_DTS_3_1, AV_CH_LAYOUT_3POINT1 }, 
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_5ch[] = {
{ MOV_CH_LAYOUT_PENTAGONAL, AV_CH_LAYOUT_5POINT0_BACK }, 

{ MOV_CH_LAYOUT_MPEG_5_0_A, AV_CH_LAYOUT_5POINT0 }, 
{ MOV_CH_LAYOUT_MPEG_5_0_B, AV_CH_LAYOUT_5POINT0 }, 
{ MOV_CH_LAYOUT_MPEG_5_0_C, AV_CH_LAYOUT_5POINT0 }, 
{ MOV_CH_LAYOUT_MPEG_5_0_D, AV_CH_LAYOUT_5POINT0 }, 

{ MOV_CH_LAYOUT_DVD_6, AV_CH_LAYOUT_2_2 | 
AV_CH_LOW_FREQUENCY },
{ MOV_CH_LAYOUT_DVD_18, AV_CH_LAYOUT_2_2 | 
AV_CH_LOW_FREQUENCY },

{ MOV_CH_LAYOUT_DVD_11, AV_CH_LAYOUT_4POINT1 }, 
{ MOV_CH_LAYOUT_AC3_3_1_1, AV_CH_LAYOUT_4POINT1 }, 
{ MOV_CH_LAYOUT_DTS_4_1, AV_CH_LAYOUT_4POINT1 }, 
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_6ch[] = {
{ MOV_CH_LAYOUT_HEXAGONAL, AV_CH_LAYOUT_HEXAGONAL }, 
{ MOV_CH_LAYOUT_DTS_6_0_C, AV_CH_LAYOUT_HEXAGONAL }, 

{ MOV_CH_LAYOUT_MPEG_5_1_A, AV_CH_LAYOUT_5POINT1 }, 
{ MOV_CH_LAYOUT_MPEG_5_1_B, AV_CH_LAYOUT_5POINT1 }, 
{ MOV_CH_LAYOUT_MPEG_5_1_C, AV_CH_LAYOUT_5POINT1 }, 
{ MOV_CH_LAYOUT_MPEG_5_1_D, AV_CH_LAYOUT_5POINT1 }, 

{ MOV_CH_LAYOUT_AUDIOUNIT_6_0, AV_CH_LAYOUT_6POINT0 }, 
{ MOV_CH_LAYOUT_AAC_6_0, AV_CH_LAYOUT_6POINT0 }, 
{ MOV_CH_LAYOUT_EAC3_6_0_A, AV_CH_LAYOUT_6POINT0 }, 

{ MOV_CH_LAYOUT_DTS_6_0_A, AV_CH_LAYOUT_6POINT0_FRONT }, 

{ MOV_CH_LAYOUT_DTS_6_0_B, AV_CH_LAYOUT_5POINT0_BACK | 
AV_CH_TOP_CENTER },
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_7ch[] = {
{ MOV_CH_LAYOUT_MPEG_6_1_A, AV_CH_LAYOUT_6POINT1 }, 
{ MOV_CH_LAYOUT_AAC_6_1, AV_CH_LAYOUT_6POINT1 }, 
{ MOV_CH_LAYOUT_EAC3_6_1_A, AV_CH_LAYOUT_6POINT1 }, 
{ MOV_CH_LAYOUT_DTS_6_1_D, AV_CH_LAYOUT_6POINT1 }, 

{ MOV_CH_LAYOUT_AUDIOUNIT_7_0, AV_CH_LAYOUT_7POINT0 }, 
{ MOV_CH_LAYOUT_AAC_7_0, AV_CH_LAYOUT_7POINT0 }, 
{ MOV_CH_LAYOUT_EAC3_7_0_A, AV_CH_LAYOUT_7POINT0 }, 

{ MOV_CH_LAYOUT_AUDIOUNIT_7_0_FRONT, AV_CH_LAYOUT_7POINT0_FRONT }, 
{ MOV_CH_LAYOUT_DTS_7_0, AV_CH_LAYOUT_7POINT0_FRONT }, 

{ MOV_CH_LAYOUT_EAC3_6_1_B, AV_CH_LAYOUT_5POINT1 | 
AV_CH_TOP_CENTER },

{ MOV_CH_LAYOUT_EAC3_6_1_C, AV_CH_LAYOUT_5POINT1 | 
AV_CH_TOP_FRONT_CENTER },

{ MOV_CH_LAYOUT_DTS_6_1_A, AV_CH_LAYOUT_6POINT1_FRONT }, 

{ MOV_CH_LAYOUT_DTS_6_1_B, AV_CH_LAYOUT_5POINT1_BACK | 
AV_CH_TOP_CENTER },

{ MOV_CH_LAYOUT_DTS_6_1_C, AV_CH_LAYOUT_6POINT1_BACK }, 
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_8ch[] = {
{ MOV_CH_LAYOUT_OCTAGONAL, AV_CH_LAYOUT_OCTAGONAL }, 
{ MOV_CH_LAYOUT_AAC_OCTAGONAL, AV_CH_LAYOUT_OCTAGONAL }, 

{ MOV_CH_LAYOUT_CUBE, AV_CH_LAYOUT_QUAD | 
AV_CH_TOP_FRONT_LEFT |
AV_CH_TOP_FRONT_RIGHT |
AV_CH_TOP_BACK_LEFT |
AV_CH_TOP_BACK_RIGHT },

{ MOV_CH_LAYOUT_MPEG_7_1_A, AV_CH_LAYOUT_7POINT1_WIDE }, 
{ MOV_CH_LAYOUT_MPEG_7_1_B, AV_CH_LAYOUT_7POINT1_WIDE }, 
{ MOV_CH_LAYOUT_EMAGIC_DEFAULT_7_1, AV_CH_LAYOUT_7POINT1_WIDE }, 
{ MOV_CH_LAYOUT_EAC3_7_1_B, AV_CH_LAYOUT_7POINT1_WIDE }, 
{ MOV_CH_LAYOUT_DTS_7_1, AV_CH_LAYOUT_7POINT1_WIDE }, 

{ MOV_CH_LAYOUT_MPEG_7_1_C, AV_CH_LAYOUT_7POINT1 }, 
{ MOV_CH_LAYOUT_EAC3_7_1_A, AV_CH_LAYOUT_7POINT1 }, 

{ MOV_CH_LAYOUT_SMPTE_DTV, AV_CH_LAYOUT_5POINT1 | 
AV_CH_LAYOUT_STEREO_DOWNMIX },

{ MOV_CH_LAYOUT_EAC3_7_1_C, AV_CH_LAYOUT_5POINT1 | 
AV_CH_SURROUND_DIRECT_LEFT |
AV_CH_SURROUND_DIRECT_RIGHT },

{ MOV_CH_LAYOUT_EAC3_7_1_D, AV_CH_LAYOUT_5POINT1 | 
AV_CH_WIDE_LEFT |
AV_CH_WIDE_RIGHT },

{ MOV_CH_LAYOUT_EAC3_7_1_E, AV_CH_LAYOUT_5POINT1 | 
AV_CH_TOP_FRONT_LEFT |
AV_CH_TOP_FRONT_RIGHT },

{ MOV_CH_LAYOUT_EAC3_7_1_F, AV_CH_LAYOUT_5POINT1 | 
AV_CH_BACK_CENTER |
AV_CH_TOP_CENTER },

{ MOV_CH_LAYOUT_EAC3_7_1_G, AV_CH_LAYOUT_5POINT1 | 
AV_CH_BACK_CENTER |
AV_CH_TOP_FRONT_CENTER },

{ MOV_CH_LAYOUT_EAC3_7_1_H, AV_CH_LAYOUT_5POINT1 | 
AV_CH_TOP_CENTER |
AV_CH_TOP_FRONT_CENTER },

{ MOV_CH_LAYOUT_DTS_8_0_A, AV_CH_LAYOUT_2_2 | 
AV_CH_BACK_LEFT |
AV_CH_BACK_RIGHT |
AV_CH_FRONT_LEFT_OF_CENTER |
AV_CH_FRONT_RIGHT_OF_CENTER },

{ MOV_CH_LAYOUT_DTS_8_0_B, AV_CH_LAYOUT_5POINT0 | 
AV_CH_FRONT_LEFT_OF_CENTER |
AV_CH_FRONT_RIGHT_OF_CENTER |
AV_CH_BACK_CENTER },
{ 0, 0 },
};

static const struct MovChannelLayoutMap mov_ch_layout_map_9ch[] = {
{ MOV_CH_LAYOUT_DTS_8_1_A, AV_CH_LAYOUT_2_2 | 
AV_CH_BACK_LEFT |
AV_CH_BACK_RIGHT |
AV_CH_FRONT_LEFT_OF_CENTER |
AV_CH_FRONT_RIGHT_OF_CENTER |
AV_CH_LOW_FREQUENCY },

{ MOV_CH_LAYOUT_DTS_8_1_B, AV_CH_LAYOUT_7POINT1_WIDE | 
AV_CH_BACK_CENTER },
{ 0, 0 },
};

static const struct MovChannelLayoutMap * const mov_ch_layout_map[] = {
mov_ch_layout_map_misc,
mov_ch_layout_map_1ch,
mov_ch_layout_map_2ch,
mov_ch_layout_map_3ch,
mov_ch_layout_map_4ch,
mov_ch_layout_map_5ch,
mov_ch_layout_map_6ch,
mov_ch_layout_map_7ch,
mov_ch_layout_map_8ch,
mov_ch_layout_map_9ch,
};

static const enum MovChannelLayoutTag mov_ch_layouts_aac[] = {
MOV_CH_LAYOUT_MONO,
MOV_CH_LAYOUT_STEREO,
MOV_CH_LAYOUT_AC3_1_0_1,
MOV_CH_LAYOUT_MPEG_3_0_B,
MOV_CH_LAYOUT_ITU_2_1,
MOV_CH_LAYOUT_DVD_4,
MOV_CH_LAYOUT_QUADRAPHONIC,
MOV_CH_LAYOUT_MPEG_4_0_B,
MOV_CH_LAYOUT_ITU_2_2,
MOV_CH_LAYOUT_AC3_2_1_1,
MOV_CH_LAYOUT_DTS_3_1,
MOV_CH_LAYOUT_MPEG_5_0_D,
MOV_CH_LAYOUT_DVD_18,
MOV_CH_LAYOUT_DTS_4_1,
MOV_CH_LAYOUT_MPEG_5_1_D,
MOV_CH_LAYOUT_AAC_6_0,
MOV_CH_LAYOUT_DTS_6_0_A,
MOV_CH_LAYOUT_AAC_6_1,
MOV_CH_LAYOUT_AAC_7_0,
MOV_CH_LAYOUT_DTS_6_1_A,
MOV_CH_LAYOUT_AAC_OCTAGONAL,
MOV_CH_LAYOUT_MPEG_7_1_B,
MOV_CH_LAYOUT_DTS_8_0_A,
0,
};

static const enum MovChannelLayoutTag mov_ch_layouts_ac3[] = {
MOV_CH_LAYOUT_MONO,
MOV_CH_LAYOUT_STEREO,
MOV_CH_LAYOUT_AC3_1_0_1,
MOV_CH_LAYOUT_AC3_3_0,
MOV_CH_LAYOUT_ITU_2_1,
MOV_CH_LAYOUT_DVD_4,
MOV_CH_LAYOUT_AC3_3_1,
MOV_CH_LAYOUT_ITU_2_2,
MOV_CH_LAYOUT_AC3_2_1_1,
MOV_CH_LAYOUT_AC3_3_0_1,
MOV_CH_LAYOUT_MPEG_5_0_C,
MOV_CH_LAYOUT_DVD_18,
MOV_CH_LAYOUT_AC3_3_1_1,
MOV_CH_LAYOUT_MPEG_5_1_C,
0,
};

static const enum MovChannelLayoutTag mov_ch_layouts_alac[] = {
MOV_CH_LAYOUT_MONO,
MOV_CH_LAYOUT_STEREO,
MOV_CH_LAYOUT_MPEG_3_0_B,
MOV_CH_LAYOUT_MPEG_4_0_B,
MOV_CH_LAYOUT_MPEG_5_0_D,
MOV_CH_LAYOUT_MPEG_5_1_D,
MOV_CH_LAYOUT_AAC_6_1,
MOV_CH_LAYOUT_MPEG_7_1_B,
0,
};

static const enum MovChannelLayoutTag mov_ch_layouts_wav[] = {
MOV_CH_LAYOUT_MONO,
MOV_CH_LAYOUT_STEREO,
MOV_CH_LAYOUT_MATRIXSTEREO,
MOV_CH_LAYOUT_MPEG_3_0_A,
MOV_CH_LAYOUT_QUADRAPHONIC,
MOV_CH_LAYOUT_MPEG_5_0_A,
MOV_CH_LAYOUT_MPEG_5_1_A,
MOV_CH_LAYOUT_MPEG_6_1_A,
MOV_CH_LAYOUT_MPEG_7_1_A,
MOV_CH_LAYOUT_MPEG_7_1_C,
MOV_CH_LAYOUT_SMPTE_DTV,
0,
};

static const struct {
enum AVCodecID codec_id;
const enum MovChannelLayoutTag *layouts;
} mov_codec_ch_layouts[] = {
{ AV_CODEC_ID_AAC, mov_ch_layouts_aac },
{ AV_CODEC_ID_AC3, mov_ch_layouts_ac3 },
{ AV_CODEC_ID_ALAC, mov_ch_layouts_alac },
{ AV_CODEC_ID_PCM_U8, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S8, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S16LE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S16BE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S24LE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S24BE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S32LE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_S32BE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_F32LE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_F32BE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_F64LE, mov_ch_layouts_wav },
{ AV_CODEC_ID_PCM_F64BE, mov_ch_layouts_wav },
{ AV_CODEC_ID_NONE, NULL },
};

uint64_t ff_mov_get_channel_layout(uint32_t tag, uint32_t bitmap)
{
int i, channels;
const struct MovChannelLayoutMap *layout_map;


if (tag == MOV_CH_LAYOUT_USE_DESCRIPTIONS)
return 0;


if (tag == MOV_CH_LAYOUT_USE_BITMAP)
return bitmap < 0x40000 ? bitmap : 0;


channels = tag & 0xFFFF;
if (channels > 9)
channels = 0;
layout_map = mov_ch_layout_map[channels];


for (i = 0; layout_map[i].tag != 0; i++) {
if (layout_map[i].tag == tag)
break;
}
return layout_map[i].layout;
}

static uint32_t mov_get_channel_label(uint32_t label)
{
if (label == 0)
return 0;
if (label <= 18)
return 1U << (label - 1);
if (label == 38)
return AV_CH_STEREO_LEFT;
if (label == 39)
return AV_CH_STEREO_RIGHT;
return 0;
}

uint32_t ff_mov_get_channel_layout_tag(enum AVCodecID codec_id,
uint64_t channel_layout,
uint32_t *bitmap)
{
int i, j;
uint32_t tag = 0;
const enum MovChannelLayoutTag *layouts = NULL;


for (i = 0; mov_codec_ch_layouts[i].codec_id != AV_CODEC_ID_NONE; i++) {
if (mov_codec_ch_layouts[i].codec_id == codec_id)
break;
}
if (mov_codec_ch_layouts[i].codec_id != AV_CODEC_ID_NONE)
layouts = mov_codec_ch_layouts[i].layouts;

if (layouts) {
int channels;
const struct MovChannelLayoutMap *layout_map;


channels = av_get_channel_layout_nb_channels(channel_layout);
if (channels > 9)
channels = 0;
layout_map = mov_ch_layout_map[channels];


for (i = 0; layouts[i] != 0; i++) {
if ((layouts[i] & 0xFFFF) != channels)
continue;
for (j = 0; layout_map[j].tag != 0; j++) {
if (layout_map[j].tag == layouts[i] &&
layout_map[j].layout == channel_layout)
break;
}
if (layout_map[j].tag)
break;
}
tag = layouts[i];
}


if (tag == 0 && channel_layout > 0 && channel_layout < 0x40000) {
tag = MOV_CH_LAYOUT_USE_BITMAP;
*bitmap = (uint32_t)channel_layout;
} else
*bitmap = 0;



return tag;
}

int ff_mov_read_chan(AVFormatContext *s, AVIOContext *pb, AVStream *st,
int64_t size)
{
uint32_t layout_tag, bitmap, num_descr, label_mask;
int i;

if (size < 12)
return AVERROR_INVALIDDATA;

layout_tag = avio_rb32(pb);
bitmap = avio_rb32(pb);
num_descr = avio_rb32(pb);

av_log(s, AV_LOG_TRACE, "chan: layout=%"PRIu32" "
"bitmap=%"PRIu32" num_descr=%"PRIu32"\n",
layout_tag, bitmap, num_descr);

if (size < 12ULL + num_descr * 20ULL)
return 0;

label_mask = 0;
for (i = 0; i < num_descr; i++) {
uint32_t label;
if (pb->eof_reached) {
av_log(s, AV_LOG_ERROR,
"reached EOF while reading channel layout\n");
return AVERROR_INVALIDDATA;
}
label = avio_rb32(pb); 
avio_rb32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
avio_rl32(pb); 
size -= 20;
if (layout_tag == 0) {
uint32_t mask_incr = mov_get_channel_label(label);
if (mask_incr == 0) {
label_mask = 0;
break;
}
label_mask |= mask_incr;
}
}
if (layout_tag == 0) {
if (label_mask)
st->codecpar->channel_layout = label_mask;
} else
st->codecpar->channel_layout = ff_mov_get_channel_layout(layout_tag, bitmap);
avio_skip(pb, size - 12);

return 0;
}
