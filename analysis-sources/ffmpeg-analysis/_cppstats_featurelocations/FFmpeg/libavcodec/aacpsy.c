

























#include "libavutil/attributes.h"
#include "libavutil/ffmath.h"

#include "avcodec.h"
#include "aactab.h"
#include "psymodel.h"











#define PSY_3GPP_THR_SPREAD_HI 1.5f 
#define PSY_3GPP_THR_SPREAD_LOW 3.0f 

#define PSY_3GPP_EN_SPREAD_HI_L1 2.0f

#define PSY_3GPP_EN_SPREAD_HI_L2 1.5f

#define PSY_3GPP_EN_SPREAD_HI_S 1.5f

#define PSY_3GPP_EN_SPREAD_LOW_L 3.0f

#define PSY_3GPP_EN_SPREAD_LOW_S 2.0f

#define PSY_3GPP_RPEMIN 0.01f
#define PSY_3GPP_RPELEV 2.0f

#define PSY_3GPP_C1 3.0f 
#define PSY_3GPP_C2 1.3219281f 
#define PSY_3GPP_C3 0.55935729f 

#define PSY_SNR_1DB 7.9432821e-1f 
#define PSY_SNR_25DB 3.1622776e-3f 

#define PSY_3GPP_SAVE_SLOPE_L -0.46666667f
#define PSY_3GPP_SAVE_SLOPE_S -0.36363637f
#define PSY_3GPP_SAVE_ADD_L -0.84285712f
#define PSY_3GPP_SAVE_ADD_S -0.75f
#define PSY_3GPP_SPEND_SLOPE_L 0.66666669f
#define PSY_3GPP_SPEND_SLOPE_S 0.81818181f
#define PSY_3GPP_SPEND_ADD_L -0.35f
#define PSY_3GPP_SPEND_ADD_S -0.26111111f
#define PSY_3GPP_CLIP_LO_L 0.2f
#define PSY_3GPP_CLIP_LO_S 0.2f
#define PSY_3GPP_CLIP_HI_L 0.95f
#define PSY_3GPP_CLIP_HI_S 0.75f

#define PSY_3GPP_AH_THR_LONG 0.5f
#define PSY_3GPP_AH_THR_SHORT 0.63f

#define PSY_PE_FORGET_SLOPE 511

enum {
PSY_3GPP_AH_NONE,
PSY_3GPP_AH_INACTIVE,
PSY_3GPP_AH_ACTIVE
};

#define PSY_3GPP_BITS_TO_PE(bits) ((bits) * 1.18f)
#define PSY_3GPP_PE_TO_BITS(bits) ((bits) / 1.18f)


#define PSY_LAME_FIR_LEN 21 
#define AAC_BLOCK_SIZE_LONG 1024 
#define AAC_BLOCK_SIZE_SHORT 128 
#define AAC_NUM_BLOCKS_SHORT 8 
#define PSY_LAME_NUM_SUBBLOCKS 3 








typedef struct AacPsyBand{
float energy; 
float thr; 
float thr_quiet; 
float nz_lines; 
float active_lines; 
float pe; 
float pe_const; 
float norm_fac; 
int avoid_holes; 
}AacPsyBand;




typedef struct AacPsyChannel{
AacPsyBand band[128]; 
AacPsyBand prev_band[128]; 

float win_energy; 
float iir_state[2]; 
uint8_t next_grouping; 
enum WindowSequence next_window_seq; 

float attack_threshold; 
float prev_energy_subshort[AAC_NUM_BLOCKS_SHORT * PSY_LAME_NUM_SUBBLOCKS];
int prev_attack; 
}AacPsyChannel;




typedef struct AacPsyCoeffs{
float ath; 
float barks; 
float spread_low[2]; 
float spread_hi [2]; 
float min_snr; 
}AacPsyCoeffs;




typedef struct AacPsyContext{
int chan_bitrate; 
int frame_bits; 
int fill_level; 
struct {
float min; 
float max; 
float previous; 
float correction; 
} pe;
AacPsyCoeffs psy_coef[2][64];
AacPsyChannel *ch;
float global_quality; 
}AacPsyContext;




typedef struct PsyLamePreset {
int quality; 



float st_lrm; 
} PsyLamePreset;




static const PsyLamePreset psy_abr_map[] = {


{ 8, 6.60},
{ 16, 6.60},
{ 24, 6.60},
{ 32, 6.60},
{ 40, 6.60},
{ 48, 6.60},
{ 56, 6.60},
{ 64, 6.40},
{ 80, 6.00},
{ 96, 5.60},
{112, 5.20},
{128, 5.20},
{160, 5.20}
};




