#include <math.h>
#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/mem.h"
#include "libavutil/thread.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "put_bits.h"
#include "wmavoice_data.h"
#include "celp_filters.h"
#include "acelp_vectors.h"
#include "acelp_filters.h"
#include "lsp.h"
#include "dct.h"
#include "rdft.h"
#include "sinewin.h"
#define MAX_BLOCKS 8 
#define MAX_LSPS 16 
#define MAX_LSPS_ALIGN16 16 
#define MAX_FRAMES 3 
#define MAX_FRAMESIZE 160 
#define MAX_SIGNAL_HISTORY 416 
#define MAX_SFRAMESIZE (MAX_FRAMESIZE * MAX_FRAMES)
#define SFRAME_CACHE_MAXSIZE 256 
#define VLC_NBITS 6 
static VLC frame_type_vlc;
enum {
ACB_TYPE_NONE = 0, 
ACB_TYPE_ASYMMETRIC = 1, 
ACB_TYPE_HAMMING = 2 
};
enum {
FCB_TYPE_SILENCE = 0, 
FCB_TYPE_HARDCODED = 1, 
FCB_TYPE_AW_PULSES = 2, 
FCB_TYPE_EXC_PULSES = 3, 
};
static const struct frame_type_desc {
uint8_t n_blocks; 
uint8_t log_n_blocks; 
uint8_t acb_type; 
uint8_t fcb_type; 
uint8_t dbl_pulses; 
} frame_descs[17] = {
{ 1, 0, ACB_TYPE_NONE, FCB_TYPE_SILENCE, 0 },
{ 2, 1, ACB_TYPE_NONE, FCB_TYPE_HARDCODED, 0 },
{ 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_AW_PULSES, 0 },
{ 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 2 },
{ 2, 1, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 5 },
{ 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 0 },
{ 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 2 },
{ 4, 2, ACB_TYPE_ASYMMETRIC, FCB_TYPE_EXC_PULSES, 5 },
{ 2, 1, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 0 },
{ 2, 1, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 2 },
{ 2, 1, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 5 },
{ 4, 2, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 0 },
{ 4, 2, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 2 },
{ 4, 2, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 5 },
{ 8, 3, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 0 },
{ 8, 3, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 2 },
{ 8, 3, ACB_TYPE_HAMMING, FCB_TYPE_EXC_PULSES, 5 }
};
typedef struct WMAVoiceContext {
GetBitContext gb; 
int8_t vbm_tree[25]; 
int spillover_bitsize; 
int history_nsamples; 
int do_apf; 
int denoise_strength; 
int denoise_tilt_corr; 
int dc_level; 
int lsps; 
int lsp_q_mode; 
int lsp_def_mode; 
int min_pitch_val; 
int max_pitch_val; 
int pitch_nbits; 
int block_pitch_nbits; 
int block_pitch_range; 
int block_delta_pitch_nbits; 
int block_delta_pitch_hrange; 
uint16_t block_conv_table[4]; 
int spillover_nbits; 
int has_residual_lsps; 
int skip_bits_next; 
uint8_t sframe_cache[SFRAME_CACHE_MAXSIZE + AV_INPUT_BUFFER_PADDING_SIZE];
int sframe_cache_size; 
PutBitContext pb; 
double prev_lsps[MAX_LSPS]; 
int last_pitch_val; 
int last_acb_type; 
int pitch_diff_sh16; 
float silence_gain; 
int aw_idx_is_ext; 
int aw_pulse_range; 
int aw_n_pulses[2]; 
int aw_first_pulse_off[2]; 
int aw_next_pulse_off_cache; 
int frame_cntr; 
int nb_superframes; 
float gain_pred_err[6]; 
float excitation_history[MAX_SIGNAL_HISTORY];
float synth_history[MAX_LSPS]; 
RDFTContext rdft, irdft; 
DCTContext dct, dst; 
float sin[511], cos[511]; 
float postfilter_agc; 
float dcf_mem[2]; 
float zero_exc_pf[MAX_SIGNAL_HISTORY + MAX_SFRAMESIZE];
float denoise_filter_cache[MAX_FRAMESIZE];
int denoise_filter_cache_size; 
DECLARE_ALIGNED(32, float, tilted_lpcs_pf)[0x80];
DECLARE_ALIGNED(32, float, denoise_coeffs_pf)[0x80];
DECLARE_ALIGNED(32, float, synth_filter_out_buf)[0x80 + MAX_LSPS_ALIGN16];
} WMAVoiceContext;
static av_cold int decode_vbmtree(GetBitContext *gb, int8_t vbm_tree[25])
{
int cntr[8] = { 0 }, n, res;
memset(vbm_tree, 0xff, sizeof(vbm_tree[0]) * 25);
for (n = 0; n < 17; n++) {
res = get_bits(gb, 3);
if (cntr[res] > 3) 
return -1;
vbm_tree[res * 3 + cntr[res]++] = n;
}
return 0;
}
static av_cold void wmavoice_init_static_data(void)
{
static const uint8_t bits[] = {
2, 2, 2, 4, 4, 4,
6, 6, 6, 8, 8, 8,
10, 10, 10, 12, 12, 12,
14, 14, 14, 14
};
static const uint16_t codes[] = {
0x0000, 0x0001, 0x0002, 
0x000c, 0x000d, 0x000e, 
0x003c, 0x003d, 0x003e, 
0x00fc, 0x00fd, 0x00fe, 
0x03fc, 0x03fd, 0x03fe, 
0x0ffc, 0x0ffd, 0x0ffe, 
0x3ffc, 0x3ffd, 0x3ffe, 0x3fff 
};
INIT_VLC_STATIC(&frame_type_vlc, VLC_NBITS, sizeof(bits),
bits, 1, 1, codes, 2, 2, 132);
}
static av_cold void wmavoice_flush(AVCodecContext *ctx)
{
WMAVoiceContext *s = ctx->priv_data;
int n;
s->postfilter_agc = 0;
s->sframe_cache_size = 0;
s->skip_bits_next = 0;
for (n = 0; n < s->lsps; n++)
s->prev_lsps[n] = M_PI * (n + 1.0) / (s->lsps + 1.0);
memset(s->excitation_history, 0,
sizeof(*s->excitation_history) * MAX_SIGNAL_HISTORY);
memset(s->synth_history, 0,
sizeof(*s->synth_history) * MAX_LSPS);
memset(s->gain_pred_err, 0,
sizeof(s->gain_pred_err));
if (s->do_apf) {
memset(&s->synth_filter_out_buf[MAX_LSPS_ALIGN16 - s->lsps], 0,
sizeof(*s->synth_filter_out_buf) * s->lsps);
memset(s->dcf_mem, 0,
sizeof(*s->dcf_mem) * 2);
memset(s->zero_exc_pf, 0,
sizeof(*s->zero_exc_pf) * s->history_nsamples);
memset(s->denoise_filter_cache, 0, sizeof(s->denoise_filter_cache));
}
}
static av_cold int wmavoice_decode_init(AVCodecContext *ctx)
{
static AVOnce init_static_once = AV_ONCE_INIT;
int n, flags, pitch_range, lsp16_flag;
WMAVoiceContext *s = ctx->priv_data;
ff_thread_once(&init_static_once, wmavoice_init_static_data);
if (ctx->extradata_size != 46) {
av_log(ctx, AV_LOG_ERROR,
"Invalid extradata size %d (should be 46)\n",
ctx->extradata_size);
return AVERROR_INVALIDDATA;
}
if (ctx->block_align <= 0 || ctx->block_align > (1<<22)) {
av_log(ctx, AV_LOG_ERROR, "Invalid block alignment %d.\n", ctx->block_align);
return AVERROR_INVALIDDATA;
}
flags = AV_RL32(ctx->extradata + 18);
s->spillover_bitsize = 3 + av_ceil_log2(ctx->block_align);
s->do_apf = flags & 0x1;
if (s->do_apf) {
ff_rdft_init(&s->rdft, 7, DFT_R2C);
ff_rdft_init(&s->irdft, 7, IDFT_C2R);
ff_dct_init(&s->dct, 6, DCT_I);
ff_dct_init(&s->dst, 6, DST_I);
ff_sine_window_init(s->cos, 256);
memcpy(&s->sin[255], s->cos, 256 * sizeof(s->cos[0]));
for (n = 0; n < 255; n++) {
s->sin[n] = -s->sin[510 - n];
s->cos[510 - n] = s->cos[n];
}
}
s->denoise_strength = (flags >> 2) & 0xF;
if (s->denoise_strength >= 12) {
av_log(ctx, AV_LOG_ERROR,
"Invalid denoise filter strength %d (max=11)\n",
s->denoise_strength);
return AVERROR_INVALIDDATA;
}
s->denoise_tilt_corr = !!(flags & 0x40);
s->dc_level = (flags >> 7) & 0xF;
s->lsp_q_mode = !!(flags & 0x2000);
s->lsp_def_mode = !!(flags & 0x4000);
lsp16_flag = flags & 0x1000;
if (lsp16_flag) {
s->lsps = 16;
} else {
s->lsps = 10;
}
for (n = 0; n < s->lsps; n++)
s->prev_lsps[n] = M_PI * (n + 1.0) / (s->lsps + 1.0);
init_get_bits(&s->gb, ctx->extradata + 22, (ctx->extradata_size - 22) << 3);
if (decode_vbmtree(&s->gb, s->vbm_tree) < 0) {
av_log(ctx, AV_LOG_ERROR, "Invalid VBM tree; broken extradata?\n");
return AVERROR_INVALIDDATA;
}
if (ctx->sample_rate >= INT_MAX / (256 * 37))
return AVERROR_INVALIDDATA;
s->min_pitch_val = ((ctx->sample_rate << 8) / 400 + 50) >> 8;
s->max_pitch_val = ((ctx->sample_rate << 8) * 37 / 2000 + 50) >> 8;
pitch_range = s->max_pitch_val - s->min_pitch_val;
if (pitch_range <= 0) {
av_log(ctx, AV_LOG_ERROR, "Invalid pitch range; broken extradata?\n");
return AVERROR_INVALIDDATA;
}
s->pitch_nbits = av_ceil_log2(pitch_range);
s->last_pitch_val = 40;
s->last_acb_type = ACB_TYPE_NONE;
s->history_nsamples = s->max_pitch_val + 8;
if (s->min_pitch_val < 1 || s->history_nsamples > MAX_SIGNAL_HISTORY) {
int min_sr = ((((1 << 8) - 50) * 400) + 0xFF) >> 8,
max_sr = ((((MAX_SIGNAL_HISTORY - 8) << 8) + 205) * 2000 / 37) >> 8;
av_log(ctx, AV_LOG_ERROR,
"Unsupported samplerate %d (min=%d, max=%d)\n",
ctx->sample_rate, min_sr, max_sr); 
return AVERROR(ENOSYS);
}
s->block_conv_table[0] = s->min_pitch_val;
s->block_conv_table[1] = (pitch_range * 25) >> 6;
s->block_conv_table[2] = (pitch_range * 44) >> 6;
s->block_conv_table[3] = s->max_pitch_val - 1;
s->block_delta_pitch_hrange = (pitch_range >> 3) & ~0xF;
if (s->block_delta_pitch_hrange <= 0) {
av_log(ctx, AV_LOG_ERROR, "Invalid delta pitch hrange; broken extradata?\n");
return AVERROR_INVALIDDATA;
}
s->block_delta_pitch_nbits = 1 + av_ceil_log2(s->block_delta_pitch_hrange);
s->block_pitch_range = s->block_conv_table[2] +
s->block_conv_table[3] + 1 +
2 * (s->block_conv_table[1] - 2 * s->min_pitch_val);
s->block_pitch_nbits = av_ceil_log2(s->block_pitch_range);
ctx->channels = 1;
ctx->channel_layout = AV_CH_LAYOUT_MONO;
ctx->sample_fmt = AV_SAMPLE_FMT_FLT;
return 0;
}
static void adaptive_gain_control(float *out, const float *in,
const float *speech_synth,
int size, float alpha, float *gain_mem)
{
int i;
float speech_energy = 0.0, postfilter_energy = 0.0, gain_scale_factor;
float mem = *gain_mem;
for (i = 0; i < size; i++) {
speech_energy += fabsf(speech_synth[i]);
postfilter_energy += fabsf(in[i]);
}
gain_scale_factor = postfilter_energy == 0.0 ? 0.0 :
(1.0 - alpha) * speech_energy / postfilter_energy;
for (i = 0; i < size; i++) {
mem = alpha * mem + gain_scale_factor;
out[i] = in[i] * mem;
}
*gain_mem = mem;
}
static int kalman_smoothen(WMAVoiceContext *s, int pitch,
const float *in, float *out, int size)
{
int n;
float optimal_gain = 0, dot;
const float *ptr = &in[-FFMAX(s->min_pitch_val, pitch - 3)],
*end = &in[-FFMIN(s->max_pitch_val, pitch + 3)],
*best_hist_ptr = NULL;
do {
dot = avpriv_scalarproduct_float_c(in, ptr, size);
if (dot > optimal_gain) {
optimal_gain = dot;
best_hist_ptr = ptr;
}
} while (--ptr >= end);
if (optimal_gain <= 0)
return -1;
dot = avpriv_scalarproduct_float_c(best_hist_ptr, best_hist_ptr, size);
if (dot <= 0) 
return -1;
if (optimal_gain <= dot) {
dot = dot / (dot + 0.6 * optimal_gain); 
} else
dot = 0.625;
for (n = 0; n < size; n++)
out[n] = best_hist_ptr[n] + dot * (in[n] - best_hist_ptr[n]);
return 0;
}
static float tilt_factor(const float *lpcs, int n_lpcs)
{
float rh0, rh1;
rh0 = 1.0 + avpriv_scalarproduct_float_c(lpcs, lpcs, n_lpcs);
rh1 = lpcs[0] + avpriv_scalarproduct_float_c(lpcs, &lpcs[1], n_lpcs - 1);
return rh1 / rh0;
}
static void calc_input_response(WMAVoiceContext *s, float *lpcs,
int fcb_type, float *coeffs, int remainder)
{
float last_coeff, min = 15.0, max = -15.0;
float irange, angle_mul, gain_mul, range, sq;
int n, idx;
s->rdft.rdft_calc(&s->rdft, lpcs);
#define log_range(var, assign) do { float tmp = log10f(assign); var = tmp; max = FFMAX(max, tmp); min = FFMIN(min, tmp); } while (0)
log_range(last_coeff, lpcs[1] * lpcs[1]);
for (n = 1; n < 64; n++)
log_range(lpcs[n], lpcs[n * 2] * lpcs[n * 2] +
lpcs[n * 2 + 1] * lpcs[n * 2 + 1]);
log_range(lpcs[0], lpcs[0] * lpcs[0]);
#undef log_range
range = max - min;
lpcs[64] = last_coeff;
irange = 64.0 / range; 
gain_mul = range * (fcb_type == FCB_TYPE_HARDCODED ? (5.0 / 13.0) :
(5.0 / 14.7));
angle_mul = gain_mul * (8.0 * M_LN10 / M_PI);
for (n = 0; n <= 64; n++) {
float pwr;
idx = lrint((max - lpcs[n]) * irange - 1);
idx = FFMAX(0, idx);
pwr = wmavoice_denoise_power_table[s->denoise_strength][idx];
lpcs[n] = angle_mul * pwr;
idx = av_clipf((pwr * gain_mul - 0.0295) * 70.570526123, 0, INT_MAX / 2);
if (idx > 127) { 
coeffs[n] = wmavoice_energy_table[127] *
powf(1.0331663, idx - 127);
} else
coeffs[n] = wmavoice_energy_table[FFMAX(0, idx)];
}
s->dct.dct_calc(&s->dct, lpcs);
s->dst.dct_calc(&s->dst, lpcs);
idx = 255 + av_clip(lpcs[64], -255, 255);
coeffs[0] = coeffs[0] * s->cos[idx];
idx = 255 + av_clip(lpcs[64] - 2 * lpcs[63], -255, 255);
last_coeff = coeffs[64] * s->cos[idx];
for (n = 63;; n--) {
idx = 255 + av_clip(-lpcs[64] - 2 * lpcs[n - 1], -255, 255);
coeffs[n * 2 + 1] = coeffs[n] * s->sin[idx];
coeffs[n * 2] = coeffs[n] * s->cos[idx];
if (!--n) break;
idx = 255 + av_clip( lpcs[64] - 2 * lpcs[n - 1], -255, 255);
coeffs[n * 2 + 1] = coeffs[n] * s->sin[idx];
coeffs[n * 2] = coeffs[n] * s->cos[idx];
}
coeffs[1] = last_coeff;
s->irdft.rdft_calc(&s->irdft, coeffs);
memset(&coeffs[remainder], 0, sizeof(coeffs[0]) * (128 - remainder));
if (s->denoise_tilt_corr) {
float tilt_mem = 0;
coeffs[remainder - 1] = 0;
ff_tilt_compensation(&tilt_mem,
-1.8 * tilt_factor(coeffs, remainder - 1),
coeffs, remainder);
}
sq = (1.0 / 64.0) * sqrtf(1 / avpriv_scalarproduct_float_c(coeffs, coeffs,
remainder));
for (n = 0; n < remainder; n++)
coeffs[n] *= sq;
}
static void wiener_denoise(WMAVoiceContext *s, int fcb_type,
float *synth_pf, int size,
const float *lpcs)
{
int remainder, lim, n;
if (fcb_type != FCB_TYPE_SILENCE) {
float *tilted_lpcs = s->tilted_lpcs_pf,
*coeffs = s->denoise_coeffs_pf, tilt_mem = 0;
tilted_lpcs[0] = 1.0;
memcpy(&tilted_lpcs[1], lpcs, sizeof(lpcs[0]) * s->lsps);
memset(&tilted_lpcs[s->lsps + 1], 0,
sizeof(tilted_lpcs[0]) * (128 - s->lsps - 1));
ff_tilt_compensation(&tilt_mem, 0.7 * tilt_factor(lpcs, s->lsps),
tilted_lpcs, s->lsps + 2);
remainder = FFMIN(127 - size, size - 1);
calc_input_response(s, tilted_lpcs, fcb_type, coeffs, remainder);
memset(&synth_pf[size], 0, sizeof(synth_pf[0]) * (128 - size));
s->rdft.rdft_calc(&s->rdft, synth_pf);
s->rdft.rdft_calc(&s->rdft, coeffs);
synth_pf[0] *= coeffs[0];
synth_pf[1] *= coeffs[1];
for (n = 1; n < 64; n++) {
float v1 = synth_pf[n * 2], v2 = synth_pf[n * 2 + 1];
synth_pf[n * 2] = v1 * coeffs[n * 2] - v2 * coeffs[n * 2 + 1];
synth_pf[n * 2 + 1] = v2 * coeffs[n * 2] + v1 * coeffs[n * 2 + 1];
}
s->irdft.rdft_calc(&s->irdft, synth_pf);
}
if (s->denoise_filter_cache_size) {
lim = FFMIN(s->denoise_filter_cache_size, size);
for (n = 0; n < lim; n++)
synth_pf[n] += s->denoise_filter_cache[n];
s->denoise_filter_cache_size -= lim;
memmove(s->denoise_filter_cache, &s->denoise_filter_cache[size],
sizeof(s->denoise_filter_cache[0]) * s->denoise_filter_cache_size);
}
if (fcb_type != FCB_TYPE_SILENCE) {
lim = FFMIN(remainder, s->denoise_filter_cache_size);
for (n = 0; n < lim; n++)
s->denoise_filter_cache[n] += synth_pf[size + n];
if (lim < remainder) {
memcpy(&s->denoise_filter_cache[lim], &synth_pf[size + lim],
sizeof(s->denoise_filter_cache[0]) * (remainder - lim));
s->denoise_filter_cache_size = remainder;
}
}
}
static void postfilter(WMAVoiceContext *s, const float *synth,
float *samples, int size,
const float *lpcs, float *zero_exc_pf,
int fcb_type, int pitch)
{
float synth_filter_in_buf[MAX_FRAMESIZE / 2],
*synth_pf = &s->synth_filter_out_buf[MAX_LSPS_ALIGN16],
*synth_filter_in = zero_exc_pf;
av_assert0(size <= MAX_FRAMESIZE / 2);
ff_celp_lp_zero_synthesis_filterf(zero_exc_pf, lpcs, synth, size, s->lsps);
if (fcb_type >= FCB_TYPE_AW_PULSES &&
!kalman_smoothen(s, pitch, zero_exc_pf, synth_filter_in_buf, size))
synth_filter_in = synth_filter_in_buf;
ff_celp_lp_synthesis_filterf(synth_pf, lpcs,
synth_filter_in, size, s->lsps);
memcpy(&synth_pf[-s->lsps], &synth_pf[size - s->lsps],
sizeof(synth_pf[0]) * s->lsps);
wiener_denoise(s, fcb_type, synth_pf, size, lpcs);
adaptive_gain_control(samples, synth_pf, synth, size, 0.99,
&s->postfilter_agc);
if (s->dc_level > 8) {
ff_acelp_apply_order_2_transfer_function(samples, samples,
(const float[2]) { -1.99997, 1.0 },
(const float[2]) { -1.9330735188, 0.93589198496 },
0.93980580475, s->dcf_mem, size);
}
}
static void dequant_lsps(double *lsps, int num,
const uint16_t *values,
const uint16_t *sizes,
int n_stages, const uint8_t *table,
const double *mul_q,
const double *base_q)
{
int n, m;
memset(lsps, 0, num * sizeof(*lsps));
for (n = 0; n < n_stages; n++) {
const uint8_t *t_off = &table[values[n] * num];
double base = base_q[n], mul = mul_q[n];
for (m = 0; m < num; m++)
lsps[m] += base + mul * t_off[m];
table += sizes[n] * num;
}
}
static void dequant_lsp10i(GetBitContext *gb, double *lsps)
{
static const uint16_t vec_sizes[4] = { 256, 64, 32, 32 };
static const double mul_lsf[4] = {
5.2187144800e-3, 1.4626986422e-3,
9.6179549166e-4, 1.1325736225e-3
};
static const double base_lsf[4] = {
M_PI * -2.15522e-1, M_PI * -6.1646e-2,
M_PI * -3.3486e-2, M_PI * -5.7408e-2
};
uint16_t v[4];
v[0] = get_bits(gb, 8);
v[1] = get_bits(gb, 6);
v[2] = get_bits(gb, 5);
v[3] = get_bits(gb, 5);
dequant_lsps(lsps, 10, v, vec_sizes, 4, wmavoice_dq_lsp10i,
mul_lsf, base_lsf);
}
static void dequant_lsp10r(GetBitContext *gb,
double *i_lsps, const double *old,
double *a1, double *a2, int q_mode)
{
static const uint16_t vec_sizes[3] = { 128, 64, 64 };
static const double mul_lsf[3] = {
2.5807601174e-3, 1.2354460219e-3, 1.1763821673e-3
};
static const double base_lsf[3] = {
M_PI * -1.07448e-1, M_PI * -5.2706e-2, M_PI * -5.1634e-2
};
const float (*ipol_tab)[2][10] = q_mode ?
wmavoice_lsp10_intercoeff_b : wmavoice_lsp10_intercoeff_a;
uint16_t interpol, v[3];
int n;
dequant_lsp10i(gb, i_lsps);
interpol = get_bits(gb, 5);
v[0] = get_bits(gb, 7);
v[1] = get_bits(gb, 6);
v[2] = get_bits(gb, 6);
for (n = 0; n < 10; n++) {
double delta = old[n] - i_lsps[n];
a1[n] = ipol_tab[interpol][0][n] * delta + i_lsps[n];
a1[10 + n] = ipol_tab[interpol][1][n] * delta + i_lsps[n];
}
dequant_lsps(a2, 20, v, vec_sizes, 3, wmavoice_dq_lsp10r,
mul_lsf, base_lsf);
}
static void dequant_lsp16i(GetBitContext *gb, double *lsps)
{
static const uint16_t vec_sizes[5] = { 256, 64, 128, 64, 128 };
static const double mul_lsf[5] = {
3.3439586280e-3, 6.9908173703e-4,
3.3216608306e-3, 1.0334960326e-3,
3.1899104283e-3
};
static const double base_lsf[5] = {
M_PI * -1.27576e-1, M_PI * -2.4292e-2,
M_PI * -1.28094e-1, M_PI * -3.2128e-2,
M_PI * -1.29816e-1
};
uint16_t v[5];
v[0] = get_bits(gb, 8);
v[1] = get_bits(gb, 6);
v[2] = get_bits(gb, 7);
v[3] = get_bits(gb, 6);
v[4] = get_bits(gb, 7);
dequant_lsps( lsps, 5, v, vec_sizes, 2,
wmavoice_dq_lsp16i1, mul_lsf, base_lsf);
dequant_lsps(&lsps[5], 5, &v[2], &vec_sizes[2], 2,
wmavoice_dq_lsp16i2, &mul_lsf[2], &base_lsf[2]);
dequant_lsps(&lsps[10], 6, &v[4], &vec_sizes[4], 1,
wmavoice_dq_lsp16i3, &mul_lsf[4], &base_lsf[4]);
}
static void dequant_lsp16r(GetBitContext *gb,
double *i_lsps, const double *old,
double *a1, double *a2, int q_mode)
{
static const uint16_t vec_sizes[3] = { 128, 128, 128 };
static const double mul_lsf[3] = {
1.2232979501e-3, 1.4062241527e-3, 1.6114744851e-3
};
static const double base_lsf[3] = {
M_PI * -5.5830e-2, M_PI * -5.2908e-2, M_PI * -5.4776e-2
};
const float (*ipol_tab)[2][16] = q_mode ?
wmavoice_lsp16_intercoeff_b : wmavoice_lsp16_intercoeff_a;
uint16_t interpol, v[3];
int n;
dequant_lsp16i(gb, i_lsps);
interpol = get_bits(gb, 5);
v[0] = get_bits(gb, 7);
v[1] = get_bits(gb, 7);
v[2] = get_bits(gb, 7);
for (n = 0; n < 16; n++) {
double delta = old[n] - i_lsps[n];
a1[n] = ipol_tab[interpol][0][n] * delta + i_lsps[n];
a1[16 + n] = ipol_tab[interpol][1][n] * delta + i_lsps[n];
}
dequant_lsps( a2, 10, v, vec_sizes, 1,
wmavoice_dq_lsp16r1, mul_lsf, base_lsf);
dequant_lsps(&a2[10], 10, &v[1], &vec_sizes[1], 1,
wmavoice_dq_lsp16r2, &mul_lsf[1], &base_lsf[1]);
dequant_lsps(&a2[20], 12, &v[2], &vec_sizes[2], 1,
wmavoice_dq_lsp16r3, &mul_lsf[2], &base_lsf[2]);
}
static void aw_parse_coords(WMAVoiceContext *s, GetBitContext *gb,
const int *pitch)
{
static const int16_t start_offset[94] = {
-11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9, 11,
13, 15, 18, 17, 19, 20, 21, 22, 23, 24, 25, 26,
27, 28, 29, 30, 31, 32, 33, 35, 37, 39, 41, 43,
45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67,
69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91,
93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115,
117, 119, 121, 123, 125, 127, 129, 131, 133, 135, 137, 139,
141, 143, 145, 147, 149, 151, 153, 155, 157, 159
};
int bits, offset;
s->aw_idx_is_ext = 0;
if ((bits = get_bits(gb, 6)) >= 54) {
s->aw_idx_is_ext = 1;
bits += (bits - 54) * 3 + get_bits(gb, 2);
}
s->aw_pulse_range = FFMIN(pitch[0], pitch[1]) > 32 ? 24 : 16;
for (offset = start_offset[bits]; offset < 0; offset += pitch[0]) ;
s->aw_n_pulses[0] = (pitch[0] - 1 + MAX_FRAMESIZE / 2 - offset) / pitch[0];
s->aw_first_pulse_off[0] = offset - s->aw_pulse_range / 2;
offset += s->aw_n_pulses[0] * pitch[0];
s->aw_n_pulses[1] = (pitch[1] - 1 + MAX_FRAMESIZE - offset) / pitch[1];
s->aw_first_pulse_off[1] = offset - (MAX_FRAMESIZE + s->aw_pulse_range) / 2;
if (start_offset[bits] < MAX_FRAMESIZE / 2) {
while (s->aw_first_pulse_off[1] - pitch[1] + s->aw_pulse_range > 0)
s->aw_first_pulse_off[1] -= pitch[1];
if (start_offset[bits] < 0)
while (s->aw_first_pulse_off[0] - pitch[0] + s->aw_pulse_range > 0)
s->aw_first_pulse_off[0] -= pitch[0];
}
}
static int aw_pulse_set2(WMAVoiceContext *s, GetBitContext *gb,
int block_idx, AMRFixed *fcb)
{
uint16_t use_mask_mem[9]; 
uint16_t *use_mask = use_mask_mem + 2;
int pulse_off = s->aw_first_pulse_off[block_idx],
pulse_start, n, idx, range, aidx, start_off = 0;
if (s->aw_n_pulses[block_idx] > 0)
while (pulse_off + s->aw_pulse_range < 1)
pulse_off += fcb->pitch_lag;
if (s->aw_n_pulses[0] > 0) {
if (block_idx == 0) {
range = 32;
} else {
range = 8;
if (s->aw_n_pulses[block_idx] > 0)
pulse_off = s->aw_next_pulse_off_cache;
}
} else
range = 16;
pulse_start = s->aw_n_pulses[block_idx] > 0 ? pulse_off - range / 2 : 0;
memset(&use_mask[-2], 0, 2 * sizeof(use_mask[0]));
memset( use_mask, -1, 5 * sizeof(use_mask[0]));
memset(&use_mask[5], 0, 2 * sizeof(use_mask[0]));
if (s->aw_n_pulses[block_idx] > 0)
for (idx = pulse_off; idx < MAX_FRAMESIZE / 2; idx += fcb->pitch_lag) {
int excl_range = s->aw_pulse_range; 
uint16_t *use_mask_ptr = &use_mask[idx >> 4];
int first_sh = 16 - (idx & 15);
*use_mask_ptr++ &= 0xFFFFu << first_sh;
excl_range -= first_sh;
if (excl_range >= 16) {
*use_mask_ptr++ = 0;
*use_mask_ptr &= 0xFFFF >> (excl_range - 16);
} else
*use_mask_ptr &= 0xFFFF >> excl_range;
}
aidx = get_bits(gb, s->aw_n_pulses[0] > 0 ? 5 - 2 * block_idx : 4);
for (n = 0; n <= aidx; pulse_start++) {
for (idx = pulse_start; idx < 0; idx += fcb->pitch_lag) ;
if (idx >= MAX_FRAMESIZE / 2) { 
if (use_mask[0]) idx = 0x0F;
else if (use_mask[1]) idx = 0x1F;
else if (use_mask[2]) idx = 0x2F;
else if (use_mask[3]) idx = 0x3F;
else if (use_mask[4]) idx = 0x4F;
else return -1;
idx -= av_log2_16bit(use_mask[idx >> 4]);
}
if (use_mask[idx >> 4] & (0x8000 >> (idx & 15))) {
use_mask[idx >> 4] &= ~(0x8000 >> (idx & 15));
n++;
start_off = idx;
}
}
fcb->x[fcb->n] = start_off;
fcb->y[fcb->n] = get_bits1(gb) ? -1.0 : 1.0;
fcb->n++;
n = (MAX_FRAMESIZE / 2 - start_off) % fcb->pitch_lag;
s->aw_next_pulse_off_cache = n ? fcb->pitch_lag - n : 0;
return 0;
}
static void aw_pulse_set1(WMAVoiceContext *s, GetBitContext *gb,
int block_idx, AMRFixed *fcb)
{
int val = get_bits(gb, 12 - 2 * (s->aw_idx_is_ext && !block_idx));
float v;
if (s->aw_n_pulses[block_idx] > 0) {
int n, v_mask, i_mask, sh, n_pulses;
if (s->aw_pulse_range == 24) { 
n_pulses = 3;
v_mask = 8;
i_mask = 7;
sh = 4;
} else { 
n_pulses = 4;
v_mask = 4;
i_mask = 3;
sh = 3;
}
for (n = n_pulses - 1; n >= 0; n--, val >>= sh) {
fcb->y[fcb->n] = (val & v_mask) ? -1.0 : 1.0;
fcb->x[fcb->n] = (val & i_mask) * n_pulses + n +
s->aw_first_pulse_off[block_idx];
while (fcb->x[fcb->n] < 0)
fcb->x[fcb->n] += fcb->pitch_lag;
if (fcb->x[fcb->n] < MAX_FRAMESIZE / 2)
fcb->n++;
}
} else {
int num2 = (val & 0x1FF) >> 1, delta, idx;
if (num2 < 1 * 79) { delta = 1; idx = num2 + 1; }
else if (num2 < 2 * 78) { delta = 3; idx = num2 + 1 - 1 * 77; }
else if (num2 < 3 * 77) { delta = 5; idx = num2 + 1 - 2 * 76; }
else { delta = 7; idx = num2 + 1 - 3 * 75; }
v = (val & 0x200) ? -1.0 : 1.0;
fcb->no_repeat_mask |= 3 << fcb->n;
fcb->x[fcb->n] = idx - delta;
fcb->y[fcb->n] = v;
fcb->x[fcb->n + 1] = idx;
fcb->y[fcb->n + 1] = (val & 1) ? -v : v;
fcb->n += 2;
}
}
static int pRNG(int frame_cntr, int block_num, int block_size)
{
static const unsigned int div_tbl[9][2] = {
{ 8332, 3 * 715827883U }, 
{ 4545, 0 * 390451573U }, 
{ 3124, 11 * 268435456U }, 
{ 2380, 15 * 204522253U }, 
{ 1922, 23 * 165191050U }, 
{ 1612, 23 * 138547333U }, 
{ 1388, 27 * 119304648U }, 
{ 1219, 16 * 104755300U }, 
{ 1086, 39 * 93368855U } 
};
unsigned int z, y, x = MUL16(block_num, 1877) + frame_cntr;
if (x >= 0xFFFF) x -= 0xFFFF; 
y = x - 9 * MULH(477218589, x); 
z = (uint16_t) (x * div_tbl[y][0] + UMULH(x, div_tbl[y][1]));
return z % (1000 - block_size);
}
static void synth_block_hardcoded(WMAVoiceContext *s, GetBitContext *gb,
int block_idx, int size,
const struct frame_type_desc *frame_desc,
float *excitation)
{
float gain;
int n, r_idx;
av_assert0(size <= MAX_FRAMESIZE);
if (frame_desc->fcb_type == FCB_TYPE_SILENCE) {
r_idx = pRNG(s->frame_cntr, block_idx, size);
gain = s->silence_gain;
} else {
r_idx = get_bits(gb, 8);
gain = wmavoice_gain_universal[get_bits(gb, 6)];
}
memset(s->gain_pred_err, 0, sizeof(s->gain_pred_err));
for (n = 0; n < size; n++)
excitation[n] = wmavoice_std_codebook[r_idx + n] * gain;
}
static void synth_block_fcb_acb(WMAVoiceContext *s, GetBitContext *gb,
int block_idx, int size,
int block_pitch_sh2,
const struct frame_type_desc *frame_desc,
float *excitation)
{
static const float gain_coeff[6] = {
0.8169, -0.06545, 0.1726, 0.0185, -0.0359, 0.0458
};
float pulses[MAX_FRAMESIZE / 2], pred_err, acb_gain, fcb_gain;
int n, idx, gain_weight;
AMRFixed fcb;
av_assert0(size <= MAX_FRAMESIZE / 2);
memset(pulses, 0, sizeof(*pulses) * size);
fcb.pitch_lag = block_pitch_sh2 >> 2;
fcb.pitch_fac = 1.0;
fcb.no_repeat_mask = 0;
fcb.n = 0;
if (frame_desc->fcb_type == FCB_TYPE_AW_PULSES) {
aw_pulse_set1(s, gb, block_idx, &fcb);
if (aw_pulse_set2(s, gb, block_idx, &fcb)) {
int r_idx = pRNG(s->frame_cntr, block_idx, size);
for (n = 0; n < size; n++)
excitation[n] =
wmavoice_std_codebook[r_idx + n] * s->silence_gain;
skip_bits(gb, 7 + 1);
return;
}
} else {
int offset_nbits = 5 - frame_desc->log_n_blocks;
fcb.no_repeat_mask = -1;
for (n = 0; n < 5; n++) {
float sign;
int pos1, pos2;
sign = get_bits1(gb) ? 1.0 : -1.0;
pos1 = get_bits(gb, offset_nbits);
fcb.x[fcb.n] = n + 5 * pos1;
fcb.y[fcb.n++] = sign;
if (n < frame_desc->dbl_pulses) {
pos2 = get_bits(gb, offset_nbits);
fcb.x[fcb.n] = n + 5 * pos2;
fcb.y[fcb.n++] = (pos1 < pos2) ? -sign : sign;
}
}
}
ff_set_fixed_vector(pulses, &fcb, 1.0, size);
idx = get_bits(gb, 7);
fcb_gain = expf(avpriv_scalarproduct_float_c(s->gain_pred_err,
gain_coeff, 6) -
5.2409161640 + wmavoice_gain_codebook_fcb[idx]);
acb_gain = wmavoice_gain_codebook_acb[idx];
pred_err = av_clipf(wmavoice_gain_codebook_fcb[idx],
-2.9957322736 ,
1.6094379124 );
gain_weight = 8 >> frame_desc->log_n_blocks;
memmove(&s->gain_pred_err[gain_weight], s->gain_pred_err,
sizeof(*s->gain_pred_err) * (6 - gain_weight));
for (n = 0; n < gain_weight; n++)
s->gain_pred_err[n] = pred_err;
if (frame_desc->acb_type == ACB_TYPE_ASYMMETRIC) {
int len;
for (n = 0; n < size; n += len) {
int next_idx_sh16;
int abs_idx = block_idx * size + n;
int pitch_sh16 = (s->last_pitch_val << 16) +
s->pitch_diff_sh16 * abs_idx;
int pitch = (pitch_sh16 + 0x6FFF) >> 16;
int idx_sh16 = ((pitch << 16) - pitch_sh16) * 8 + 0x58000;
idx = idx_sh16 >> 16;
if (s->pitch_diff_sh16) {
if (s->pitch_diff_sh16 > 0) {
next_idx_sh16 = (idx_sh16) &~ 0xFFFF;
} else
next_idx_sh16 = (idx_sh16 + 0x10000) &~ 0xFFFF;
len = av_clip((idx_sh16 - next_idx_sh16) / s->pitch_diff_sh16 / 8,
1, size - n);
} else
len = size;
ff_acelp_interpolatef(&excitation[n], &excitation[n - pitch],
wmavoice_ipol1_coeffs, 17,
idx, 9, len);
}
} else {
int block_pitch = block_pitch_sh2 >> 2;
idx = block_pitch_sh2 & 3;
if (idx) {
ff_acelp_interpolatef(excitation, &excitation[-block_pitch],
wmavoice_ipol2_coeffs, 4,
idx, 8, size);
} else
av_memcpy_backptr((uint8_t *) excitation, sizeof(float) * block_pitch,
sizeof(float) * size);
}
ff_weighted_vector_sumf(excitation, excitation, pulses,
acb_gain, fcb_gain, size);
}
static void synth_block(WMAVoiceContext *s, GetBitContext *gb,
int block_idx, int size,
int block_pitch_sh2,
const double *lsps, const double *prev_lsps,
const struct frame_type_desc *frame_desc,
float *excitation, float *synth)
{
double i_lsps[MAX_LSPS];
float lpcs[MAX_LSPS];
float fac;
int n;
if (frame_desc->acb_type == ACB_TYPE_NONE)
synth_block_hardcoded(s, gb, block_idx, size, frame_desc, excitation);
else
synth_block_fcb_acb(s, gb, block_idx, size, block_pitch_sh2,
frame_desc, excitation);
fac = (block_idx + 0.5) / frame_desc->n_blocks;
for (n = 0; n < s->lsps; n++) 
i_lsps[n] = cos(prev_lsps[n] + fac * (lsps[n] - prev_lsps[n]));
ff_acelp_lspd2lpc(i_lsps, lpcs, s->lsps >> 1);
ff_celp_lp_synthesis_filterf(synth, lpcs, excitation, size, s->lsps);
}
static int synth_frame(AVCodecContext *ctx, GetBitContext *gb, int frame_idx,
float *samples,
const double *lsps, const double *prev_lsps,
float *excitation, float *synth)
{
WMAVoiceContext *s = ctx->priv_data;
int n, n_blocks_x2, log_n_blocks_x2, av_uninit(cur_pitch_val);
int pitch[MAX_BLOCKS], av_uninit(last_block_pitch);
int bd_idx = s->vbm_tree[get_vlc2(gb, frame_type_vlc.table, 6, 3)], block_nsamples;
if (bd_idx < 0) {
av_log(ctx, AV_LOG_ERROR,
"Invalid frame type VLC code, skipping\n");
return AVERROR_INVALIDDATA;
}
block_nsamples = MAX_FRAMESIZE / frame_descs[bd_idx].n_blocks;
if (frame_descs[bd_idx].acb_type == ACB_TYPE_ASYMMETRIC) {
n_blocks_x2 = frame_descs[bd_idx].n_blocks << 1;
log_n_blocks_x2 = frame_descs[bd_idx].log_n_blocks + 1;
cur_pitch_val = s->min_pitch_val + get_bits(gb, s->pitch_nbits);
cur_pitch_val = FFMIN(cur_pitch_val, s->max_pitch_val - 1);
if (s->last_acb_type == ACB_TYPE_NONE ||
20 * abs(cur_pitch_val - s->last_pitch_val) >
(cur_pitch_val + s->last_pitch_val))
s->last_pitch_val = cur_pitch_val;
for (n = 0; n < frame_descs[bd_idx].n_blocks; n++) {
int fac = n * 2 + 1;
pitch[n] = (MUL16(fac, cur_pitch_val) +
MUL16((n_blocks_x2 - fac), s->last_pitch_val) +
frame_descs[bd_idx].n_blocks) >> log_n_blocks_x2;
}
s->pitch_diff_sh16 =
(cur_pitch_val - s->last_pitch_val) * (1 << 16) / MAX_FRAMESIZE;
}
switch (frame_descs[bd_idx].fcb_type) {
case FCB_TYPE_SILENCE:
s->silence_gain = wmavoice_gain_silence[get_bits(gb, 8)];
break;
case FCB_TYPE_AW_PULSES:
aw_parse_coords(s, gb, pitch);
break;
}
for (n = 0; n < frame_descs[bd_idx].n_blocks; n++) {
int bl_pitch_sh2;
switch (frame_descs[bd_idx].acb_type) {
case ACB_TYPE_HAMMING: {
int block_pitch,
t1 = (s->block_conv_table[1] - s->block_conv_table[0]) << 2,
t2 = (s->block_conv_table[2] - s->block_conv_table[1]) << 1,
t3 = s->block_conv_table[3] - s->block_conv_table[2] + 1;
if (n == 0) {
block_pitch = get_bits(gb, s->block_pitch_nbits);
} else
block_pitch = last_block_pitch - s->block_delta_pitch_hrange +
get_bits(gb, s->block_delta_pitch_nbits);
last_block_pitch = av_clip(block_pitch,
s->block_delta_pitch_hrange,
s->block_pitch_range -
s->block_delta_pitch_hrange);
if (block_pitch < t1) {
bl_pitch_sh2 = (s->block_conv_table[0] << 2) + block_pitch;
} else {
block_pitch -= t1;
if (block_pitch < t2) {
bl_pitch_sh2 =
(s->block_conv_table[1] << 2) + (block_pitch << 1);
} else {
block_pitch -= t2;
if (block_pitch < t3) {
bl_pitch_sh2 =
(s->block_conv_table[2] + block_pitch) << 2;
} else
bl_pitch_sh2 = s->block_conv_table[3] << 2;
}
}
pitch[n] = bl_pitch_sh2 >> 2;
break;
}
case ACB_TYPE_ASYMMETRIC: {
bl_pitch_sh2 = pitch[n] << 2;
break;
}
default: 
bl_pitch_sh2 = 0;
break;
}
synth_block(s, gb, n, block_nsamples, bl_pitch_sh2,
lsps, prev_lsps, &frame_descs[bd_idx],
&excitation[n * block_nsamples],
&synth[n * block_nsamples]);
}
if (s->do_apf) {
double i_lsps[MAX_LSPS];
float lpcs[MAX_LSPS];
for (n = 0; n < s->lsps; n++) 
i_lsps[n] = cos(0.5 * (prev_lsps[n] + lsps[n]));
ff_acelp_lspd2lpc(i_lsps, lpcs, s->lsps >> 1);
postfilter(s, synth, samples, 80, lpcs,
&s->zero_exc_pf[s->history_nsamples + MAX_FRAMESIZE * frame_idx],
frame_descs[bd_idx].fcb_type, pitch[0]);
for (n = 0; n < s->lsps; n++) 
i_lsps[n] = cos(lsps[n]);
ff_acelp_lspd2lpc(i_lsps, lpcs, s->lsps >> 1);
postfilter(s, &synth[80], &samples[80], 80, lpcs,
&s->zero_exc_pf[s->history_nsamples + MAX_FRAMESIZE * frame_idx + 80],
frame_descs[bd_idx].fcb_type, pitch[0]);
} else
memcpy(samples, synth, 160 * sizeof(synth[0]));
s->frame_cntr++;
if (s->frame_cntr >= 0xFFFF) s->frame_cntr -= 0xFFFF; 
s->last_acb_type = frame_descs[bd_idx].acb_type;
switch (frame_descs[bd_idx].acb_type) {
case ACB_TYPE_NONE:
s->last_pitch_val = 0;
break;
case ACB_TYPE_ASYMMETRIC:
s->last_pitch_val = cur_pitch_val;
break;
case ACB_TYPE_HAMMING:
s->last_pitch_val = pitch[frame_descs[bd_idx].n_blocks - 1];
break;
}
return 0;
}
static void stabilize_lsps(double *lsps, int num)
{
int n, m, l;
lsps[0] = FFMAX(lsps[0], 0.0015 * M_PI);
for (n = 1; n < num; n++)
lsps[n] = FFMAX(lsps[n], lsps[n - 1] + 0.0125 * M_PI);
lsps[num - 1] = FFMIN(lsps[num - 1], 0.9985 * M_PI);
for (n = 1; n < num; n++) {
if (lsps[n] < lsps[n - 1]) {
for (m = 1; m < num; m++) {
double tmp = lsps[m];
for (l = m - 1; l >= 0; l--) {
if (lsps[l] <= tmp) break;
lsps[l + 1] = lsps[l];
}
lsps[l + 1] = tmp;
}
break;
}
}
}
static int synth_superframe(AVCodecContext *ctx, AVFrame *frame,
int *got_frame_ptr)
{
WMAVoiceContext *s = ctx->priv_data;
GetBitContext *gb = &s->gb, s_gb;
int n, res, n_samples = MAX_SFRAMESIZE;
double lsps[MAX_FRAMES][MAX_LSPS];
const double *mean_lsf = s->lsps == 16 ?
wmavoice_mean_lsf16[s->lsp_def_mode] : wmavoice_mean_lsf10[s->lsp_def_mode];
float excitation[MAX_SIGNAL_HISTORY + MAX_SFRAMESIZE + 12];
float synth[MAX_LSPS + MAX_SFRAMESIZE];
float *samples;
memcpy(synth, s->synth_history,
s->lsps * sizeof(*synth));
memcpy(excitation, s->excitation_history,
s->history_nsamples * sizeof(*excitation));
if (s->sframe_cache_size > 0) {
gb = &s_gb;
init_get_bits(gb, s->sframe_cache, s->sframe_cache_size);
s->sframe_cache_size = 0;
}
if (!get_bits1(gb)) {
avpriv_request_sample(ctx, "WMAPro-in-WMAVoice");
return AVERROR_PATCHWELCOME;
}
if (get_bits1(gb)) {
if ((n_samples = get_bits(gb, 12)) > MAX_SFRAMESIZE) {
av_log(ctx, AV_LOG_ERROR,
"Superframe encodes > %d samples (%d), not allowed\n",
MAX_SFRAMESIZE, n_samples);
return AVERROR_INVALIDDATA;
}
}
if (s->has_residual_lsps) {
double prev_lsps[MAX_LSPS], a1[MAX_LSPS * 2], a2[MAX_LSPS * 2];
for (n = 0; n < s->lsps; n++)
prev_lsps[n] = s->prev_lsps[n] - mean_lsf[n];
if (s->lsps == 10) {
dequant_lsp10r(gb, lsps[2], prev_lsps, a1, a2, s->lsp_q_mode);
} else 
dequant_lsp16r(gb, lsps[2], prev_lsps, a1, a2, s->lsp_q_mode);
for (n = 0; n < s->lsps; n++) {
lsps[0][n] = mean_lsf[n] + (a1[n] - a2[n * 2]);
lsps[1][n] = mean_lsf[n] + (a1[s->lsps + n] - a2[n * 2 + 1]);
lsps[2][n] += mean_lsf[n];
}
for (n = 0; n < 3; n++)
stabilize_lsps(lsps[n], s->lsps);
}
av_frame_unref(frame);
frame->nb_samples = MAX_SFRAMESIZE;
if ((res = ff_get_buffer(ctx, frame, 0)) < 0)
return res;
frame->nb_samples = n_samples;
samples = (float *)frame->data[0];
for (n = 0; n < 3; n++) {
if (!s->has_residual_lsps) {
int m;
if (s->lsps == 10) {
dequant_lsp10i(gb, lsps[n]);
} else 
dequant_lsp16i(gb, lsps[n]);
for (m = 0; m < s->lsps; m++)
lsps[n][m] += mean_lsf[m];
stabilize_lsps(lsps[n], s->lsps);
}
if ((res = synth_frame(ctx, gb, n,
&samples[n * MAX_FRAMESIZE],
lsps[n], n == 0 ? s->prev_lsps : lsps[n - 1],
&excitation[s->history_nsamples + n * MAX_FRAMESIZE],
&synth[s->lsps + n * MAX_FRAMESIZE]))) {
*got_frame_ptr = 0;
return res;
}
}
if (get_bits1(gb)) {
res = get_bits(gb, 4);
skip_bits(gb, 10 * (res + 1));
}
if (get_bits_left(gb) < 0) {
wmavoice_flush(ctx);
return AVERROR_INVALIDDATA;
}
*got_frame_ptr = 1;
memcpy(s->prev_lsps, lsps[2],
s->lsps * sizeof(*s->prev_lsps));
memcpy(s->synth_history, &synth[MAX_SFRAMESIZE],
s->lsps * sizeof(*synth));
memcpy(s->excitation_history, &excitation[MAX_SFRAMESIZE],
s->history_nsamples * sizeof(*excitation));
if (s->do_apf)
memmove(s->zero_exc_pf, &s->zero_exc_pf[MAX_SFRAMESIZE],
s->history_nsamples * sizeof(*s->zero_exc_pf));
return 0;
}
static int parse_packet_header(WMAVoiceContext *s)
{
GetBitContext *gb = &s->gb;
unsigned int res, n_superframes = 0;
skip_bits(gb, 4); 
s->has_residual_lsps = get_bits1(gb);
do {
if (get_bits_left(gb) < 6 + s->spillover_bitsize)
return AVERROR_INVALIDDATA;
res = get_bits(gb, 6); 
n_superframes += res;
} while (res == 0x3F);
s->spillover_nbits = get_bits(gb, s->spillover_bitsize);
return get_bits_left(gb) >= 0 ? n_superframes : AVERROR_INVALIDDATA;
}
static void copy_bits(PutBitContext *pb,
const uint8_t *data, int size,
GetBitContext *gb, int nbits)
{
int rmn_bytes, rmn_bits;
rmn_bits = rmn_bytes = get_bits_left(gb);
if (rmn_bits < nbits)
return;
if (nbits > pb->size_in_bits - put_bits_count(pb))
return;
rmn_bits &= 7; rmn_bytes >>= 3;
if ((rmn_bits = FFMIN(rmn_bits, nbits)) > 0)
put_bits(pb, rmn_bits, get_bits(gb, rmn_bits));
avpriv_copy_bits(pb, data + size - rmn_bytes,
FFMIN(nbits - rmn_bits, rmn_bytes << 3));
}
static int wmavoice_decode_packet(AVCodecContext *ctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
WMAVoiceContext *s = ctx->priv_data;
GetBitContext *gb = &s->gb;
int size, res, pos;
for (size = avpkt->size; size > ctx->block_align; size -= ctx->block_align);
init_get_bits8(&s->gb, avpkt->data, size);
if (!(size % ctx->block_align)) { 
if (!size) {
s->spillover_nbits = 0;
s->nb_superframes = 0;
} else {
if ((res = parse_packet_header(s)) < 0)
return res;
s->nb_superframes = res;
}
if (s->sframe_cache_size > 0) {
int cnt = get_bits_count(gb);
if (cnt + s->spillover_nbits > avpkt->size * 8) {
s->spillover_nbits = avpkt->size * 8 - cnt;
}
copy_bits(&s->pb, avpkt->data, size, gb, s->spillover_nbits);
flush_put_bits(&s->pb);
s->sframe_cache_size += s->spillover_nbits;
if ((res = synth_superframe(ctx, data, got_frame_ptr)) == 0 &&
*got_frame_ptr) {
cnt += s->spillover_nbits;
s->skip_bits_next = cnt & 7;
res = cnt >> 3;
return res;
} else
skip_bits_long (gb, s->spillover_nbits - cnt +
get_bits_count(gb)); 
} else if (s->spillover_nbits) {
skip_bits_long(gb, s->spillover_nbits); 
}
} else if (s->skip_bits_next)
skip_bits(gb, s->skip_bits_next);
s->sframe_cache_size = 0;
s->skip_bits_next = 0;
pos = get_bits_left(gb);
if (s->nb_superframes-- == 0) {
*got_frame_ptr = 0;
return size;
} else if (s->nb_superframes > 0) {
if ((res = synth_superframe(ctx, data, got_frame_ptr)) < 0) {
return res;
} else if (*got_frame_ptr) {
int cnt = get_bits_count(gb);
s->skip_bits_next = cnt & 7;
res = cnt >> 3;
return res;
}
} else if ((s->sframe_cache_size = pos) > 0) {
init_put_bits(&s->pb, s->sframe_cache, SFRAME_CACHE_MAXSIZE);
copy_bits(&s->pb, avpkt->data, size, gb, s->sframe_cache_size);
}
return size;
}
static av_cold int wmavoice_decode_end(AVCodecContext *ctx)
{
WMAVoiceContext *s = ctx->priv_data;
if (s->do_apf) {
ff_rdft_end(&s->rdft);
ff_rdft_end(&s->irdft);
ff_dct_end(&s->dct);
ff_dct_end(&s->dst);
}
return 0;
}
AVCodec ff_wmavoice_decoder = {
.name = "wmavoice",
.long_name = NULL_IF_CONFIG_SMALL("Windows Media Audio Voice"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_WMAVOICE,
.priv_data_size = sizeof(WMAVoiceContext),
.init = wmavoice_decode_init,
.close = wmavoice_decode_end,
.decode = wmavoice_decode_packet,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DELAY,
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP,
.flush = wmavoice_flush,
};
