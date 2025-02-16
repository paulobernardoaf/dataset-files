#include "aac.h"
#define CB_TOT 12
#define CB_TOT_ALL 15
#define AAC_MAX_CHANNELS 16
extern const uint8_t *ff_aac_swb_size_1024[];
extern const int ff_aac_swb_size_1024_len;
extern const uint8_t *ff_aac_swb_size_128[];
extern const int ff_aac_swb_size_128_len;
static const int64_t aac_normal_chan_layouts[7] = {
AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_4POINT0,
AV_CH_LAYOUT_5POINT0_BACK,
AV_CH_LAYOUT_5POINT1_BACK,
AV_CH_LAYOUT_7POINT1,
};
static const uint8_t aac_chan_configs[AAC_MAX_CHANNELS][6] = {
{1, TYPE_SCE}, 
{1, TYPE_CPE}, 
{2, TYPE_SCE, TYPE_CPE}, 
{3, TYPE_SCE, TYPE_CPE, TYPE_SCE}, 
{3, TYPE_SCE, TYPE_CPE, TYPE_CPE}, 
{4, TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_LFE}, 
{0}, 
{5, TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_CPE, TYPE_LFE}, 
};
static const uint8_t aac_chan_maps[AAC_MAX_CHANNELS][AAC_MAX_CHANNELS] = {
{ 0 },
{ 0, 1 },
{ 2, 0, 1 },
{ 2, 0, 1, 3 },
{ 2, 0, 1, 3, 4 },
{ 2, 0, 1, 4, 5, 3 },
{ 0 },
{ 2, 0, 1, 6, 7, 4, 5, 3 },
};
static const int mpeg4audio_sample_rates[16] = {
96000, 88200, 64000, 48000, 44100, 32000,
24000, 22050, 16000, 12000, 11025, 8000, 7350
};
static const uint8_t run_value_bits_long[64] = {
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 10,
10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 15
};
static const uint8_t run_value_bits_short[16] = {
3, 3, 3, 3, 3, 3, 3, 6, 6, 6, 6, 6, 6, 6, 6, 9
};
static const uint8_t tns_min_sfb_short[16] = {
2, 2, 2, 3, 3, 4, 6, 6, 8, 10, 10, 12, 12, 12, 12, 12
};
static const uint8_t tns_min_sfb_long[16] = {
12, 13, 15, 16, 17, 20, 25, 26, 24, 28, 30, 31, 31, 31, 31, 31
};
static const uint8_t * const tns_min_sfb[2] = {
tns_min_sfb_long, tns_min_sfb_short
};
static const uint8_t * const run_value_bits[2] = {
run_value_bits_long, run_value_bits_short
};
static const uint8_t aac_cb_out_map[CB_TOT_ALL] = {0,1,2,3,4,5,6,7,8,9,10,11,13,14,15};
static const uint8_t aac_cb_in_map[CB_TOT_ALL+1] = {0,1,2,3,4,5,6,7,8,9,10,11,0,12,13,14};
static const uint8_t aac_cb_range [12] = {0, 3, 3, 3, 3, 9, 9, 8, 8, 13, 13, 17};
static const uint8_t aac_cb_maxval[12] = {0, 1, 1, 2, 2, 4, 4, 7, 7, 12, 12, 16};
static const unsigned char aac_maxval_cb[] = {
0, 1, 3, 5, 5, 7, 7, 7, 9, 9, 9, 9, 9, 11
};
static const int aacenc_profiles[] = {
FF_PROFILE_AAC_MAIN,
FF_PROFILE_AAC_LOW,
FF_PROFILE_AAC_LTP,
FF_PROFILE_MPEG2_AAC_LOW,
};