static const PsyLamePreset psy_vbr_map[] = {

{ 0, 4.20},
{ 1, 4.20},
{ 2, 4.20},
{ 3, 4.20},
{ 4, 4.20},
{ 5, 4.20},
{ 6, 4.20},
{ 7, 4.20},
{ 8, 4.20},
{ 9, 4.20},
{10, 4.20}
};




static const float psy_fir_coeffs[] = {
-8.65163e-18 * 2, -0.00851586 * 2, -6.74764e-18 * 2, 0.0209036 * 2,
-3.36639e-17 * 2, -0.0438162 * 2, -1.54175e-17 * 2, 0.0931738 * 2,
-5.52212e-17 * 2, -0.313819 * 2
};

#if ARCH_MIPS
#include "mips/aacpsy_mips.h"
#endif 




static float lame_calc_attack_threshold(int bitrate)
{

int lower_range = 12, upper_range = 12;
int lower_range_kbps = psy_abr_map[12].quality;
int upper_range_kbps = psy_abr_map[12].quality;
int i;




for (i = 1; i < 13; i++) {
if (FFMAX(bitrate, psy_abr_map[i].quality) != bitrate) {
upper_range = i;
upper_range_kbps = psy_abr_map[i ].quality;
lower_range = i - 1;
lower_range_kbps = psy_abr_map[i - 1].quality;
break; 
}
}


if ((upper_range_kbps - bitrate) > (bitrate - lower_range_kbps))
return psy_abr_map[lower_range].st_lrm;
return psy_abr_map[upper_range].st_lrm;
}




static av_cold void lame_window_init(AacPsyContext *ctx, AVCodecContext *avctx)
{
int i, j;

for (i = 0; i < avctx->channels; i++) {
AacPsyChannel *pch = &ctx->ch[i];

if (avctx->flags & AV_CODEC_FLAG_QSCALE)
pch->attack_threshold = psy_vbr_map[avctx->global_quality / FF_QP2LAMBDA].st_lrm;
else
pch->attack_threshold = lame_calc_attack_threshold(avctx->bit_rate / avctx->channels / 1000);

for (j = 0; j < AAC_NUM_BLOCKS_SHORT * PSY_LAME_NUM_SUBBLOCKS; j++)
pch->prev_energy_subshort[j] = 10.0f;
}
}




static av_cold float calc_bark(float f)
{
return 13.3f * atanf(0.00076f * f) + 3.5f * atanf((f / 7500.0f) * (f / 7500.0f));
}

#define ATH_ADD 4




static av_cold float ath(float f, float add)
{
f /= 1000.0f;
return 3.64 * pow(f, -0.8)
- 6.8 * exp(-0.6 * (f - 3.4) * (f - 3.4))
+ 6.0 * exp(-0.15 * (f - 8.7) * (f - 8.7))
+ (0.6 + 0.04 * add) * 0.001 * f * f * f * f;
}

