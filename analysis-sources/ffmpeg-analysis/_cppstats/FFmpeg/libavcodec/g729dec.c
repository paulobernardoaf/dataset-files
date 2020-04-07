#include <inttypes.h>
#include <string.h>
#include "avcodec.h"
#include "libavutil/avutil.h"
#include "get_bits.h"
#include "audiodsp.h"
#include "internal.h"
#include "g729.h"
#include "lsp.h"
#include "celp_math.h"
#include "celp_filters.h"
#include "acelp_filters.h"
#include "acelp_pitch_delay.h"
#include "acelp_vectors.h"
#include "g729data.h"
#include "g729postfilter.h"
#define LSFQ_MIN 40
#define LSFQ_MAX 25681
#define LSFQ_DIFF_MIN 321
#define INTERPOL_LEN 11
#define SHARP_MIN 3277
#define SHARP_MAX 13017
#define MR_ENERGY 1018156
#define DECISION_NOISE 0
#define DECISION_INTERMEDIATE 1
#define DECISION_VOICE 2
typedef enum {
FORMAT_G729_8K = 0,
FORMAT_G729D_6K4,
FORMAT_COUNT,
} G729Formats;
typedef struct {
uint8_t ac_index_bits[2]; 
uint8_t parity_bit; 
uint8_t gc_1st_index_bits; 
uint8_t gc_2nd_index_bits; 
uint8_t fc_signs_bits; 
uint8_t fc_indexes_bits; 
uint8_t block_size;
} G729FormatDescription;
typedef struct {
int16_t exc_base[2*SUBFRAME_SIZE+PITCH_DELAY_MAX+INTERPOL_LEN];
int16_t* exc; 
int pitch_delay_int_prev; 
int16_t past_quantizer_output_buf[MA_NP + 1][10];
int16_t* past_quantizer_outputs[MA_NP + 1];
int16_t lsfq[10]; 
int16_t lsp_buf[2][10]; 
int16_t *lsp[2]; 
int16_t quant_energy[4]; 
int16_t syn_filter_data[10];
int16_t residual[SUBFRAME_SIZE + RES_PREV_DATA_SIZE];
int16_t res_filter_data[SUBFRAME_SIZE+10];
int16_t pos_filter_data[SUBFRAME_SIZE+10];
int16_t past_gain_pitch[6];
int16_t past_gain_code[2];
int16_t voice_decision;
int16_t onset; 
int16_t was_periodic; 
int16_t ht_prev_data; 
int gain_coeff; 
uint16_t rand_value; 
int ma_predictor_prev; 
int hpf_f[2];
int16_t hpf_z[2];
} G729ChannelContext;
typedef struct {
AudioDSPContext adsp;
G729ChannelContext *channel_context;
} G729Context;
static const G729FormatDescription format_g729_8k = {
.ac_index_bits = {8,5},
.parity_bit = 1,
.gc_1st_index_bits = GC_1ST_IDX_BITS_8K,
.gc_2nd_index_bits = GC_2ND_IDX_BITS_8K,
.fc_signs_bits = 4,
.fc_indexes_bits = 13,
.block_size = G729_8K_BLOCK_SIZE,
};
static const G729FormatDescription format_g729d_6k4 = {
.ac_index_bits = {8,4},
.parity_bit = 0,
.gc_1st_index_bits = GC_1ST_IDX_BITS_6K4,
.gc_2nd_index_bits = GC_2ND_IDX_BITS_6K4,
.fc_signs_bits = 2,
.fc_indexes_bits = 9,
.block_size = G729D_6K4_BLOCK_SIZE,
};
static inline uint16_t g729_prng(uint16_t value)
{
return 31821 * value + 13849;
}
static void lsf_decode(int16_t* lsfq, int16_t* past_quantizer_outputs[MA_NP + 1],
int16_t ma_predictor,
int16_t vq_1st, int16_t vq_2nd_low, int16_t vq_2nd_high)
{
int i,j;
static const uint8_t min_distance[2]={10, 5}; 
int16_t* quantizer_output = past_quantizer_outputs[MA_NP];
for (i = 0; i < 5; i++) {
quantizer_output[i] = cb_lsp_1st[vq_1st][i ] + cb_lsp_2nd[vq_2nd_low ][i ];
quantizer_output[i + 5] = cb_lsp_1st[vq_1st][i + 5] + cb_lsp_2nd[vq_2nd_high][i + 5];
}
for (j = 0; j < 2; j++) {
for (i = 1; i < 10; i++) {
int diff = (quantizer_output[i - 1] - quantizer_output[i] + min_distance[j]) >> 1;
if (diff > 0) {
quantizer_output[i - 1] -= diff;
quantizer_output[i ] += diff;
}
}
}
for (i = 0; i < 10; i++) {
int sum = quantizer_output[i] * cb_ma_predictor_sum[ma_predictor][i];
for (j = 0; j < MA_NP; j++)
sum += past_quantizer_outputs[j][i] * cb_ma_predictor[ma_predictor][j][i];
lsfq[i] = sum >> 15;
}
ff_acelp_reorder_lsf(lsfq, LSFQ_DIFF_MIN, LSFQ_MIN, LSFQ_MAX, 10);
}
static void lsf_restore_from_previous(int16_t* lsfq,
int16_t* past_quantizer_outputs[MA_NP + 1],
int ma_predictor_prev)
{
int16_t* quantizer_output = past_quantizer_outputs[MA_NP];
int i,k;
for (i = 0; i < 10; i++) {
int tmp = lsfq[i] << 15;
for (k = 0; k < MA_NP; k++)
tmp -= past_quantizer_outputs[k][i] * cb_ma_predictor[ma_predictor_prev][k][i];
quantizer_output[i] = ((tmp >> 15) * cb_ma_predictor_sum_inv[ma_predictor_prev][i]) >> 12;
}
}
static void g729d_get_new_exc(
int16_t* out,
const int16_t* in,
const int16_t* fc_cur,
int dstate,
int gain_code,
int subframe_size)
{
int i;
int16_t fc_new[SUBFRAME_SIZE];
ff_celp_convolve_circ(fc_new, fc_cur, phase_filter[dstate], subframe_size);
for (i = 0; i < subframe_size; i++) {
out[i] = in[i];
out[i] -= (gain_code * fc_cur[i] + 0x2000) >> 14;
out[i] += (gain_code * fc_new[i] + 0x2000) >> 14;
}
}
static int g729d_onset_decision(int past_onset, const int16_t* past_gain_code)
{
if ((past_gain_code[0] >> 1) > past_gain_code[1])
return 2;
return FFMAX(past_onset-1, 0);
}
static int16_t g729d_voice_decision(int onset, int prev_voice_decision, const int16_t* past_gain_pitch)
{
int i, low_gain_pitch_cnt, voice_decision;
if (past_gain_pitch[0] >= 14745) { 
voice_decision = DECISION_VOICE;
} else if (past_gain_pitch[0] <= 9830) { 
voice_decision = DECISION_NOISE;
} else {
voice_decision = DECISION_INTERMEDIATE;
}
for (i = 0, low_gain_pitch_cnt = 0; i < 6; i++)
if (past_gain_pitch[i] < 9830)
low_gain_pitch_cnt++;
if (low_gain_pitch_cnt > 2 && !onset)
voice_decision = DECISION_NOISE;
if (!onset && voice_decision > prev_voice_decision + 1)
voice_decision--;
if (onset && voice_decision < DECISION_VOICE)
voice_decision++;
return voice_decision;
}
static int32_t scalarproduct_int16_c(const int16_t * v1, const int16_t * v2, int order)
{
int64_t res = 0;
while (order--)
res += *v1++ * *v2++;
if (res > INT32_MAX) return INT32_MAX;
else if (res < INT32_MIN) return INT32_MIN;
return res;
}
static av_cold int decoder_init(AVCodecContext * avctx)
{
G729Context *s = avctx->priv_data;
G729ChannelContext *ctx;
int c,i,k;
if (avctx->channels < 1 || avctx->channels > 2) {
av_log(avctx, AV_LOG_ERROR, "Only mono and stereo are supported (requested channels: %d).\n", avctx->channels);
return AVERROR(EINVAL);
}
avctx->sample_fmt = AV_SAMPLE_FMT_S16P;
avctx->frame_size = SUBFRAME_SIZE << 1;
ctx =
s->channel_context = av_mallocz(sizeof(G729ChannelContext) * avctx->channels);
if (!ctx)
return AVERROR(ENOMEM);
for (c = 0; c < avctx->channels; c++) {
ctx->gain_coeff = 16384; 
for (k = 0; k < MA_NP + 1; k++) {
ctx->past_quantizer_outputs[k] = ctx->past_quantizer_output_buf[k];
for (i = 1; i < 11; i++)
ctx->past_quantizer_outputs[k][i - 1] = (18717 * i) >> 3;
}
ctx->lsp[0] = ctx->lsp_buf[0];
ctx->lsp[1] = ctx->lsp_buf[1];
memcpy(ctx->lsp[0], lsp_init, 10 * sizeof(int16_t));
ctx->exc = &ctx->exc_base[PITCH_DELAY_MAX+INTERPOL_LEN];
ctx->pitch_delay_int_prev = PITCH_DELAY_MIN;
ctx->rand_value = 21845;
for (i = 0; i < 4; i++)
ctx->quant_energy[i] = -14336; 
ctx++;
}
ff_audiodsp_init(&s->adsp);
s->adsp.scalarproduct_int16 = scalarproduct_int16_c;
return 0;
}
static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame_ptr,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
int16_t *out_frame;
GetBitContext gb;
const G729FormatDescription *format;
int c, i;
int16_t *tmp;
G729Formats packet_type;
G729Context *s = avctx->priv_data;
G729ChannelContext *ctx = s->channel_context;
int16_t lp[2][11]; 
uint8_t ma_predictor; 
uint8_t quantizer_1st; 
uint8_t quantizer_2nd_lo; 
uint8_t quantizer_2nd_hi; 
int pitch_delay_int[2]; 
int pitch_delay_3x; 
int16_t fc[SUBFRAME_SIZE]; 
int16_t synth[SUBFRAME_SIZE+10]; 
int j, ret;
int gain_before, gain_after;
AVFrame *frame = data;
frame->nb_samples = SUBFRAME_SIZE<<1;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
if (buf_size && buf_size % ((G729_8K_BLOCK_SIZE + (avctx->codec_id == AV_CODEC_ID_ACELP_KELVIN)) * avctx->channels) == 0) {
packet_type = FORMAT_G729_8K;
format = &format_g729_8k;
ctx->onset = 0;
ctx->voice_decision = DECISION_VOICE;
av_log(avctx, AV_LOG_DEBUG, "Packet type: %s\n", "G.729 @ 8kbit/s");
} else if (buf_size == G729D_6K4_BLOCK_SIZE * avctx->channels && avctx->codec_id != AV_CODEC_ID_ACELP_KELVIN) {
packet_type = FORMAT_G729D_6K4;
format = &format_g729d_6k4;
av_log(avctx, AV_LOG_DEBUG, "Packet type: %s\n", "G.729D @ 6.4kbit/s");
} else {
av_log(avctx, AV_LOG_ERROR, "Packet size %d is unknown.\n", buf_size);
return AVERROR_INVALIDDATA;
}
for (c = 0; c < avctx->channels; c++) {
int frame_erasure = 0; 
int bad_pitch = 0; 
int is_periodic = 0; 
out_frame = (int16_t*)frame->data[c];
if (avctx->codec_id == AV_CODEC_ID_ACELP_KELVIN) {
if (*buf != ((avctx->channels - 1 - c) * 0x80 | 2))
avpriv_request_sample(avctx, "First byte value %x for channel %d", *buf, c);
buf++;
}
for (i = 0; i < format->block_size; i++)
frame_erasure |= buf[i];
frame_erasure = !frame_erasure;
init_get_bits8(&gb, buf, format->block_size);
ma_predictor = get_bits(&gb, 1);
quantizer_1st = get_bits(&gb, VQ_1ST_BITS);
quantizer_2nd_lo = get_bits(&gb, VQ_2ND_BITS);
quantizer_2nd_hi = get_bits(&gb, VQ_2ND_BITS);
if (frame_erasure) {
lsf_restore_from_previous(ctx->lsfq, ctx->past_quantizer_outputs,
ctx->ma_predictor_prev);
} else {
lsf_decode(ctx->lsfq, ctx->past_quantizer_outputs,
ma_predictor,
quantizer_1st, quantizer_2nd_lo, quantizer_2nd_hi);
ctx->ma_predictor_prev = ma_predictor;
}
tmp = ctx->past_quantizer_outputs[MA_NP];
memmove(ctx->past_quantizer_outputs + 1, ctx->past_quantizer_outputs,
MA_NP * sizeof(int16_t*));
ctx->past_quantizer_outputs[0] = tmp;
ff_acelp_lsf2lsp(ctx->lsp[1], ctx->lsfq, 10);
ff_acelp_lp_decode(&lp[0][0], &lp[1][0], ctx->lsp[1], ctx->lsp[0], 10);
FFSWAP(int16_t*, ctx->lsp[1], ctx->lsp[0]);
for (i = 0; i < 2; i++) {
int gain_corr_factor;
uint8_t ac_index; 
uint8_t pulses_signs; 
int fc_indexes; 
uint8_t gc_1st_index; 
uint8_t gc_2nd_index; 
ac_index = get_bits(&gb, format->ac_index_bits[i]);
if (!i && format->parity_bit)
bad_pitch = av_parity(ac_index >> 2) == get_bits1(&gb);
fc_indexes = get_bits(&gb, format->fc_indexes_bits);
pulses_signs = get_bits(&gb, format->fc_signs_bits);
gc_1st_index = get_bits(&gb, format->gc_1st_index_bits);
gc_2nd_index = get_bits(&gb, format->gc_2nd_index_bits);
if (frame_erasure) {
pitch_delay_3x = 3 * ctx->pitch_delay_int_prev;
} else if (!i) {
if (bad_pitch) {
pitch_delay_3x = 3 * ctx->pitch_delay_int_prev;
} else {
pitch_delay_3x = ff_acelp_decode_8bit_to_1st_delay3(ac_index);
}
} else {
int pitch_delay_min = av_clip(ctx->pitch_delay_int_prev - 5,
PITCH_DELAY_MIN, PITCH_DELAY_MAX - 9);
if (packet_type == FORMAT_G729D_6K4) {
pitch_delay_3x = ff_acelp_decode_4bit_to_2nd_delay3(ac_index, pitch_delay_min);
} else {
pitch_delay_3x = ff_acelp_decode_5_6_bit_to_2nd_delay3(ac_index, pitch_delay_min);
}
}
pitch_delay_int[i] = (pitch_delay_3x + 1) / 3;
if (pitch_delay_int[i] > PITCH_DELAY_MAX) {
av_log(avctx, AV_LOG_WARNING, "pitch_delay_int %d is too large\n", pitch_delay_int[i]);
pitch_delay_int[i] = PITCH_DELAY_MAX;
}
if (frame_erasure) {
ctx->rand_value = g729_prng(ctx->rand_value);
fc_indexes = av_mod_uintp2(ctx->rand_value, format->fc_indexes_bits);
ctx->rand_value = g729_prng(ctx->rand_value);
pulses_signs = ctx->rand_value;
}
memset(fc, 0, sizeof(int16_t) * SUBFRAME_SIZE);
switch (packet_type) {
case FORMAT_G729_8K:
ff_acelp_fc_pulse_per_track(fc, ff_fc_4pulses_8bits_tracks_13,
ff_fc_4pulses_8bits_track_4,
fc_indexes, pulses_signs, 3, 3);
break;
case FORMAT_G729D_6K4:
ff_acelp_fc_pulse_per_track(fc, ff_fc_2pulses_9bits_track1_gray,
ff_fc_2pulses_9bits_track2_gray,
fc_indexes, pulses_signs, 1, 4);
break;
}
if (SUBFRAME_SIZE > pitch_delay_int[i])
ff_acelp_weighted_vector_sum(fc + pitch_delay_int[i],
fc + pitch_delay_int[i],
fc, 1 << 14,
av_clip(ctx->past_gain_pitch[0], SHARP_MIN, SHARP_MAX),
0, 14,
SUBFRAME_SIZE - pitch_delay_int[i]);
memmove(ctx->past_gain_pitch+1, ctx->past_gain_pitch, 5 * sizeof(int16_t));
ctx->past_gain_code[1] = ctx->past_gain_code[0];
if (frame_erasure) {
ctx->past_gain_pitch[0] = (29491 * ctx->past_gain_pitch[0]) >> 15; 
ctx->past_gain_code[0] = ( 2007 * ctx->past_gain_code[0] ) >> 11; 
gain_corr_factor = 0;
} else {
if (packet_type == FORMAT_G729D_6K4) {
ctx->past_gain_pitch[0] = cb_gain_1st_6k4[gc_1st_index][0] +
cb_gain_2nd_6k4[gc_2nd_index][0];
gain_corr_factor = cb_gain_1st_6k4[gc_1st_index][1] +
cb_gain_2nd_6k4[gc_2nd_index][1];
gain_corr_factor = FFMAX(gain_corr_factor, 1024);
#if !defined(G729_BITEXACT)
gain_corr_factor >>= 1;
#endif
} else {
ctx->past_gain_pitch[0] = cb_gain_1st_8k[gc_1st_index][0] +
cb_gain_2nd_8k[gc_2nd_index][0];
gain_corr_factor = cb_gain_1st_8k[gc_1st_index][1] +
cb_gain_2nd_8k[gc_2nd_index][1];
}
ctx->past_gain_code[0] = ff_acelp_decode_gain_code(&s->adsp, gain_corr_factor,
fc, MR_ENERGY,
ctx->quant_energy,
ma_prediction_coeff,
SUBFRAME_SIZE, 4);
#if defined(G729_BITEXACT)
if (packet_type == FORMAT_G729D_6K4) {
gain_corr_factor >>= 1;
ctx->past_gain_code[0] >>= 1;
}
#endif
}
ff_acelp_update_past_gain(ctx->quant_energy, gain_corr_factor, 2, frame_erasure);
ff_acelp_interpolate(ctx->exc + i * SUBFRAME_SIZE,
ctx->exc + i * SUBFRAME_SIZE - pitch_delay_3x / 3,
ff_acelp_interp_filter, 6,
(pitch_delay_3x % 3) << 1,
10, SUBFRAME_SIZE);
ff_acelp_weighted_vector_sum(ctx->exc + i * SUBFRAME_SIZE,
ctx->exc + i * SUBFRAME_SIZE, fc,
(!ctx->was_periodic && frame_erasure) ? 0 : ctx->past_gain_pitch[0],
( ctx->was_periodic && frame_erasure) ? 0 : ctx->past_gain_code[0],
1 << 13, 14, SUBFRAME_SIZE);
memcpy(synth, ctx->syn_filter_data, 10 * sizeof(int16_t));
if (ff_celp_lp_synthesis_filter(
synth+10,
&lp[i][1],
ctx->exc + i * SUBFRAME_SIZE,
SUBFRAME_SIZE,
10,
1,
0,
0x800))
for (j = 0; j < 2 * SUBFRAME_SIZE + PITCH_DELAY_MAX + INTERPOL_LEN; j++)
ctx->exc_base[j] >>= 2;
if (packet_type == FORMAT_G729D_6K4) {
int16_t exc_new[SUBFRAME_SIZE];
ctx->onset = g729d_onset_decision(ctx->onset, ctx->past_gain_code);
ctx->voice_decision = g729d_voice_decision(ctx->onset, ctx->voice_decision, ctx->past_gain_pitch);
g729d_get_new_exc(exc_new, ctx->exc + i * SUBFRAME_SIZE, fc, ctx->voice_decision, ctx->past_gain_code[0], SUBFRAME_SIZE);
ff_celp_lp_synthesis_filter(
synth+10,
&lp[i][1],
exc_new,
SUBFRAME_SIZE,
10,
0,
0,
0x800);
} else {
ff_celp_lp_synthesis_filter(
synth+10,
&lp[i][1],
ctx->exc + i * SUBFRAME_SIZE,
SUBFRAME_SIZE,
10,
0,
0,
0x800);
}
memcpy(ctx->syn_filter_data, synth+SUBFRAME_SIZE, 10 * sizeof(int16_t));
gain_before = 0;
for (j = 0; j < SUBFRAME_SIZE; j++)
gain_before += FFABS(synth[j+10]);
ff_g729_postfilter(
&s->adsp,
&ctx->ht_prev_data,
&is_periodic,
&lp[i][0],
pitch_delay_int[0],
ctx->residual,
ctx->res_filter_data,
ctx->pos_filter_data,
synth+10,
SUBFRAME_SIZE);
gain_after = 0;
for (j = 0; j < SUBFRAME_SIZE; j++)
gain_after += FFABS(synth[j+10]);
ctx->gain_coeff = ff_g729_adaptive_gain_control(
gain_before,
gain_after,
synth+10,
SUBFRAME_SIZE,
ctx->gain_coeff);
if (frame_erasure) {
ctx->pitch_delay_int_prev = FFMIN(ctx->pitch_delay_int_prev + 1, PITCH_DELAY_MAX);
} else {
ctx->pitch_delay_int_prev = pitch_delay_int[i];
}
memcpy(synth+8, ctx->hpf_z, 2*sizeof(int16_t));
ff_acelp_high_pass_filter(
out_frame + i*SUBFRAME_SIZE,
ctx->hpf_f,
synth+10,
SUBFRAME_SIZE);
memcpy(ctx->hpf_z, synth+8+SUBFRAME_SIZE, 2*sizeof(int16_t));
}
ctx->was_periodic = is_periodic;
memmove(ctx->exc_base, ctx->exc_base + 2 * SUBFRAME_SIZE, (PITCH_DELAY_MAX+INTERPOL_LEN)*sizeof(int16_t));
buf += format->block_size;
ctx++;
}
*got_frame_ptr = 1;
return (format->block_size + (avctx->codec_id == AV_CODEC_ID_ACELP_KELVIN)) * avctx->channels;
}
static av_cold int decode_close(AVCodecContext *avctx)
{
G729Context *s = avctx->priv_data;
av_freep(&s->channel_context);
return 0;
}
AVCodec ff_g729_decoder = {
.name = "g729",
.long_name = NULL_IF_CONFIG_SMALL("G.729"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_G729,
.priv_data_size = sizeof(G729Context),
.init = decoder_init,
.decode = decode_frame,
.close = decode_close,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
};
AVCodec ff_acelp_kelvin_decoder = {
.name = "acelp.kelvin",
.long_name = NULL_IF_CONFIG_SMALL("Sipro ACELP.KELVIN"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_ACELP_KELVIN,
.priv_data_size = sizeof(G729Context),
.init = decoder_init,
.decode = decode_frame,
.close = decode_close,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
};
