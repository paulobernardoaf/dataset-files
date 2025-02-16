



















#if !defined(AVCODEC_MLP_PARSE_H)
#define AVCODEC_MLP_PARSE_H

#include "get_bits.h"

typedef struct MLPHeaderInfo
{
int stream_type; 
int header_size; 

int group1_bits; 
int group2_bits; 

int group1_samplerate; 
int group2_samplerate; 

int channel_arrangement;

int channel_modifier_thd_stream0; 
int channel_modifier_thd_stream1; 
int channel_modifier_thd_stream2; 

int channels_mlp; 
int channels_thd_stream1; 
int channels_thd_stream2; 
uint64_t channel_layout_mlp; 
uint64_t channel_layout_thd_stream1; 
uint64_t channel_layout_thd_stream2; 

int access_unit_size; 
int access_unit_size_pow2; 

int is_vbr; 
int peak_bitrate; 

int num_substreams; 
} MLPHeaderInfo;

static const uint8_t thd_chancount[13] = {

2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1
};

static const uint64_t thd_layout[13] = {
AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT, 
AV_CH_FRONT_CENTER, 
AV_CH_LOW_FREQUENCY, 
AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT, 
AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT, 
AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER, 
AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT, 
AV_CH_BACK_CENTER, 
AV_CH_TOP_CENTER, 
AV_CH_SURROUND_DIRECT_LEFT|AV_CH_SURROUND_DIRECT_RIGHT, 
AV_CH_WIDE_LEFT|AV_CH_WIDE_RIGHT, 
AV_CH_TOP_FRONT_CENTER, 
AV_CH_LOW_FREQUENCY_2, 
};

static inline int mlp_samplerate(int in)
{
if (in == 0xF)
return 0;

return (in & 8 ? 44100 : 48000) << (in & 7) ;
}

static inline int truehd_channels(int chanmap)
{
int channels = 0, i;

for (i = 0; i < 13; i++)
channels += thd_chancount[i] * ((chanmap >> i) & 1);

return channels;
}

static inline uint64_t truehd_layout(int chanmap)
{
int i;
uint64_t layout = 0;

for (i = 0; i < 13; i++)
layout |= thd_layout[i] * ((chanmap >> i) & 1);

return layout;
}

int ff_mlp_read_major_sync(void *log, MLPHeaderInfo *mh, GetBitContext *gb);

#endif 