static av_cold int psy_3gpp_init(FFPsyContext *ctx) {
AacPsyContext *pctx;
float bark;
int i, j, g, start;
float prev, minscale, minath, minsnr, pe_min;
int chan_bitrate = ctx->avctx->bit_rate / ((ctx->avctx->flags & AV_CODEC_FLAG_QSCALE) ? 2.0f : ctx->avctx->channels);

const int bandwidth = ctx->cutoff ? ctx->cutoff : AAC_CUTOFF(ctx->avctx);
const float num_bark = calc_bark((float)bandwidth);

ctx->model_priv_data = av_mallocz(sizeof(AacPsyContext));
if (!ctx->model_priv_data)
return AVERROR(ENOMEM);
pctx = ctx->model_priv_data;
pctx->global_quality = (ctx->avctx->global_quality ? ctx->avctx->global_quality : 120) * 0.01f;

if (ctx->avctx->flags & AV_CODEC_FLAG_QSCALE) {

chan_bitrate = (int)(chan_bitrate / 120.0 * (ctx->avctx->global_quality ? ctx->avctx->global_quality : 120));
}

pctx->chan_bitrate = chan_bitrate;
pctx->frame_bits = FFMIN(2560, chan_bitrate * AAC_BLOCK_SIZE_LONG / ctx->avctx->sample_rate);
pctx->pe.min = 8.0f * AAC_BLOCK_SIZE_LONG * bandwidth / (ctx->avctx->sample_rate * 2.0f);
pctx->pe.max = 12.0f * AAC_BLOCK_SIZE_LONG * bandwidth / (ctx->avctx->sample_rate * 2.0f);
ctx->bitres.size = 6144 - pctx->frame_bits;
ctx->bitres.size -= ctx->bitres.size % 8;
pctx->fill_level = ctx->bitres.size;
minath = ath(3410 - 0.733 * ATH_ADD, ATH_ADD);
for (j = 0; j < 2; j++) {
AacPsyCoeffs *coeffs = pctx->psy_coef[j];
const uint8_t *band_sizes = ctx->bands[j];
float line_to_frequency = ctx->avctx->sample_rate / (j ? 256.f : 2048.0f);
float avg_chan_bits = chan_bitrate * (j ? 128.0f : 1024.0f) / ctx->avctx->sample_rate;

float bark_pe = 0.024f * PSY_3GPP_BITS_TO_PE(avg_chan_bits) / num_bark;
float en_spread_low = j ? PSY_3GPP_EN_SPREAD_LOW_S : PSY_3GPP_EN_SPREAD_LOW_L;

float en_spread_hi = (j || (chan_bitrate <= 22.0f)) ? PSY_3GPP_EN_SPREAD_HI_S : PSY_3GPP_EN_SPREAD_HI_L1;

i = 0;
prev = 0.0;
for (g = 0; g < ctx->num_bands[j]; g++) {
i += band_sizes[g];
bark = calc_bark((i-1) * line_to_frequency);
coeffs[g].barks = (bark + prev) / 2.0;
prev = bark;
}
for (g = 0; g < ctx->num_bands[j] - 1; g++) {
AacPsyCoeffs *coeff = &coeffs[g];
float bark_width = coeffs[g+1].barks - coeffs->barks;
coeff->spread_low[0] = ff_exp10(-bark_width * PSY_3GPP_THR_SPREAD_LOW);
coeff->spread_hi [0] = ff_exp10(-bark_width * PSY_3GPP_THR_SPREAD_HI);
coeff->spread_low[1] = ff_exp10(-bark_width * en_spread_low);
coeff->spread_hi [1] = ff_exp10(-bark_width * en_spread_hi);
pe_min = bark_pe * bark_width;
minsnr = exp2(pe_min / band_sizes[g]) - 1.5f;
coeff->min_snr = av_clipf(1.0f / minsnr, PSY_SNR_25DB, PSY_SNR_1DB);
}
start = 0;
for (g = 0; g < ctx->num_bands[j]; g++) {
minscale = ath(start * line_to_frequency, ATH_ADD);
for (i = 1; i < band_sizes[g]; i++)
minscale = FFMIN(minscale, ath((start + i) * line_to_frequency, ATH_ADD));
coeffs[g].ath = minscale - minath;
start += band_sizes[g];
}
}

pctx->ch = av_mallocz_array(ctx->avctx->channels, sizeof(AacPsyChannel));
if (!pctx->ch) {
av_freep(&ctx->model_priv_data);
return AVERROR(ENOMEM);
}

lame_window_init(pctx, ctx->avctx);

return 0;
}




static float iir_filter(int in, float state[2])
{
float ret;

ret = 0.7548f * (in - state[0]) + 0.5095f * state[1];
state[0] = in;
state[1] = ret;
return ret;
}




static const uint8_t window_grouping[9] = {
0xB6, 0x6C, 0xD8, 0xB2, 0x66, 0xC6, 0x96, 0x36, 0x36
};





