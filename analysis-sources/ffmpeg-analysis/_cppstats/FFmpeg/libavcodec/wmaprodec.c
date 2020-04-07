#include <inttypes.h>
#include "libavutil/ffmath.h"
#include "libavutil/float_dsp.h"
#include "libavutil/intfloat.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "wmaprodata.h"
#include "sinewin.h"
#include "wma.h"
#include "wma_common.h"
#define WMAPRO_MAX_CHANNELS 8 
#define MAX_SUBFRAMES 32 
#define MAX_BANDS 29 
#define MAX_FRAMESIZE 32768 
#define XMA_MAX_STREAMS 8
#define XMA_MAX_CHANNELS_STREAM 2
#define XMA_MAX_CHANNELS (XMA_MAX_STREAMS * XMA_MAX_CHANNELS_STREAM)
#define WMAPRO_BLOCK_MIN_BITS 6 
#define WMAPRO_BLOCK_MAX_BITS 13 
#define WMAPRO_BLOCK_MIN_SIZE (1 << WMAPRO_BLOCK_MIN_BITS) 
#define WMAPRO_BLOCK_MAX_SIZE (1 << WMAPRO_BLOCK_MAX_BITS) 
#define WMAPRO_BLOCK_SIZES (WMAPRO_BLOCK_MAX_BITS - WMAPRO_BLOCK_MIN_BITS + 1) 
#define VLCBITS 9
#define SCALEVLCBITS 8
#define VEC4MAXDEPTH ((HUFF_VEC4_MAXBITS+VLCBITS-1)/VLCBITS)
#define VEC2MAXDEPTH ((HUFF_VEC2_MAXBITS+VLCBITS-1)/VLCBITS)
#define VEC1MAXDEPTH ((HUFF_VEC1_MAXBITS+VLCBITS-1)/VLCBITS)
#define SCALEMAXDEPTH ((HUFF_SCALE_MAXBITS+SCALEVLCBITS-1)/SCALEVLCBITS)
#define SCALERLMAXDEPTH ((HUFF_SCALE_RL_MAXBITS+VLCBITS-1)/VLCBITS)
static VLC sf_vlc; 
static VLC sf_rl_vlc; 
static VLC vec4_vlc; 
static VLC vec2_vlc; 
static VLC vec1_vlc; 
static VLC coef_vlc[2]; 
static float sin64[33]; 
typedef struct WMAProChannelCtx {
int16_t prev_block_len; 
uint8_t transmit_coefs;
uint8_t num_subframes;
uint16_t subframe_len[MAX_SUBFRAMES]; 
uint16_t subframe_offset[MAX_SUBFRAMES]; 
uint8_t cur_subframe; 
uint16_t decoded_samples; 
uint8_t grouped; 
int quant_step; 
int8_t reuse_sf; 
int8_t scale_factor_step; 
int max_scale_factor; 
int saved_scale_factors[2][MAX_BANDS]; 
int8_t scale_factor_idx; 
int* scale_factors; 
uint8_t table_idx; 
float* coeffs; 
uint16_t num_vec_coeffs; 
DECLARE_ALIGNED(32, float, out)[WMAPRO_BLOCK_MAX_SIZE + WMAPRO_BLOCK_MAX_SIZE / 2]; 
} WMAProChannelCtx;
typedef struct WMAProChannelGrp {
uint8_t num_channels; 
int8_t transform; 
int8_t transform_band[MAX_BANDS]; 
float decorrelation_matrix[WMAPRO_MAX_CHANNELS*WMAPRO_MAX_CHANNELS];
float* channel_data[WMAPRO_MAX_CHANNELS]; 
} WMAProChannelGrp;
typedef struct WMAProDecodeCtx {
AVCodecContext* avctx; 
AVFloatDSPContext *fdsp;
uint8_t frame_data[MAX_FRAMESIZE +
AV_INPUT_BUFFER_PADDING_SIZE];
PutBitContext pb; 
FFTContext mdct_ctx[WMAPRO_BLOCK_SIZES]; 
DECLARE_ALIGNED(32, float, tmp)[WMAPRO_BLOCK_MAX_SIZE]; 
const float* windows[WMAPRO_BLOCK_SIZES]; 
uint32_t decode_flags; 
uint8_t len_prefix; 
uint8_t dynamic_range_compression; 
uint8_t bits_per_sample; 
uint16_t samples_per_frame; 
uint16_t log2_frame_size;
int8_t lfe_channel; 
uint8_t max_num_subframes;
uint8_t subframe_len_bits; 
uint8_t max_subframe_len_bit; 
uint16_t min_samples_per_subframe;
int8_t num_sfb[WMAPRO_BLOCK_SIZES]; 
int16_t sfb_offsets[WMAPRO_BLOCK_SIZES][MAX_BANDS]; 
int8_t sf_offsets[WMAPRO_BLOCK_SIZES][WMAPRO_BLOCK_SIZES][MAX_BANDS]; 
int16_t subwoofer_cutoffs[WMAPRO_BLOCK_SIZES]; 
GetBitContext pgb; 
int next_packet_start; 
uint8_t packet_offset; 
uint8_t packet_sequence_number; 
int num_saved_bits; 
int frame_offset; 
int subframe_offset; 
uint8_t packet_loss; 
uint8_t packet_done; 
uint8_t eof_done; 
uint32_t frame_num; 
GetBitContext gb; 
int buf_bit_size; 
uint8_t drc_gain; 
int8_t skip_frame; 
int8_t parsed_all_subframes; 
uint8_t skip_packets; 
int16_t subframe_len; 
int8_t nb_channels; 
int8_t channels_for_cur_subframe; 
int8_t channel_indexes_for_cur_subframe[WMAPRO_MAX_CHANNELS];
int8_t num_bands; 
int8_t transmit_num_vec_coeffs; 
int16_t* cur_sfb_offsets; 
uint8_t table_idx; 
int8_t esc_len; 
uint8_t num_chgroups; 
WMAProChannelGrp chgroup[WMAPRO_MAX_CHANNELS]; 
WMAProChannelCtx channel[WMAPRO_MAX_CHANNELS]; 
} WMAProDecodeCtx;
typedef struct XMADecodeCtx {
WMAProDecodeCtx xma[XMA_MAX_STREAMS];
AVFrame *frames[XMA_MAX_STREAMS];
int current_stream;
int num_streams;
float samples[XMA_MAX_CHANNELS][512 * 64];
int offset[XMA_MAX_STREAMS];
int start_channel[XMA_MAX_STREAMS];
} XMADecodeCtx;
static av_cold void dump_context(WMAProDecodeCtx *s)
{
#define PRINT(a, b) av_log(s->avctx, AV_LOG_DEBUG, " %s = %d\n", a, b);
#define PRINT_HEX(a, b) av_log(s->avctx, AV_LOG_DEBUG, " %s = %"PRIx32"\n", a, b);
PRINT("ed sample bit depth", s->bits_per_sample);
PRINT_HEX("ed decode flags", s->decode_flags);
PRINT("samples per frame", s->samples_per_frame);
PRINT("log2 frame size", s->log2_frame_size);
PRINT("max num subframes", s->max_num_subframes);
PRINT("len prefix", s->len_prefix);
PRINT("num channels", s->nb_channels);
}
static av_cold int decode_end(WMAProDecodeCtx *s)
{
int i;
av_freep(&s->fdsp);
for (i = 0; i < WMAPRO_BLOCK_SIZES; i++)
ff_mdct_end(&s->mdct_ctx[i]);
return 0;
}
static av_cold int wmapro_decode_end(AVCodecContext *avctx)
{
WMAProDecodeCtx *s = avctx->priv_data;
decode_end(s);
return 0;
}
static av_cold int get_rate(AVCodecContext *avctx)
{
if (avctx->codec_id != AV_CODEC_ID_WMAPRO) { 
if (avctx->sample_rate > 44100)
return 48000;
else if (avctx->sample_rate > 32000)
return 44100;
else if (avctx->sample_rate > 24000)
return 32000;
return 24000;
}
return avctx->sample_rate;
}
static av_cold int decode_init(WMAProDecodeCtx *s, AVCodecContext *avctx, int num_stream)
{
uint8_t *edata_ptr = avctx->extradata;
unsigned int channel_mask;
int i, bits;
int log2_max_num_subframes;
int num_possible_block_sizes;
if (avctx->codec_id == AV_CODEC_ID_XMA1 || avctx->codec_id == AV_CODEC_ID_XMA2)
avctx->block_align = 2048;
if (!avctx->block_align) {
av_log(avctx, AV_LOG_ERROR, "block_align is not set\n");
return AVERROR(EINVAL);
}
s->avctx = avctx;
init_put_bits(&s->pb, s->frame_data, MAX_FRAMESIZE);
avctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
av_log(avctx, AV_LOG_DEBUG, "extradata:\n");
for (i = 0; i < avctx->extradata_size; i++)
av_log(avctx, AV_LOG_DEBUG, "[%x] ", avctx->extradata[i]);
av_log(avctx, AV_LOG_DEBUG, "\n");
if (avctx->codec_id == AV_CODEC_ID_XMA2 && avctx->extradata_size == 34) { 
s->decode_flags = 0x10d6;
s->bits_per_sample = 16;
channel_mask = 0; 
if ((num_stream+1) * XMA_MAX_CHANNELS_STREAM > avctx->channels) 
s->nb_channels = 1;
else
s->nb_channels = 2;
} else if (avctx->codec_id == AV_CODEC_ID_XMA2) { 
s->decode_flags = 0x10d6;
s->bits_per_sample = 16;
channel_mask = 0; 
s->nb_channels = edata_ptr[32 + ((edata_ptr[0]==3)?0:8) + 4*num_stream + 0]; 
} else if (avctx->codec_id == AV_CODEC_ID_XMA1) { 
s->decode_flags = 0x10d6;
s->bits_per_sample = 16;
channel_mask = 0; 
s->nb_channels = edata_ptr[8 + 20*num_stream + 17]; 
} else if (avctx->codec_id == AV_CODEC_ID_WMAPRO && avctx->extradata_size >= 18) {
s->decode_flags = AV_RL16(edata_ptr+14);
channel_mask = AV_RL32(edata_ptr+2);
s->bits_per_sample = AV_RL16(edata_ptr);
s->nb_channels = avctx->channels;
if (s->bits_per_sample > 32 || s->bits_per_sample < 1) {
avpriv_request_sample(avctx, "bits per sample is %d", s->bits_per_sample);
return AVERROR_PATCHWELCOME;
}
} else {
avpriv_request_sample(avctx, "Unknown extradata size");
return AVERROR_PATCHWELCOME;
}
s->log2_frame_size = av_log2(avctx->block_align) + 4;
if (s->log2_frame_size > 25) {
avpriv_request_sample(avctx, "Large block align");
return AVERROR_PATCHWELCOME;
}
if (avctx->codec_id != AV_CODEC_ID_WMAPRO)
s->skip_frame = 0;
else
s->skip_frame = 1; 
s->packet_loss = 1;
s->len_prefix = (s->decode_flags & 0x40);
if (avctx->codec_id == AV_CODEC_ID_WMAPRO) {
bits = ff_wma_get_frame_len_bits(avctx->sample_rate, 3, s->decode_flags);
if (bits > WMAPRO_BLOCK_MAX_BITS) {
avpriv_request_sample(avctx, "14-bit block sizes");
return AVERROR_PATCHWELCOME;
}
s->samples_per_frame = 1 << bits;
} else {
s->samples_per_frame = 512;
}
log2_max_num_subframes = ((s->decode_flags & 0x38) >> 3);
s->max_num_subframes = 1 << log2_max_num_subframes;
if (s->max_num_subframes == 16 || s->max_num_subframes == 4)
s->max_subframe_len_bit = 1;
s->subframe_len_bits = av_log2(log2_max_num_subframes) + 1;
num_possible_block_sizes = log2_max_num_subframes + 1;
s->min_samples_per_subframe = s->samples_per_frame / s->max_num_subframes;
s->dynamic_range_compression = (s->decode_flags & 0x80);
if (s->max_num_subframes > MAX_SUBFRAMES) {
av_log(avctx, AV_LOG_ERROR, "invalid number of subframes %"PRId8"\n",
s->max_num_subframes);
return AVERROR_INVALIDDATA;
}
if (s->min_samples_per_subframe < WMAPRO_BLOCK_MIN_SIZE) {
av_log(avctx, AV_LOG_ERROR, "min_samples_per_subframe of %d too small\n",
s->min_samples_per_subframe);
return AVERROR_INVALIDDATA;
}
if (s->avctx->sample_rate <= 0) {
av_log(avctx, AV_LOG_ERROR, "invalid sample rate\n");
return AVERROR_INVALIDDATA;
}
if (s->nb_channels <= 0) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels %d\n",
s->nb_channels);
return AVERROR_INVALIDDATA;
} else if (avctx->codec_id != AV_CODEC_ID_WMAPRO && s->nb_channels > XMA_MAX_CHANNELS_STREAM) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels per XMA stream %d\n",
s->nb_channels);
return AVERROR_INVALIDDATA;
} else if (s->nb_channels > WMAPRO_MAX_CHANNELS || s->nb_channels > avctx->channels) {
avpriv_request_sample(avctx,
"More than %d channels", WMAPRO_MAX_CHANNELS);
return AVERROR_PATCHWELCOME;
}
for (i = 0; i < s->nb_channels; i++)
s->channel[i].prev_block_len = s->samples_per_frame;
s->lfe_channel = -1;
if (channel_mask & 8) {
unsigned int mask;
for (mask = 1; mask < 16; mask <<= 1) {
if (channel_mask & mask)
++s->lfe_channel;
}
}
INIT_VLC_STATIC(&sf_vlc, SCALEVLCBITS, HUFF_SCALE_SIZE,
scale_huffbits, 1, 1,
scale_huffcodes, 2, 2, 616);
INIT_VLC_STATIC(&sf_rl_vlc, VLCBITS, HUFF_SCALE_RL_SIZE,
scale_rl_huffbits, 1, 1,
scale_rl_huffcodes, 4, 4, 1406);
INIT_VLC_STATIC(&coef_vlc[0], VLCBITS, HUFF_COEF0_SIZE,
coef0_huffbits, 1, 1,
coef0_huffcodes, 4, 4, 2108);
INIT_VLC_STATIC(&coef_vlc[1], VLCBITS, HUFF_COEF1_SIZE,
coef1_huffbits, 1, 1,
coef1_huffcodes, 4, 4, 3912);
INIT_VLC_STATIC(&vec4_vlc, VLCBITS, HUFF_VEC4_SIZE,
vec4_huffbits, 1, 1,
vec4_huffcodes, 2, 2, 604);
INIT_VLC_STATIC(&vec2_vlc, VLCBITS, HUFF_VEC2_SIZE,
vec2_huffbits, 1, 1,
vec2_huffcodes, 2, 2, 562);
INIT_VLC_STATIC(&vec1_vlc, VLCBITS, HUFF_VEC1_SIZE,
vec1_huffbits, 1, 1,
vec1_huffcodes, 2, 2, 562);
for (i = 0; i < num_possible_block_sizes; i++) {
int subframe_len = s->samples_per_frame >> i;
int x;
int band = 1;
int rate = get_rate(avctx);
s->sfb_offsets[i][0] = 0;
for (x = 0; x < MAX_BANDS-1 && s->sfb_offsets[i][band - 1] < subframe_len; x++) {
int offset = (subframe_len * 2 * critical_freq[x]) / rate + 2;
offset &= ~3;
if (offset > s->sfb_offsets[i][band - 1])
s->sfb_offsets[i][band++] = offset;
if (offset >= subframe_len)
break;
}
s->sfb_offsets[i][band - 1] = subframe_len;
s->num_sfb[i] = band - 1;
if (s->num_sfb[i] <= 0) {
av_log(avctx, AV_LOG_ERROR, "num_sfb invalid\n");
return AVERROR_INVALIDDATA;
}
}
for (i = 0; i < num_possible_block_sizes; i++) {
int b;
for (b = 0; b < s->num_sfb[i]; b++) {
int x;
int offset = ((s->sfb_offsets[i][b]
+ s->sfb_offsets[i][b + 1] - 1) << i) >> 1;
for (x = 0; x < num_possible_block_sizes; x++) {
int v = 0;
while (s->sfb_offsets[x][v + 1] << x < offset) {
v++;
av_assert0(v < MAX_BANDS);
}
s->sf_offsets[i][x][b] = v;
}
}
}
s->fdsp = avpriv_float_dsp_alloc(avctx->flags & AV_CODEC_FLAG_BITEXACT);
if (!s->fdsp)
return AVERROR(ENOMEM);
for (i = 0; i < WMAPRO_BLOCK_SIZES; i++)
ff_mdct_init(&s->mdct_ctx[i], WMAPRO_BLOCK_MIN_BITS+1+i, 1,
1.0 / (1 << (WMAPRO_BLOCK_MIN_BITS + i - 1))
/ (1ll << (s->bits_per_sample - 1)));
for (i = 0; i < WMAPRO_BLOCK_SIZES; i++) {
const int win_idx = WMAPRO_BLOCK_MAX_BITS - i;
ff_init_ff_sine_windows(win_idx);
s->windows[WMAPRO_BLOCK_SIZES - i - 1] = ff_sine_windows[win_idx];
}
for (i = 0; i < num_possible_block_sizes; i++) {
int block_size = s->samples_per_frame >> i;
int cutoff = (440*block_size + 3LL * (s->avctx->sample_rate >> 1) - 1)
/ s->avctx->sample_rate;
s->subwoofer_cutoffs[i] = av_clip(cutoff, 4, block_size);
}
for (i = 0; i < 33; i++)
sin64[i] = sin(i*M_PI / 64.0);
if (avctx->debug & FF_DEBUG_BITSTREAM)
dump_context(s);
avctx->channel_layout = channel_mask;
return 0;
}
static av_cold int wmapro_decode_init(AVCodecContext *avctx)
{
WMAProDecodeCtx *s = avctx->priv_data;
return decode_init(s, avctx, 0);
}
static int decode_subframe_length(WMAProDecodeCtx *s, int offset)
{
int frame_len_shift = 0;
int subframe_len;
if (offset == s->samples_per_frame - s->min_samples_per_subframe)
return s->min_samples_per_subframe;
if (get_bits_left(&s->gb) < 1)
return AVERROR_INVALIDDATA;
if (s->max_subframe_len_bit) {
if (get_bits1(&s->gb))
frame_len_shift = 1 + get_bits(&s->gb, s->subframe_len_bits-1);
} else
frame_len_shift = get_bits(&s->gb, s->subframe_len_bits);
subframe_len = s->samples_per_frame >> frame_len_shift;
if (subframe_len < s->min_samples_per_subframe ||
subframe_len > s->samples_per_frame) {
av_log(s->avctx, AV_LOG_ERROR, "broken frame: subframe_len %i\n",
subframe_len);
return AVERROR_INVALIDDATA;
}
return subframe_len;
}
static int decode_tilehdr(WMAProDecodeCtx *s)
{
uint16_t num_samples[WMAPRO_MAX_CHANNELS] = { 0 };
uint8_t contains_subframe[WMAPRO_MAX_CHANNELS]; 
int channels_for_cur_subframe = s->nb_channels; 
int fixed_channel_layout = 0; 
int min_channel_len = 0; 
int c;
for (c = 0; c < s->nb_channels; c++)
s->channel[c].num_subframes = 0;
if (s->max_num_subframes == 1 || get_bits1(&s->gb))
fixed_channel_layout = 1;
do {
int subframe_len;
for (c = 0; c < s->nb_channels; c++) {
if (num_samples[c] == min_channel_len) {
if (fixed_channel_layout || channels_for_cur_subframe == 1 ||
(min_channel_len == s->samples_per_frame - s->min_samples_per_subframe))
contains_subframe[c] = 1;
else
contains_subframe[c] = get_bits1(&s->gb);
} else
contains_subframe[c] = 0;
}
if ((subframe_len = decode_subframe_length(s, min_channel_len)) <= 0)
return AVERROR_INVALIDDATA;
min_channel_len += subframe_len;
for (c = 0; c < s->nb_channels; c++) {
WMAProChannelCtx* chan = &s->channel[c];
if (contains_subframe[c]) {
if (chan->num_subframes >= MAX_SUBFRAMES) {
av_log(s->avctx, AV_LOG_ERROR,
"broken frame: num subframes > 31\n");
return AVERROR_INVALIDDATA;
}
chan->subframe_len[chan->num_subframes] = subframe_len;
num_samples[c] += subframe_len;
++chan->num_subframes;
if (num_samples[c] > s->samples_per_frame) {
av_log(s->avctx, AV_LOG_ERROR, "broken frame: "
"channel len > samples_per_frame\n");
return AVERROR_INVALIDDATA;
}
} else if (num_samples[c] <= min_channel_len) {
if (num_samples[c] < min_channel_len) {
channels_for_cur_subframe = 0;
min_channel_len = num_samples[c];
}
++channels_for_cur_subframe;
}
}
} while (min_channel_len < s->samples_per_frame);
for (c = 0; c < s->nb_channels; c++) {
int i;
int offset = 0;
for (i = 0; i < s->channel[c].num_subframes; i++) {
ff_dlog(s->avctx, "frame[%"PRIu32"] channel[%i] subframe[%i]"
" len %i\n", s->frame_num, c, i,
s->channel[c].subframe_len[i]);
s->channel[c].subframe_offset[i] = offset;
offset += s->channel[c].subframe_len[i];
}
}
return 0;
}
static void decode_decorrelation_matrix(WMAProDecodeCtx *s,
WMAProChannelGrp *chgroup)
{
int i;
int offset = 0;
int8_t rotation_offset[WMAPRO_MAX_CHANNELS * WMAPRO_MAX_CHANNELS];
memset(chgroup->decorrelation_matrix, 0, s->nb_channels *
s->nb_channels * sizeof(*chgroup->decorrelation_matrix));
for (i = 0; i < chgroup->num_channels * (chgroup->num_channels - 1) >> 1; i++)
rotation_offset[i] = get_bits(&s->gb, 6);
for (i = 0; i < chgroup->num_channels; i++)
chgroup->decorrelation_matrix[chgroup->num_channels * i + i] =
get_bits1(&s->gb) ? 1.0 : -1.0;
for (i = 1; i < chgroup->num_channels; i++) {
int x;
for (x = 0; x < i; x++) {
int y;
for (y = 0; y < i + 1; y++) {
float v1 = chgroup->decorrelation_matrix[x * chgroup->num_channels + y];
float v2 = chgroup->decorrelation_matrix[i * chgroup->num_channels + y];
int n = rotation_offset[offset + x];
float sinv;
float cosv;
if (n < 32) {
sinv = sin64[n];
cosv = sin64[32 - n];
} else {
sinv = sin64[64 - n];
cosv = -sin64[n - 32];
}
chgroup->decorrelation_matrix[y + x * chgroup->num_channels] =
(v1 * sinv) - (v2 * cosv);
chgroup->decorrelation_matrix[y + i * chgroup->num_channels] =
(v1 * cosv) + (v2 * sinv);
}
}
offset += i;
}
}
static int decode_channel_transform(WMAProDecodeCtx* s)
{
int i;
s->num_chgroups = 0;
if (s->nb_channels > 1) {
int remaining_channels = s->channels_for_cur_subframe;
if (get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx,
"Channel transform bit");
return AVERROR_PATCHWELCOME;
}
for (s->num_chgroups = 0; remaining_channels &&
s->num_chgroups < s->channels_for_cur_subframe; s->num_chgroups++) {
WMAProChannelGrp* chgroup = &s->chgroup[s->num_chgroups];
float** channel_data = chgroup->channel_data;
chgroup->num_channels = 0;
chgroup->transform = 0;
if (remaining_channels > 2) {
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int channel_idx = s->channel_indexes_for_cur_subframe[i];
if (!s->channel[channel_idx].grouped
&& get_bits1(&s->gb)) {
++chgroup->num_channels;
s->channel[channel_idx].grouped = 1;
*channel_data++ = s->channel[channel_idx].coeffs;
}
}
} else {
chgroup->num_channels = remaining_channels;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int channel_idx = s->channel_indexes_for_cur_subframe[i];
if (!s->channel[channel_idx].grouped)
*channel_data++ = s->channel[channel_idx].coeffs;
s->channel[channel_idx].grouped = 1;
}
}
if (chgroup->num_channels == 2) {
if (get_bits1(&s->gb)) {
if (get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx,
"Unknown channel transform type");
return AVERROR_PATCHWELCOME;
}
} else {
chgroup->transform = 1;
if (s->nb_channels == 2) {
chgroup->decorrelation_matrix[0] = 1.0;
chgroup->decorrelation_matrix[1] = -1.0;
chgroup->decorrelation_matrix[2] = 1.0;
chgroup->decorrelation_matrix[3] = 1.0;
} else {
chgroup->decorrelation_matrix[0] = 0.70703125;
chgroup->decorrelation_matrix[1] = -0.70703125;
chgroup->decorrelation_matrix[2] = 0.70703125;
chgroup->decorrelation_matrix[3] = 0.70703125;
}
}
} else if (chgroup->num_channels > 2) {
if (get_bits1(&s->gb)) {
chgroup->transform = 1;
if (get_bits1(&s->gb)) {
decode_decorrelation_matrix(s, chgroup);
} else {
if (chgroup->num_channels > 6) {
avpriv_request_sample(s->avctx,
"Coupled channels > 6");
} else {
memcpy(chgroup->decorrelation_matrix,
default_decorrelation[chgroup->num_channels],
chgroup->num_channels * chgroup->num_channels *
sizeof(*chgroup->decorrelation_matrix));
}
}
}
}
if (chgroup->transform) {
if (!get_bits1(&s->gb)) {
int i;
for (i = 0; i < s->num_bands; i++) {
chgroup->transform_band[i] = get_bits1(&s->gb);
}
} else {
memset(chgroup->transform_band, 1, s->num_bands);
}
}
remaining_channels -= chgroup->num_channels;
}
}
return 0;
}
static int decode_coeffs(WMAProDecodeCtx *s, int c)
{
static const uint32_t fval_tab[16] = {
0x00000000, 0x3f800000, 0x40000000, 0x40400000,
0x40800000, 0x40a00000, 0x40c00000, 0x40e00000,
0x41000000, 0x41100000, 0x41200000, 0x41300000,
0x41400000, 0x41500000, 0x41600000, 0x41700000,
};
int vlctable;
VLC* vlc;
WMAProChannelCtx* ci = &s->channel[c];
int rl_mode = 0;
int cur_coeff = 0;
int num_zeros = 0;
const uint16_t* run;
const float* level;
ff_dlog(s->avctx, "decode coefficients for channel %i\n", c);
vlctable = get_bits1(&s->gb);
vlc = &coef_vlc[vlctable];
if (vlctable) {
run = coef1_run;
level = coef1_level;
} else {
run = coef0_run;
level = coef0_level;
}
while ((s->transmit_num_vec_coeffs || !rl_mode) &&
(cur_coeff + 3 < ci->num_vec_coeffs)) {
uint32_t vals[4];
int i;
unsigned int idx;
idx = get_vlc2(&s->gb, vec4_vlc.table, VLCBITS, VEC4MAXDEPTH);
if (idx == HUFF_VEC4_SIZE - 1) {
for (i = 0; i < 4; i += 2) {
idx = get_vlc2(&s->gb, vec2_vlc.table, VLCBITS, VEC2MAXDEPTH);
if (idx == HUFF_VEC2_SIZE - 1) {
uint32_t v0, v1;
v0 = get_vlc2(&s->gb, vec1_vlc.table, VLCBITS, VEC1MAXDEPTH);
if (v0 == HUFF_VEC1_SIZE - 1)
v0 += ff_wma_get_large_val(&s->gb);
v1 = get_vlc2(&s->gb, vec1_vlc.table, VLCBITS, VEC1MAXDEPTH);
if (v1 == HUFF_VEC1_SIZE - 1)
v1 += ff_wma_get_large_val(&s->gb);
vals[i ] = av_float2int(v0);
vals[i+1] = av_float2int(v1);
} else {
vals[i] = fval_tab[symbol_to_vec2[idx] >> 4 ];
vals[i+1] = fval_tab[symbol_to_vec2[idx] & 0xF];
}
}
} else {
vals[0] = fval_tab[ symbol_to_vec4[idx] >> 12 ];
vals[1] = fval_tab[(symbol_to_vec4[idx] >> 8) & 0xF];
vals[2] = fval_tab[(symbol_to_vec4[idx] >> 4) & 0xF];
vals[3] = fval_tab[ symbol_to_vec4[idx] & 0xF];
}
for (i = 0; i < 4; i++) {
if (vals[i]) {
uint32_t sign = get_bits1(&s->gb) - 1;
AV_WN32A(&ci->coeffs[cur_coeff], vals[i] ^ sign << 31);
num_zeros = 0;
} else {
ci->coeffs[cur_coeff] = 0;
rl_mode |= (++num_zeros > s->subframe_len >> 8);
}
++cur_coeff;
}
}
if (cur_coeff < s->subframe_len) {
memset(&ci->coeffs[cur_coeff], 0,
sizeof(*ci->coeffs) * (s->subframe_len - cur_coeff));
if (ff_wma_run_level_decode(s->avctx, &s->gb, vlc,
level, run, 1, ci->coeffs,
cur_coeff, s->subframe_len,
s->subframe_len, s->esc_len, 0))
return AVERROR_INVALIDDATA;
}
return 0;
}
static int decode_scale_factors(WMAProDecodeCtx* s)
{
int i;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
int* sf;
int* sf_end;
s->channel[c].scale_factors = s->channel[c].saved_scale_factors[!s->channel[c].scale_factor_idx];
sf_end = s->channel[c].scale_factors + s->num_bands;
if (s->channel[c].reuse_sf) {
const int8_t* sf_offsets = s->sf_offsets[s->table_idx][s->channel[c].table_idx];
int b;
for (b = 0; b < s->num_bands; b++)
s->channel[c].scale_factors[b] =
s->channel[c].saved_scale_factors[s->channel[c].scale_factor_idx][*sf_offsets++];
}
if (!s->channel[c].cur_subframe || get_bits1(&s->gb)) {
if (!s->channel[c].reuse_sf) {
int val;
s->channel[c].scale_factor_step = get_bits(&s->gb, 2) + 1;
val = 45 / s->channel[c].scale_factor_step;
for (sf = s->channel[c].scale_factors; sf < sf_end; sf++) {
val += get_vlc2(&s->gb, sf_vlc.table, SCALEVLCBITS, SCALEMAXDEPTH) - 60;
*sf = val;
}
} else {
int i;
for (i = 0; i < s->num_bands; i++) {
int idx;
int skip;
int val;
int sign;
idx = get_vlc2(&s->gb, sf_rl_vlc.table, VLCBITS, SCALERLMAXDEPTH);
if (!idx) {
uint32_t code = get_bits(&s->gb, 14);
val = code >> 6;
sign = (code & 1) - 1;
skip = (code & 0x3f) >> 1;
} else if (idx == 1) {
break;
} else {
skip = scale_rl_run[idx];
val = scale_rl_level[idx];
sign = get_bits1(&s->gb)-1;
}
i += skip;
if (i >= s->num_bands) {
av_log(s->avctx, AV_LOG_ERROR,
"invalid scale factor coding\n");
return AVERROR_INVALIDDATA;
}
s->channel[c].scale_factors[i] += (val ^ sign) - sign;
}
}
s->channel[c].scale_factor_idx = !s->channel[c].scale_factor_idx;
s->channel[c].table_idx = s->table_idx;
s->channel[c].reuse_sf = 1;
}
s->channel[c].max_scale_factor = s->channel[c].scale_factors[0];
for (sf = s->channel[c].scale_factors + 1; sf < sf_end; sf++) {
s->channel[c].max_scale_factor =
FFMAX(s->channel[c].max_scale_factor, *sf);
}
}
return 0;
}
static void inverse_channel_transform(WMAProDecodeCtx *s)
{
int i;
for (i = 0; i < s->num_chgroups; i++) {
if (s->chgroup[i].transform) {
float data[WMAPRO_MAX_CHANNELS];
const int num_channels = s->chgroup[i].num_channels;
float** ch_data = s->chgroup[i].channel_data;
float** ch_end = ch_data + num_channels;
const int8_t* tb = s->chgroup[i].transform_band;
int16_t* sfb;
for (sfb = s->cur_sfb_offsets;
sfb < s->cur_sfb_offsets + s->num_bands; sfb++) {
int y;
if (*tb++ == 1) {
for (y = sfb[0]; y < FFMIN(sfb[1], s->subframe_len); y++) {
const float* mat = s->chgroup[i].decorrelation_matrix;
const float* data_end = data + num_channels;
float* data_ptr = data;
float** ch;
for (ch = ch_data; ch < ch_end; ch++)
*data_ptr++ = (*ch)[y];
for (ch = ch_data; ch < ch_end; ch++) {
float sum = 0;
data_ptr = data;
while (data_ptr < data_end)
sum += *data_ptr++ * *mat++;
(*ch)[y] = sum;
}
}
} else if (s->nb_channels == 2) {
int len = FFMIN(sfb[1], s->subframe_len) - sfb[0];
s->fdsp->vector_fmul_scalar(ch_data[0] + sfb[0],
ch_data[0] + sfb[0],
181.0 / 128, len);
s->fdsp->vector_fmul_scalar(ch_data[1] + sfb[0],
ch_data[1] + sfb[0],
181.0 / 128, len);
}
}
}
}
}
static void wmapro_window(WMAProDecodeCtx *s)
{
int i;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
const float* window;
int winlen = s->channel[c].prev_block_len;
float* start = s->channel[c].coeffs - (winlen >> 1);
if (s->subframe_len < winlen) {
start += (winlen - s->subframe_len) >> 1;
winlen = s->subframe_len;
}
window = s->windows[av_log2(winlen) - WMAPRO_BLOCK_MIN_BITS];
winlen >>= 1;
s->fdsp->vector_fmul_window(start, start, start + winlen,
window, winlen);
s->channel[c].prev_block_len = s->subframe_len;
}
}
static int decode_subframe(WMAProDecodeCtx *s)
{
int offset = s->samples_per_frame;
int subframe_len = s->samples_per_frame;
int i;
int total_samples = s->samples_per_frame * s->nb_channels;
int transmit_coeffs = 0;
int cur_subwoofer_cutoff;
s->subframe_offset = get_bits_count(&s->gb);
for (i = 0; i < s->nb_channels; i++) {
s->channel[i].grouped = 0;
if (offset > s->channel[i].decoded_samples) {
offset = s->channel[i].decoded_samples;
subframe_len =
s->channel[i].subframe_len[s->channel[i].cur_subframe];
}
}
ff_dlog(s->avctx,
"processing subframe with offset %i len %i\n", offset, subframe_len);
s->channels_for_cur_subframe = 0;
for (i = 0; i < s->nb_channels; i++) {
const int cur_subframe = s->channel[i].cur_subframe;
total_samples -= s->channel[i].decoded_samples;
if (offset == s->channel[i].decoded_samples &&
subframe_len == s->channel[i].subframe_len[cur_subframe]) {
total_samples -= s->channel[i].subframe_len[cur_subframe];
s->channel[i].decoded_samples +=
s->channel[i].subframe_len[cur_subframe];
s->channel_indexes_for_cur_subframe[s->channels_for_cur_subframe] = i;
++s->channels_for_cur_subframe;
}
}
if (!total_samples)
s->parsed_all_subframes = 1;
ff_dlog(s->avctx, "subframe is part of %i channels\n",
s->channels_for_cur_subframe);
s->table_idx = av_log2(s->samples_per_frame/subframe_len);
s->num_bands = s->num_sfb[s->table_idx];
s->cur_sfb_offsets = s->sfb_offsets[s->table_idx];
cur_subwoofer_cutoff = s->subwoofer_cutoffs[s->table_idx];
offset += s->samples_per_frame >> 1;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
s->channel[c].coeffs = &s->channel[c].out[offset];
}
s->subframe_len = subframe_len;
s->esc_len = av_log2(s->subframe_len - 1) + 1;
if (get_bits1(&s->gb)) {
int num_fill_bits;
if (!(num_fill_bits = get_bits(&s->gb, 2))) {
int len = get_bits(&s->gb, 4);
num_fill_bits = get_bitsz(&s->gb, len) + 1;
}
if (num_fill_bits >= 0) {
if (get_bits_count(&s->gb) + num_fill_bits > s->num_saved_bits) {
av_log(s->avctx, AV_LOG_ERROR, "invalid number of fill bits\n");
return AVERROR_INVALIDDATA;
}
skip_bits_long(&s->gb, num_fill_bits);
}
}
if (get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx, "Reserved bit");
return AVERROR_PATCHWELCOME;
}
if (decode_channel_transform(s) < 0)
return AVERROR_INVALIDDATA;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
if ((s->channel[c].transmit_coefs = get_bits1(&s->gb)))
transmit_coeffs = 1;
}
av_assert0(s->subframe_len <= WMAPRO_BLOCK_MAX_SIZE);
if (transmit_coeffs) {
int step;
int quant_step = 90 * s->bits_per_sample >> 4;
if ((s->transmit_num_vec_coeffs = get_bits1(&s->gb))) {
int num_bits = av_log2((s->subframe_len + 3)/4) + 1;
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
int num_vec_coeffs = get_bits(&s->gb, num_bits) << 2;
if (num_vec_coeffs > s->subframe_len) {
av_log(s->avctx, AV_LOG_ERROR, "num_vec_coeffs %d is too large\n", num_vec_coeffs);
return AVERROR_INVALIDDATA;
}
av_assert0(num_vec_coeffs + offset <= FF_ARRAY_ELEMS(s->channel[c].out));
s->channel[c].num_vec_coeffs = num_vec_coeffs;
}
} else {
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
s->channel[c].num_vec_coeffs = s->subframe_len;
}
}
step = get_sbits(&s->gb, 6);
quant_step += step;
if (step == -32 || step == 31) {
const int sign = (step == 31) - 1;
int quant = 0;
while (get_bits_count(&s->gb) + 5 < s->num_saved_bits &&
(step = get_bits(&s->gb, 5)) == 31) {
quant += 31;
}
quant_step += ((quant + step) ^ sign) - sign;
}
if (quant_step < 0) {
av_log(s->avctx, AV_LOG_DEBUG, "negative quant step\n");
}
if (s->channels_for_cur_subframe == 1) {
s->channel[s->channel_indexes_for_cur_subframe[0]].quant_step = quant_step;
} else {
int modifier_len = get_bits(&s->gb, 3);
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
s->channel[c].quant_step = quant_step;
if (get_bits1(&s->gb)) {
if (modifier_len) {
s->channel[c].quant_step += get_bits(&s->gb, modifier_len) + 1;
} else
++s->channel[c].quant_step;
}
}
}
if (decode_scale_factors(s) < 0)
return AVERROR_INVALIDDATA;
}
ff_dlog(s->avctx, "BITSTREAM: subframe header length was %i\n",
get_bits_count(&s->gb) - s->subframe_offset);
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
if (s->channel[c].transmit_coefs &&
get_bits_count(&s->gb) < s->num_saved_bits) {
decode_coeffs(s, c);
} else
memset(s->channel[c].coeffs, 0,
sizeof(*s->channel[c].coeffs) * subframe_len);
}
ff_dlog(s->avctx, "BITSTREAM: subframe length was %i\n",
get_bits_count(&s->gb) - s->subframe_offset);
if (transmit_coeffs) {
FFTContext *mdct = &s->mdct_ctx[av_log2(subframe_len) - WMAPRO_BLOCK_MIN_BITS];
inverse_channel_transform(s);
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
const int* sf = s->channel[c].scale_factors;
int b;
if (c == s->lfe_channel)
memset(&s->tmp[cur_subwoofer_cutoff], 0, sizeof(*s->tmp) *
(subframe_len - cur_subwoofer_cutoff));
for (b = 0; b < s->num_bands; b++) {
const int end = FFMIN(s->cur_sfb_offsets[b+1], s->subframe_len);
const int exp = s->channel[c].quant_step -
(s->channel[c].max_scale_factor - *sf++) *
s->channel[c].scale_factor_step;
const float quant = ff_exp10(exp / 20.0);
int start = s->cur_sfb_offsets[b];
s->fdsp->vector_fmul_scalar(s->tmp + start,
s->channel[c].coeffs + start,
quant, end - start);
}
mdct->imdct_half(mdct, s->channel[c].coeffs, s->tmp);
}
}
wmapro_window(s);
for (i = 0; i < s->channels_for_cur_subframe; i++) {
int c = s->channel_indexes_for_cur_subframe[i];
if (s->channel[c].cur_subframe >= s->channel[c].num_subframes) {
av_log(s->avctx, AV_LOG_ERROR, "broken subframe\n");
return AVERROR_INVALIDDATA;
}
++s->channel[c].cur_subframe;
}
return 0;
}
static int decode_frame(WMAProDecodeCtx *s, AVFrame *frame, int *got_frame_ptr)
{
GetBitContext* gb = &s->gb;
int more_frames = 0;
int len = 0;
int i;
if (s->len_prefix)
len = get_bits(gb, s->log2_frame_size);
ff_dlog(s->avctx, "decoding frame with length %x\n", len);
if (decode_tilehdr(s)) {
s->packet_loss = 1;
return 0;
}
if (s->nb_channels > 1 && get_bits1(gb)) {
if (get_bits1(gb)) {
for (i = 0; i < s->nb_channels * s->nb_channels; i++)
skip_bits(gb, 4);
}
}
if (s->dynamic_range_compression) {
s->drc_gain = get_bits(gb, 8);
ff_dlog(s->avctx, "drc_gain %i\n", s->drc_gain);
}
if (get_bits1(gb)) {
int av_unused skip;
if (get_bits1(gb)) {
skip = get_bits(gb, av_log2(s->samples_per_frame * 2));
ff_dlog(s->avctx, "start skip: %i\n", skip);
}
if (get_bits1(gb)) {
skip = get_bits(gb, av_log2(s->samples_per_frame * 2));
ff_dlog(s->avctx, "end skip: %i\n", skip);
}
}
ff_dlog(s->avctx, "BITSTREAM: frame header length was %i\n",
get_bits_count(gb) - s->frame_offset);
s->parsed_all_subframes = 0;
for (i = 0; i < s->nb_channels; i++) {
s->channel[i].decoded_samples = 0;
s->channel[i].cur_subframe = 0;
s->channel[i].reuse_sf = 0;
}
while (!s->parsed_all_subframes) {
if (decode_subframe(s) < 0) {
s->packet_loss = 1;
return 0;
}
}
for (i = 0; i < s->nb_channels; i++)
memcpy(frame->extended_data[i], s->channel[i].out,
s->samples_per_frame * sizeof(*s->channel[i].out));
for (i = 0; i < s->nb_channels; i++) {
memcpy(&s->channel[i].out[0],
&s->channel[i].out[s->samples_per_frame],
s->samples_per_frame * sizeof(*s->channel[i].out) >> 1);
}
if (s->skip_frame) {
s->skip_frame = 0;
*got_frame_ptr = 0;
av_frame_unref(frame);
} else {
*got_frame_ptr = 1;
}
if (s->len_prefix) {
if (len != (get_bits_count(gb) - s->frame_offset) + 2) {
av_log(s->avctx, AV_LOG_ERROR,
"frame[%"PRIu32"] would have to skip %i bits\n",
s->frame_num,
len - (get_bits_count(gb) - s->frame_offset) - 1);
s->packet_loss = 1;
return 0;
}
skip_bits_long(gb, len - (get_bits_count(gb) - s->frame_offset) - 1);
} else {
while (get_bits_count(gb) < s->num_saved_bits && get_bits1(gb) == 0) {
}
}
more_frames = get_bits1(gb);
++s->frame_num;
return more_frames;
}
static int remaining_bits(WMAProDecodeCtx *s, GetBitContext *gb)
{
return s->buf_bit_size - get_bits_count(gb);
}
static void save_bits(WMAProDecodeCtx *s, GetBitContext* gb, int len,
int append)
{
int buflen;
if (!append) {
s->frame_offset = get_bits_count(gb) & 7;
s->num_saved_bits = s->frame_offset;
init_put_bits(&s->pb, s->frame_data, MAX_FRAMESIZE);
buflen = (s->num_saved_bits + len + 7) >> 3;
} else
buflen = (put_bits_count(&s->pb) + len + 7) >> 3;
if (len <= 0 || buflen > MAX_FRAMESIZE) {
avpriv_request_sample(s->avctx, "Too small input buffer");
s->packet_loss = 1;
return;
}
av_assert0(len <= put_bits_left(&s->pb));
s->num_saved_bits += len;
if (!append) {
avpriv_copy_bits(&s->pb, gb->buffer + (get_bits_count(gb) >> 3),
s->num_saved_bits);
} else {
int align = 8 - (get_bits_count(gb) & 7);
align = FFMIN(align, len);
put_bits(&s->pb, align, get_bits(gb, align));
len -= align;
avpriv_copy_bits(&s->pb, gb->buffer + (get_bits_count(gb) >> 3), len);
}
skip_bits_long(gb, len);
{
PutBitContext tmp = s->pb;
flush_put_bits(&tmp);
}
init_get_bits(&s->gb, s->frame_data, s->num_saved_bits);
skip_bits(&s->gb, s->frame_offset);
}
static int decode_packet(AVCodecContext *avctx, WMAProDecodeCtx *s,
void *data, int *got_frame_ptr, AVPacket *avpkt)
{
GetBitContext* gb = &s->pgb;
const uint8_t* buf = avpkt->data;
int buf_size = avpkt->size;
int num_bits_prev_frame;
int packet_sequence_number;
*got_frame_ptr = 0;
if (!buf_size) {
AVFrame *frame = data;
int i;
s->packet_done = 0;
if (s->eof_done)
return 0;
for (i = 0; i < s->nb_channels; i++) {
memset(frame->extended_data[i], 0,
s->samples_per_frame * sizeof(*s->channel[i].out));
memcpy(frame->extended_data[i], s->channel[i].out,
s->samples_per_frame * sizeof(*s->channel[i].out) >> 1);
}
s->eof_done = 1;
s->packet_done = 1;
*got_frame_ptr = 1;
return 0;
}
else if (s->packet_done || s->packet_loss) {
s->packet_done = 0;
if (avctx->codec_id == AV_CODEC_ID_WMAPRO && buf_size < avctx->block_align) {
av_log(avctx, AV_LOG_ERROR, "Input packet too small (%d < %d)\n",
buf_size, avctx->block_align);
s->packet_loss = 1;
return AVERROR_INVALIDDATA;
}
if (avctx->codec_id == AV_CODEC_ID_WMAPRO) {
s->next_packet_start = buf_size - avctx->block_align;
buf_size = avctx->block_align;
} else {
s->next_packet_start = buf_size - FFMIN(buf_size, avctx->block_align);
buf_size = FFMIN(buf_size, avctx->block_align);
}
s->buf_bit_size = buf_size << 3;
init_get_bits(gb, buf, s->buf_bit_size);
if (avctx->codec_id != AV_CODEC_ID_XMA2) {
packet_sequence_number = get_bits(gb, 4);
skip_bits(gb, 2);
} else {
int num_frames = get_bits(gb, 6);
ff_dlog(avctx, "packet[%d]: number of frames %d\n", avctx->frame_number, num_frames);
packet_sequence_number = 0;
}
num_bits_prev_frame = get_bits(gb, s->log2_frame_size);
if (avctx->codec_id != AV_CODEC_ID_WMAPRO) {
skip_bits(gb, 3);
s->skip_packets = get_bits(gb, 8);
ff_dlog(avctx, "packet[%d]: skip packets %d\n", avctx->frame_number, s->skip_packets);
}
ff_dlog(avctx, "packet[%d]: nbpf %x\n", avctx->frame_number,
num_bits_prev_frame);
if (avctx->codec_id == AV_CODEC_ID_WMAPRO && !s->packet_loss &&
((s->packet_sequence_number + 1) & 0xF) != packet_sequence_number) {
s->packet_loss = 1;
av_log(avctx, AV_LOG_ERROR,
"Packet loss detected! seq %"PRIx8" vs %x\n",
s->packet_sequence_number, packet_sequence_number);
}
s->packet_sequence_number = packet_sequence_number;
if (num_bits_prev_frame > 0) {
int remaining_packet_bits = s->buf_bit_size - get_bits_count(gb);
if (num_bits_prev_frame >= remaining_packet_bits) {
num_bits_prev_frame = remaining_packet_bits;
s->packet_done = 1;
}
save_bits(s, gb, num_bits_prev_frame, 1);
ff_dlog(avctx, "accumulated %x bits of frame data\n",
s->num_saved_bits - s->frame_offset);
if (!s->packet_loss)
decode_frame(s, data, got_frame_ptr);
} else if (s->num_saved_bits - s->frame_offset) {
ff_dlog(avctx, "ignoring %x previously saved bits\n",
s->num_saved_bits - s->frame_offset);
}
if (s->packet_loss) {
s->num_saved_bits = 0;
s->packet_loss = 0;
}
} else {
int frame_size;
s->buf_bit_size = (avpkt->size - s->next_packet_start) << 3;
init_get_bits(gb, avpkt->data, s->buf_bit_size);
skip_bits(gb, s->packet_offset);
if (s->len_prefix && remaining_bits(s, gb) > s->log2_frame_size &&
(frame_size = show_bits(gb, s->log2_frame_size)) &&
frame_size <= remaining_bits(s, gb)) {
save_bits(s, gb, frame_size, 0);
if (!s->packet_loss)
s->packet_done = !decode_frame(s, data, got_frame_ptr);
} else if (!s->len_prefix
&& s->num_saved_bits > get_bits_count(&s->gb)) {
s->packet_done = !decode_frame(s, data, got_frame_ptr);
} else {
s->packet_done = 1;
}
}
if (remaining_bits(s, gb) < 0) {
av_log(avctx, AV_LOG_ERROR, "Overread %d\n", -remaining_bits(s, gb));
s->packet_loss = 1;
}
if (s->packet_done && !s->packet_loss &&
remaining_bits(s, gb) > 0) {
save_bits(s, gb, remaining_bits(s, gb), 0);
}
s->packet_offset = get_bits_count(gb) & 7;
if (s->packet_loss)
return AVERROR_INVALIDDATA;
return get_bits_count(gb) >> 3;
}
static int wmapro_decode_packet(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
WMAProDecodeCtx *s = avctx->priv_data;
AVFrame *frame = data;
int ret;
frame->nb_samples = s->samples_per_frame;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0) {
s->packet_loss = 1;
return 0;
}
return decode_packet(avctx, s, data, got_frame_ptr, avpkt);
}
static int xma_decode_packet(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
XMADecodeCtx *s = avctx->priv_data;
int got_stream_frame_ptr = 0;
AVFrame *frame = data;
int i, ret, offset = INT_MAX;
if (!s->frames[s->current_stream]->data[0]) {
s->frames[s->current_stream]->nb_samples = 512;
if ((ret = ff_get_buffer(avctx, s->frames[s->current_stream], 0)) < 0) {
return ret;
}
}
ret = decode_packet(avctx, &s->xma[s->current_stream], s->frames[s->current_stream],
&got_stream_frame_ptr, avpkt);
if (got_stream_frame_ptr && s->offset[s->current_stream] >= 64) {
got_stream_frame_ptr = 0;
ret = AVERROR_INVALIDDATA;
}
if (got_stream_frame_ptr) {
int start_ch = s->start_channel[s->current_stream];
memcpy(&s->samples[start_ch + 0][s->offset[s->current_stream] * 512],
s->frames[s->current_stream]->extended_data[0], 512 * 4);
if (s->xma[s->current_stream].nb_channels > 1)
memcpy(&s->samples[start_ch + 1][s->offset[s->current_stream] * 512],
s->frames[s->current_stream]->extended_data[1], 512 * 4);
s->offset[s->current_stream]++;
} else if (ret < 0) {
memset(s->offset, 0, sizeof(s->offset));
s->current_stream = 0;
return ret;
}
if (s->xma[s->current_stream].packet_done ||
s->xma[s->current_stream].packet_loss) {
if (s->xma[s->current_stream].skip_packets != 0) {
int min[2];
min[0] = s->xma[0].skip_packets;
min[1] = i = 0;
for (i = 1; i < s->num_streams; i++) {
if (s->xma[i].skip_packets < min[0]) {
min[0] = s->xma[i].skip_packets;
min[1] = i;
}
}
s->current_stream = min[1];
}
for (i = 0; i < s->num_streams; i++) {
s->xma[i].skip_packets = FFMAX(0, s->xma[i].skip_packets - 1);
}
for (i = 0; i < s->num_streams; i++) {
offset = FFMIN(offset, s->offset[i]);
}
if (offset > 0) {
int bret;
frame->nb_samples = 512 * offset;
if ((bret = ff_get_buffer(avctx, frame, 0)) < 0)
return bret;
for (i = 0; i < s->num_streams; i++) {
int start_ch = s->start_channel[i];
memcpy(frame->extended_data[start_ch + 0], s->samples[start_ch + 0], frame->nb_samples * 4);
if (s->xma[i].nb_channels > 1)
memcpy(frame->extended_data[start_ch + 1], s->samples[start_ch + 1], frame->nb_samples * 4);
s->offset[i] -= offset;
if (s->offset[i]) {
memmove(s->samples[start_ch + 0], s->samples[start_ch + 0] + frame->nb_samples, s->offset[i] * 4 * 512);
if (s->xma[i].nb_channels > 1)
memmove(s->samples[start_ch + 1], s->samples[start_ch + 1] + frame->nb_samples, s->offset[i] * 4 * 512);
}
}
*got_frame_ptr = 1;
}
}
return ret;
}
static av_cold int xma_decode_init(AVCodecContext *avctx)
{
XMADecodeCtx *s = avctx->priv_data;
int i, ret, start_channels = 0;
if (avctx->channels <= 0 || avctx->extradata_size == 0)
return AVERROR_INVALIDDATA;
if (avctx->codec_id == AV_CODEC_ID_XMA2 && avctx->extradata_size == 34) { 
s->num_streams = (avctx->channels + 1) / 2;
} else if (avctx->codec_id == AV_CODEC_ID_XMA2 && avctx->extradata_size >= 2) { 
s->num_streams = avctx->extradata[1];
if (avctx->extradata_size != (32 + ((avctx->extradata[0]==3)?0:8) + 4*s->num_streams)) {
av_log(avctx, AV_LOG_ERROR, "Incorrect XMA2 extradata size\n");
s->num_streams = 0;
return AVERROR(EINVAL);
}
} else if (avctx->codec_id == AV_CODEC_ID_XMA1 && avctx->extradata_size >= 4) { 
s->num_streams = avctx->extradata[4];
if (avctx->extradata_size != (8 + 20*s->num_streams)) {
av_log(avctx, AV_LOG_ERROR, "Incorrect XMA1 extradata size\n");
s->num_streams = 0;
return AVERROR(EINVAL);
}
} else {
av_log(avctx, AV_LOG_ERROR, "Incorrect XMA config\n");
return AVERROR(EINVAL);
}
if (avctx->channels > XMA_MAX_CHANNELS || s->num_streams > XMA_MAX_STREAMS ||
s->num_streams <= 0
) {
avpriv_request_sample(avctx, "More than %d channels in %d streams", XMA_MAX_CHANNELS, s->num_streams);
s->num_streams = 0;
return AVERROR_PATCHWELCOME;
}
for (i = 0; i < s->num_streams; i++) {
ret = decode_init(&s->xma[i], avctx, i);
if (ret < 0)
return ret;
s->frames[i] = av_frame_alloc();
if (!s->frames[i])
return AVERROR(ENOMEM);
s->start_channel[i] = start_channels;
start_channels += s->xma[i].nb_channels;
}
if (start_channels != avctx->channels)
return AVERROR_INVALIDDATA;
return ret;
}
static av_cold int xma_decode_end(AVCodecContext *avctx)
{
XMADecodeCtx *s = avctx->priv_data;
int i;
for (i = 0; i < s->num_streams; i++) {
decode_end(&s->xma[i]);
av_frame_free(&s->frames[i]);
}
s->num_streams = 0;
return 0;
}
static void flush(WMAProDecodeCtx *s)
{
int i;
for (i = 0; i < s->nb_channels; i++)
memset(s->channel[i].out, 0, s->samples_per_frame *
sizeof(*s->channel[i].out));
s->packet_loss = 1;
s->skip_packets = 0;
s->eof_done = 0;
}
static void wmapro_flush(AVCodecContext *avctx)
{
WMAProDecodeCtx *s = avctx->priv_data;
flush(s);
}
static void xma_flush(AVCodecContext *avctx)
{
XMADecodeCtx *s = avctx->priv_data;
int i;
for (i = 0; i < s->num_streams; i++)
flush(&s->xma[i]);
memset(s->offset, 0, sizeof(s->offset));
s->current_stream = 0;
}
AVCodec ff_wmapro_decoder = {
.name = "wmapro",
.long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio 9 Professional"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_WMAPRO,
.priv_data_size = sizeof(WMAProDecodeCtx),
.init = wmapro_decode_init,
.close = wmapro_decode_end,
.decode = wmapro_decode_packet,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP,
.flush = wmapro_flush,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
};
AVCodec ff_xma1_decoder = {
.name = "xma1",
.long_name = NULL_IF_CONFIG_SMALL("Xbox Media Audio 1"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_XMA1,
.priv_data_size = sizeof(XMADecodeCtx),
.init = xma_decode_init,
.close = xma_decode_end,
.decode = xma_decode_packet,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
};
AVCodec ff_xma2_decoder = {
.name = "xma2",
.long_name = NULL_IF_CONFIG_SMALL("Xbox Media Audio 2"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_XMA2,
.priv_data_size = sizeof(XMADecodeCtx),
.init = xma_decode_init,
.close = xma_decode_end,
.decode = xma_decode_packet,
.flush = xma_flush,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
};