static av_unused FFPsyWindowInfo psy_3gpp_window(FFPsyContext *ctx,
const int16_t *audio,
const int16_t *la,
int channel, int prev_type)
{
int i, j;
int br = ((AacPsyContext*)ctx->model_priv_data)->chan_bitrate;
int attack_ratio = br <= 16000 ? 18 : 10;
AacPsyContext *pctx = (AacPsyContext*) ctx->model_priv_data;
AacPsyChannel *pch = &pctx->ch[channel];
uint8_t grouping = 0;
int next_type = pch->next_window_seq;
FFPsyWindowInfo wi = { { 0 } };

if (la) {
float s[8], v;
int switch_to_eight = 0;
float sum = 0.0, sum2 = 0.0;
int attack_n = 0;
int stay_short = 0;
for (i = 0; i < 8; i++) {
for (j = 0; j < 128; j++) {
v = iir_filter(la[i*128+j], pch->iir_state);
sum += v*v;
}
s[i] = sum;
sum2 += sum;
}
for (i = 0; i < 8; i++) {
if (s[i] > pch->win_energy * attack_ratio) {
attack_n = i + 1;
switch_to_eight = 1;
break;
}
}
pch->win_energy = pch->win_energy*7/8 + sum2/64;

wi.window_type[1] = prev_type;
switch (prev_type) {
case ONLY_LONG_SEQUENCE:
wi.window_type[0] = switch_to_eight ? LONG_START_SEQUENCE : ONLY_LONG_SEQUENCE;
next_type = switch_to_eight ? EIGHT_SHORT_SEQUENCE : ONLY_LONG_SEQUENCE;
break;
case LONG_START_SEQUENCE:
wi.window_type[0] = EIGHT_SHORT_SEQUENCE;
grouping = pch->next_grouping;
next_type = switch_to_eight ? EIGHT_SHORT_SEQUENCE : LONG_STOP_SEQUENCE;
break;
case LONG_STOP_SEQUENCE:
wi.window_type[0] = switch_to_eight ? LONG_START_SEQUENCE : ONLY_LONG_SEQUENCE;
next_type = switch_to_eight ? EIGHT_SHORT_SEQUENCE : ONLY_LONG_SEQUENCE;
break;
case EIGHT_SHORT_SEQUENCE:
stay_short = next_type == EIGHT_SHORT_SEQUENCE || switch_to_eight;
wi.window_type[0] = stay_short ? EIGHT_SHORT_SEQUENCE : LONG_STOP_SEQUENCE;
grouping = next_type == EIGHT_SHORT_SEQUENCE ? pch->next_grouping : 0;
next_type = switch_to_eight ? EIGHT_SHORT_SEQUENCE : LONG_STOP_SEQUENCE;
break;
}

pch->next_grouping = window_grouping[attack_n];
pch->next_window_seq = next_type;
} else {
for (i = 0; i < 3; i++)
wi.window_type[i] = prev_type;
grouping = (prev_type == EIGHT_SHORT_SEQUENCE) ? window_grouping[0] : 0;
}

wi.window_shape = 1;
if (wi.window_type[0] != EIGHT_SHORT_SEQUENCE) {
wi.num_windows = 1;
wi.grouping[0] = 1;
} else {
int lastgrp = 0;
wi.num_windows = 8;
for (i = 0; i < 8; i++) {
if (!((grouping >> i) & 1))
lastgrp = i;
wi.grouping[lastgrp]++;
}
}

return wi;
}


static int calc_bit_demand(AacPsyContext *ctx, float pe, int bits, int size,
int short_window)
{
const float bitsave_slope = short_window ? PSY_3GPP_SAVE_SLOPE_S : PSY_3GPP_SAVE_SLOPE_L;
const float bitsave_add = short_window ? PSY_3GPP_SAVE_ADD_S : PSY_3GPP_SAVE_ADD_L;
const float bitspend_slope = short_window ? PSY_3GPP_SPEND_SLOPE_S : PSY_3GPP_SPEND_SLOPE_L;
const float bitspend_add = short_window ? PSY_3GPP_SPEND_ADD_S : PSY_3GPP_SPEND_ADD_L;
const float clip_low = short_window ? PSY_3GPP_CLIP_LO_S : PSY_3GPP_CLIP_LO_L;
const float clip_high = short_window ? PSY_3GPP_CLIP_HI_S : PSY_3GPP_CLIP_HI_L;
float clipped_pe, bit_save, bit_spend, bit_factor, fill_level, forgetful_min_pe;

ctx->fill_level += ctx->frame_bits - bits;
ctx->fill_level = av_clip(ctx->fill_level, 0, size);
fill_level = av_clipf((float)ctx->fill_level / size, clip_low, clip_high);
clipped_pe = av_clipf(pe, ctx->pe.min, ctx->pe.max);
bit_save = (fill_level + bitsave_add) * bitsave_slope;
assert(bit_save <= 0.3f && bit_save >= -0.05000001f);
bit_spend = (fill_level + bitspend_add) * bitspend_slope;
assert(bit_spend <= 0.5f && bit_spend >= -0.1f);






bit_factor = 1.0f - bit_save + ((bit_spend - bit_save) / (ctx->pe.max - ctx->pe.min)) * (clipped_pe - ctx->pe.min);




ctx->pe.max = FFMAX(pe, ctx->pe.max);
forgetful_min_pe = ((ctx->pe.min * PSY_PE_FORGET_SLOPE)
+ FFMAX(ctx->pe.min, pe * (pe / ctx->pe.max))) / (PSY_PE_FORGET_SLOPE + 1);
ctx->pe.min = FFMIN(pe, forgetful_min_pe);




return FFMIN(
ctx->frame_bits * bit_factor,
FFMAX(ctx->frame_bits + size - bits, ctx->frame_bits / 8));
}

static float calc_pe_3gpp(AacPsyBand *band)
{
float pe, a;

band->pe = 0.0f;
band->pe_const = 0.0f;
band->active_lines = 0.0f;
if (band->energy > band->thr) {
a = log2f(band->energy);
pe = a - log2f(band->thr);
band->active_lines = band->nz_lines;
if (pe < PSY_3GPP_C1) {
pe = pe * PSY_3GPP_C3 + PSY_3GPP_C2;
a = a * PSY_3GPP_C3 + PSY_3GPP_C2;
band->active_lines *= PSY_3GPP_C3;
}
band->pe = pe * band->nz_lines;
band->pe_const = a * band->nz_lines;
}

return band->pe;
}

static float calc_reduction_3gpp(float a, float desired_pe, float pe,
float active_lines)
{
float thr_avg, reduction;

if(active_lines == 0.0)
return 0;

thr_avg = exp2f((a - pe) / (4.0f * active_lines));
reduction = exp2f((a - desired_pe) / (4.0f * active_lines)) - thr_avg;

return FFMAX(reduction, 0.0f);
}

static float calc_reduced_thr_3gpp(AacPsyBand *band, float min_snr,
float reduction)
{
float thr = band->thr;

if (band->energy > thr) {
thr = sqrtf(thr);
thr = sqrtf(thr) + reduction;
thr *= thr;
thr *= thr;






if (thr > band->energy * min_snr && band->avoid_holes != PSY_3GPP_AH_NONE) {
thr = FFMAX(band->thr, band->energy * min_snr);
band->avoid_holes = PSY_3GPP_AH_ACTIVE;
}
}

return thr;
}

#if !defined(calc_thr_3gpp)
static void calc_thr_3gpp(const FFPsyWindowInfo *wi, const int num_bands, AacPsyChannel *pch,
const uint8_t *band_sizes, const float *coefs, const int cutoff)
{
int i, w, g;
int start = 0, wstart = 0;
for (w = 0; w < wi->num_windows*16; w += 16) {
wstart = 0;
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];

float form_factor = 0.0f;
float Temp;
band->energy = 0.0f;
if (wstart < cutoff) {
for (i = 0; i < band_sizes[g]; i++) {
band->energy += coefs[start+i] * coefs[start+i];
form_factor += sqrtf(fabs(coefs[start+i]));
}
}
Temp = band->energy > 0 ? sqrtf((float)band_sizes[g] / band->energy) : 0;
band->thr = band->energy * 0.001258925f;
band->nz_lines = form_factor * sqrtf(Temp);

start += band_sizes[g];
wstart += band_sizes[g];
}
}
}
#endif 

#if !defined(psy_hp_filter)
static void psy_hp_filter(const float *firbuf, float *hpfsmpl, const float *psy_fir_coeffs)
{
int i, j;
for (i = 0; i < AAC_BLOCK_SIZE_LONG; i++) {
float sum1, sum2;
sum1 = firbuf[i + (PSY_LAME_FIR_LEN - 1) / 2];
sum2 = 0.0;
for (j = 0; j < ((PSY_LAME_FIR_LEN - 1) / 2) - 1; j += 2) {
sum1 += psy_fir_coeffs[j] * (firbuf[i + j] + firbuf[i + PSY_LAME_FIR_LEN - j]);
sum2 += psy_fir_coeffs[j + 1] * (firbuf[i + j + 1] + firbuf[i + PSY_LAME_FIR_LEN - j - 1]);
}


hpfsmpl[i] = (sum1 + sum2) * 32768.0f;
}
}
#endif 




static void psy_3gpp_analyze_channel(FFPsyContext *ctx, int channel,
const float *coefs, const FFPsyWindowInfo *wi)
{
AacPsyContext *pctx = (AacPsyContext*) ctx->model_priv_data;
AacPsyChannel *pch = &pctx->ch[channel];
int i, w, g;
float desired_bits, desired_pe, delta_pe, reduction= NAN, spread_en[128] = {0};
float a = 0.0f, active_lines = 0.0f, norm_fac = 0.0f;
float pe = pctx->chan_bitrate > 32000 ? 0.0f : FFMAX(50.0f, 100.0f - pctx->chan_bitrate * 100.0f / 32000.0f);
const int num_bands = ctx->num_bands[wi->num_windows == 8];
const uint8_t *band_sizes = ctx->bands[wi->num_windows == 8];
AacPsyCoeffs *coeffs = pctx->psy_coef[wi->num_windows == 8];
const float avoid_hole_thr = wi->num_windows == 8 ? PSY_3GPP_AH_THR_SHORT : PSY_3GPP_AH_THR_LONG;
const int bandwidth = ctx->cutoff ? ctx->cutoff : AAC_CUTOFF(ctx->avctx);
const int cutoff = bandwidth * 2048 / wi->num_windows / ctx->avctx->sample_rate;


calc_thr_3gpp(wi, num_bands, pch, band_sizes, coefs, cutoff);


for (w = 0; w < wi->num_windows*16; w += 16) {
AacPsyBand *bands = &pch->band[w];


spread_en[0] = bands[0].energy;
for (g = 1; g < num_bands; g++) {
bands[g].thr = FFMAX(bands[g].thr, bands[g-1].thr * coeffs[g].spread_hi[0]);
spread_en[w+g] = FFMAX(bands[g].energy, spread_en[w+g-1] * coeffs[g].spread_hi[1]);
}
for (g = num_bands - 2; g >= 0; g--) {
bands[g].thr = FFMAX(bands[g].thr, bands[g+1].thr * coeffs[g].spread_low[0]);
spread_en[w+g] = FFMAX(spread_en[w+g], spread_en[w+g+1] * coeffs[g].spread_low[1]);
}

for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &bands[g];

band->thr_quiet = band->thr = FFMAX(band->thr, coeffs[g].ath);

if (!(wi->window_type[0] == LONG_STOP_SEQUENCE || (!w && wi->window_type[1] == LONG_START_SEQUENCE)))
band->thr = FFMAX(PSY_3GPP_RPEMIN*band->thr, FFMIN(band->thr,
PSY_3GPP_RPELEV*pch->prev_band[w+g].thr_quiet));


pe += calc_pe_3gpp(band);
a += band->pe_const;
active_lines += band->active_lines;


if (spread_en[w+g] * avoid_hole_thr > band->energy || coeffs[g].min_snr > 1.0f)
band->avoid_holes = PSY_3GPP_AH_NONE;
else
band->avoid_holes = PSY_3GPP_AH_INACTIVE;
}
}


ctx->ch[channel].entropy = pe;
if (ctx->avctx->flags & AV_CODEC_FLAG_QSCALE) {



desired_pe = pe * (ctx->avctx->global_quality ? ctx->avctx->global_quality : 120) / (2 * 2.5f * 120.0f);
desired_bits = FFMIN(2560, PSY_3GPP_PE_TO_BITS(desired_pe));
desired_pe = PSY_3GPP_BITS_TO_PE(desired_bits); 


if (ctx->bitres.bits > 0) {
desired_bits = FFMIN(2560, PSY_3GPP_PE_TO_BITS(desired_pe));
desired_pe = PSY_3GPP_BITS_TO_PE(desired_bits); 
}

pctx->pe.max = FFMAX(pe, pctx->pe.max);
pctx->pe.min = FFMIN(pe, pctx->pe.min);
} else {
desired_bits = calc_bit_demand(pctx, pe, ctx->bitres.bits, ctx->bitres.size, wi->num_windows == 8);
desired_pe = PSY_3GPP_BITS_TO_PE(desired_bits);





if (ctx->bitres.bits > 0)
desired_pe *= av_clipf(pctx->pe.previous / PSY_3GPP_BITS_TO_PE(ctx->bitres.bits),
0.85f, 1.15f);
}
pctx->pe.previous = PSY_3GPP_BITS_TO_PE(desired_bits);
ctx->bitres.alloc = desired_bits;

if (desired_pe < pe) {

for (w = 0; w < wi->num_windows*16; w += 16) {
reduction = calc_reduction_3gpp(a, desired_pe, pe, active_lines);
pe = 0.0f;
a = 0.0f;
active_lines = 0.0f;
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];

band->thr = calc_reduced_thr_3gpp(band, coeffs[g].min_snr, reduction);

pe += calc_pe_3gpp(band);
a += band->pe_const;
active_lines += band->active_lines;
}
}


for (i = 0; i < 2; i++) {
float pe_no_ah = 0.0f, desired_pe_no_ah;
active_lines = a = 0.0f;
for (w = 0; w < wi->num_windows*16; w += 16) {
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];

if (band->avoid_holes != PSY_3GPP_AH_ACTIVE) {
pe_no_ah += band->pe;
a += band->pe_const;
active_lines += band->active_lines;
}
}
}
desired_pe_no_ah = FFMAX(desired_pe - (pe - pe_no_ah), 0.0f);
if (active_lines > 0.0f)
reduction = calc_reduction_3gpp(a, desired_pe_no_ah, pe_no_ah, active_lines);

pe = 0.0f;
for (w = 0; w < wi->num_windows*16; w += 16) {
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];

if (active_lines > 0.0f)
band->thr = calc_reduced_thr_3gpp(band, coeffs[g].min_snr, reduction);
pe += calc_pe_3gpp(band);
if (band->thr > 0.0f)
band->norm_fac = band->active_lines / band->thr;
else
band->norm_fac = 0.0f;
norm_fac += band->norm_fac;
}
}
delta_pe = desired_pe - pe;
if (fabs(delta_pe) > 0.05f * desired_pe)
break;
}

if (pe < 1.15f * desired_pe) {

norm_fac = 1.0f / norm_fac;
for (w = 0; w < wi->num_windows*16; w += 16) {
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];

if (band->active_lines > 0.5f) {
float delta_sfb_pe = band->norm_fac * norm_fac * delta_pe;
float thr = band->thr;

thr *= exp2f(delta_sfb_pe / band->active_lines);
if (thr > coeffs[g].min_snr * band->energy && band->avoid_holes == PSY_3GPP_AH_INACTIVE)
thr = FFMAX(band->thr, coeffs[g].min_snr * band->energy);
band->thr = thr;
}
}
}
} else {

g = num_bands;
while (pe > desired_pe && g--) {
for (w = 0; w < wi->num_windows*16; w+= 16) {
AacPsyBand *band = &pch->band[w+g];
if (band->avoid_holes != PSY_3GPP_AH_NONE && coeffs[g].min_snr < PSY_SNR_1DB) {
coeffs[g].min_snr = PSY_SNR_1DB;
band->thr = band->energy * PSY_SNR_1DB;
pe += band->active_lines * 1.5f - band->pe;
}
}
}

}
}

for (w = 0; w < wi->num_windows*16; w += 16) {
for (g = 0; g < num_bands; g++) {
AacPsyBand *band = &pch->band[w+g];
FFPsyBand *psy_band = &ctx->ch[channel].psy_bands[w+g];

psy_band->threshold = band->thr;
psy_band->energy = band->energy;
psy_band->spread = band->active_lines * 2.0f / band_sizes[g];
psy_band->bits = PSY_3GPP_PE_TO_BITS(band->pe);
}
}

memcpy(pch->prev_band, pch->band, sizeof(pch->band));
}

static void psy_3gpp_analyze(FFPsyContext *ctx, int channel,
const float **coeffs, const FFPsyWindowInfo *wi)
{
int ch;
FFPsyChannelGroup *group = ff_psy_find_group(ctx, channel);

for (ch = 0; ch < group->num_ch; ch++)
psy_3gpp_analyze_channel(ctx, channel + ch, coeffs[ch], &wi[ch]);
}

static av_cold void psy_3gpp_end(FFPsyContext *apc)
{
AacPsyContext *pctx = (AacPsyContext*) apc->model_priv_data;
av_freep(&pctx->ch);
av_freep(&apc->model_priv_data);
}

static void lame_apply_block_type(AacPsyChannel *ctx, FFPsyWindowInfo *wi, int uselongblock)
{
int blocktype = ONLY_LONG_SEQUENCE;
if (uselongblock) {
if (ctx->next_window_seq == EIGHT_SHORT_SEQUENCE)
blocktype = LONG_STOP_SEQUENCE;
} else {
blocktype = EIGHT_SHORT_SEQUENCE;
if (ctx->next_window_seq == ONLY_LONG_SEQUENCE)
ctx->next_window_seq = LONG_START_SEQUENCE;
if (ctx->next_window_seq == LONG_STOP_SEQUENCE)
ctx->next_window_seq = EIGHT_SHORT_SEQUENCE;
}

wi->window_type[0] = ctx->next_window_seq;
ctx->next_window_seq = blocktype;
}

static FFPsyWindowInfo psy_lame_window(FFPsyContext *ctx, const float *audio,
const float *la, int channel, int prev_type)
{
AacPsyContext *pctx = (AacPsyContext*) ctx->model_priv_data;
AacPsyChannel *pch = &pctx->ch[channel];
int grouping = 0;
int uselongblock = 1;
int attacks[AAC_NUM_BLOCKS_SHORT + 1] = { 0 };
int i;
FFPsyWindowInfo wi = { { 0 } };

if (la) {
float hpfsmpl[AAC_BLOCK_SIZE_LONG];
const float *pf = hpfsmpl;
float attack_intensity[(AAC_NUM_BLOCKS_SHORT + 1) * PSY_LAME_NUM_SUBBLOCKS];
float energy_subshort[(AAC_NUM_BLOCKS_SHORT + 1) * PSY_LAME_NUM_SUBBLOCKS];
float energy_short[AAC_NUM_BLOCKS_SHORT + 1] = { 0 };
const float *firbuf = la + (AAC_BLOCK_SIZE_SHORT/4 - PSY_LAME_FIR_LEN);
int att_sum = 0;


psy_hp_filter(firbuf, hpfsmpl, psy_fir_coeffs);


for (i = 0; i < PSY_LAME_NUM_SUBBLOCKS; i++) {
energy_subshort[i] = pch->prev_energy_subshort[i + ((AAC_NUM_BLOCKS_SHORT - 1) * PSY_LAME_NUM_SUBBLOCKS)];
assert(pch->prev_energy_subshort[i + ((AAC_NUM_BLOCKS_SHORT - 2) * PSY_LAME_NUM_SUBBLOCKS + 1)] > 0);
attack_intensity[i] = energy_subshort[i] / pch->prev_energy_subshort[i + ((AAC_NUM_BLOCKS_SHORT - 2) * PSY_LAME_NUM_SUBBLOCKS + 1)];
energy_short[0] += energy_subshort[i];
}

for (i = 0; i < AAC_NUM_BLOCKS_SHORT * PSY_LAME_NUM_SUBBLOCKS; i++) {
const float *const pfe = pf + AAC_BLOCK_SIZE_LONG / (AAC_NUM_BLOCKS_SHORT * PSY_LAME_NUM_SUBBLOCKS);
float p = 1.0f;
for (; pf < pfe; pf++)
p = FFMAX(p, fabsf(*pf));
pch->prev_energy_subshort[i] = energy_subshort[i + PSY_LAME_NUM_SUBBLOCKS] = p;
energy_short[1 + i / PSY_LAME_NUM_SUBBLOCKS] += p;







if (p > energy_subshort[i + 1])
p = p / energy_subshort[i + 1];
else if (energy_subshort[i + 1] > p * 10.0f)
p = energy_subshort[i + 1] / (p * 10.0f);
else
p = 0.0;
attack_intensity[i + PSY_LAME_NUM_SUBBLOCKS] = p;
}


for (i = 0; i < (AAC_NUM_BLOCKS_SHORT + 1) * PSY_LAME_NUM_SUBBLOCKS; i++)
if (!attacks[i / PSY_LAME_NUM_SUBBLOCKS])
if (attack_intensity[i] > pch->attack_threshold)
attacks[i / PSY_LAME_NUM_SUBBLOCKS] = (i % PSY_LAME_NUM_SUBBLOCKS) + 1;





for (i = 1; i < AAC_NUM_BLOCKS_SHORT + 1; i++) {
const float u = energy_short[i - 1];
const float v = energy_short[i];
const float m = FFMAX(u, v);
if (m < 40000) { 
if (u < 1.7f * v && v < 1.7f * u) { 
if (i == 1 && attacks[0] < attacks[i])
attacks[0] = 0;
attacks[i] = 0;
}
}
att_sum += attacks[i];
}

if (attacks[0] <= pch->prev_attack)
attacks[0] = 0;

att_sum += attacks[0];

if (pch->prev_attack == 3 || att_sum) {
uselongblock = 0;

for (i = 1; i < AAC_NUM_BLOCKS_SHORT + 1; i++)
if (attacks[i] && attacks[i-1])
attacks[i] = 0;
}
} else {

uselongblock = !(prev_type == EIGHT_SHORT_SEQUENCE);
}

lame_apply_block_type(pch, &wi, uselongblock);

wi.window_type[1] = prev_type;
if (wi.window_type[0] != EIGHT_SHORT_SEQUENCE) {

wi.num_windows = 1;
wi.grouping[0] = 1;
if (wi.window_type[0] == LONG_START_SEQUENCE)
wi.window_shape = 0;
else
wi.window_shape = 1;

} else {
int lastgrp = 0;

wi.num_windows = 8;
wi.window_shape = 0;
for (i = 0; i < 8; i++) {
if (!((pch->next_grouping >> i) & 1))
lastgrp = i;
wi.grouping[lastgrp]++;
}
}







for (i = 0; i < 9; i++) {
if (attacks[i]) {
grouping = i;
break;
}
}
pch->next_grouping = window_grouping[grouping];

pch->prev_attack = attacks[8];

return wi;
}

const FFPsyModel ff_aac_psy_model =
{
.name = "3GPP TS 26.403-inspired model",
.init = psy_3gpp_init,
.window = psy_lame_window,
.analyze = psy_3gpp_analyze,
.end = psy_3gpp_end,
};
