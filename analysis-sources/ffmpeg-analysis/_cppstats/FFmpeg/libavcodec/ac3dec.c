#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include "libavutil/channel_layout.h"
#include "libavutil/crc.h"
#include "libavutil/downmix_info.h"
#include "libavutil/opt.h"
#include "bswapdsp.h"
#include "internal.h"
#include "aac_ac3_parser.h"
#include "ac3_parser_internal.h"
#include "ac3dec.h"
#include "ac3dec_data.h"
#include "kbdwin.h"
static uint8_t ungroup_3_in_7_bits_tab[128][3];
static int b1_mantissas[32][3];
static int b2_mantissas[128][3];
static int b3_mantissas[8];
static int b4_mantissas[128][2];
static int b5_mantissas[16];
static const uint8_t quantization_tab[16] = {
0, 3, 5, 7, 11, 15,
5, 6, 7, 8, 9, 10, 11, 12, 14, 16
};
#if (!USE_FIXED)
static float dynamic_range_tab[256];
float ff_ac3_heavy_dynamic_range_tab[256];
#endif
static const float gain_levels[9] = {
LEVEL_PLUS_3DB,
LEVEL_PLUS_1POINT5DB,
LEVEL_ONE,
LEVEL_MINUS_1POINT5DB,
LEVEL_MINUS_3DB,
LEVEL_MINUS_4POINT5DB,
LEVEL_MINUS_6DB,
LEVEL_ZERO,
LEVEL_MINUS_9DB
};
static const float gain_levels_lfe[32] = {
3.162275, 2.818382, 2.511886, 2.238719, 1.995261, 1.778278, 1.584893,
1.412536, 1.258924, 1.122018, 1.000000, 0.891251, 0.794328, 0.707946,
0.630957, 0.562341, 0.501187, 0.446683, 0.398107, 0.354813, 0.316227,
0.281838, 0.251188, 0.223872, 0.199526, 0.177828, 0.158489, 0.141253,
0.125892, 0.112201, 0.100000, 0.089125
};
static const uint8_t ac3_default_coeffs[8][5][2] = {
{ { 2, 7 }, { 7, 2 }, },
{ { 4, 4 }, },
{ { 2, 7 }, { 7, 2 }, },
{ { 2, 7 }, { 5, 5 }, { 7, 2 }, },
{ { 2, 7 }, { 7, 2 }, { 6, 6 }, },
{ { 2, 7 }, { 5, 5 }, { 7, 2 }, { 8, 8 }, },
{ { 2, 7 }, { 7, 2 }, { 6, 7 }, { 7, 6 }, },
{ { 2, 7 }, { 5, 5 }, { 7, 2 }, { 6, 7 }, { 7, 6 }, },
};
static inline int
symmetric_dequant(int code, int levels)
{
return ((code - (levels >> 1)) * (1 << 24)) / levels;
}
static av_cold void ac3_tables_init(void)
{
int i;
for (i = 0; i < 128; i++) {
ungroup_3_in_7_bits_tab[i][0] = i / 25;
ungroup_3_in_7_bits_tab[i][1] = (i % 25) / 5;
ungroup_3_in_7_bits_tab[i][2] = (i % 25) % 5;
}
for (i = 0; i < 32; i++) {
b1_mantissas[i][0] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][0], 3);
b1_mantissas[i][1] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][1], 3);
b1_mantissas[i][2] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][2], 3);
}
for (i = 0; i < 128; i++) {
b2_mantissas[i][0] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][0], 5);
b2_mantissas[i][1] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][1], 5);
b2_mantissas[i][2] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][2], 5);
b4_mantissas[i][0] = symmetric_dequant(i / 11, 11);
b4_mantissas[i][1] = symmetric_dequant(i % 11, 11);
}
for (i = 0; i < 7; i++) {
b3_mantissas[i] = symmetric_dequant(i, 7);
}
for (i = 0; i < 15; i++) {
b5_mantissas[i] = symmetric_dequant(i, 15);
}
#if (!USE_FIXED)
for (i = 0; i < 256; i++) {
int v = (i >> 5) - ((i >> 7) << 3) - 5;
dynamic_range_tab[i] = powf(2.0f, v) * ((i & 0x1F) | 0x20);
}
for (i = 0; i < 256; i++) {
int v = (i >> 4) - ((i >> 7) << 4) - 4;
ff_ac3_heavy_dynamic_range_tab[i] = powf(2.0f, v) * ((i & 0xF) | 0x10);
}
#endif
}
static av_cold int ac3_decode_init(AVCodecContext *avctx)
{
AC3DecodeContext *s = avctx->priv_data;
int i;
s->avctx = avctx;
ac3_tables_init();
ff_mdct_init(&s->imdct_256, 8, 1, 1.0);
ff_mdct_init(&s->imdct_512, 9, 1, 1.0);
AC3_RENAME(ff_kbd_window_init)(s->window, 5.0, 256);
ff_bswapdsp_init(&s->bdsp);
#if (USE_FIXED)
s->fdsp = avpriv_alloc_fixed_dsp(avctx->flags & AV_CODEC_FLAG_BITEXACT);
#else
s->fdsp = avpriv_float_dsp_alloc(avctx->flags & AV_CODEC_FLAG_BITEXACT);
ff_fmt_convert_init(&s->fmt_conv, avctx);
#endif
ff_ac3dsp_init(&s->ac3dsp, avctx->flags & AV_CODEC_FLAG_BITEXACT);
av_lfg_init(&s->dith_state, 0);
if (USE_FIXED)
avctx->sample_fmt = AV_SAMPLE_FMT_S16P;
else
avctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
if (avctx->channels > 1 &&
avctx->request_channel_layout == AV_CH_LAYOUT_MONO)
avctx->channels = 1;
else if (avctx->channels > 2 &&
avctx->request_channel_layout == AV_CH_LAYOUT_STEREO)
avctx->channels = 2;
s->downmixed = 1;
for (i = 0; i < AC3_MAX_CHANNELS; i++) {
s->xcfptr[i] = s->transform_coeffs[i];
s->dlyptr[i] = s->delay[i];
}
return 0;
}
static int ac3_parse_header(AC3DecodeContext *s)
{
GetBitContext *gbc = &s->gbc;
int i;
i = !s->channel_mode;
do {
s->dialog_normalization[(!s->channel_mode)-i] = -get_bits(gbc, 5);
if (s->dialog_normalization[(!s->channel_mode)-i] == 0) {
s->dialog_normalization[(!s->channel_mode)-i] = -31;
}
if (s->target_level != 0) {
s->level_gain[(!s->channel_mode)-i] = powf(2.0f,
(float)(s->target_level -
s->dialog_normalization[(!s->channel_mode)-i])/6.0f);
}
if (s->compression_exists[(!s->channel_mode)-i] = get_bits1(gbc)) {
s->heavy_dynamic_range[(!s->channel_mode)-i] =
AC3_HEAVY_RANGE(get_bits(gbc, 8));
}
if (get_bits1(gbc))
skip_bits(gbc, 8); 
if (get_bits1(gbc))
skip_bits(gbc, 7); 
} while (i--);
skip_bits(gbc, 2); 
if (s->bitstream_id != 6) {
if (get_bits1(gbc))
skip_bits(gbc, 14); 
if (get_bits1(gbc))
skip_bits(gbc, 14); 
} else {
if (get_bits1(gbc)) {
s->preferred_downmix = get_bits(gbc, 2);
s->center_mix_level_ltrt = get_bits(gbc, 3);
s->surround_mix_level_ltrt = av_clip(get_bits(gbc, 3), 3, 7);
s->center_mix_level = get_bits(gbc, 3);
s->surround_mix_level = av_clip(get_bits(gbc, 3), 3, 7);
}
if (get_bits1(gbc)) {
s->dolby_surround_ex_mode = get_bits(gbc, 2);
s->dolby_headphone_mode = get_bits(gbc, 2);
skip_bits(gbc, 10); 
}
}
if (get_bits1(gbc)) {
i = get_bits(gbc, 6);
do {
skip_bits(gbc, 8);
} while (i--);
}
return 0;
}
static int parse_frame_header(AC3DecodeContext *s)
{
AC3HeaderInfo hdr;
int err;
err = ff_ac3_parse_header(&s->gbc, &hdr);
if (err)
return err;
s->bit_alloc_params.sr_code = hdr.sr_code;
s->bitstream_id = hdr.bitstream_id;
s->bitstream_mode = hdr.bitstream_mode;
s->channel_mode = hdr.channel_mode;
s->lfe_on = hdr.lfe_on;
s->bit_alloc_params.sr_shift = hdr.sr_shift;
s->sample_rate = hdr.sample_rate;
s->bit_rate = hdr.bit_rate;
s->channels = hdr.channels;
s->fbw_channels = s->channels - s->lfe_on;
s->lfe_ch = s->fbw_channels + 1;
s->frame_size = hdr.frame_size;
s->superframe_size += hdr.frame_size;
s->preferred_downmix = AC3_DMIXMOD_NOTINDICATED;
s->center_mix_level = hdr.center_mix_level;
s->center_mix_level_ltrt = 4; 
s->surround_mix_level = hdr.surround_mix_level;
s->surround_mix_level_ltrt = 4; 
s->lfe_mix_level_exists = 0;
s->num_blocks = hdr.num_blocks;
s->frame_type = hdr.frame_type;
s->substreamid = hdr.substreamid;
s->dolby_surround_mode = hdr.dolby_surround_mode;
s->dolby_surround_ex_mode = AC3_DSUREXMOD_NOTINDICATED;
s->dolby_headphone_mode = AC3_DHEADPHONMOD_NOTINDICATED;
if (s->lfe_on) {
s->start_freq[s->lfe_ch] = 0;
s->end_freq[s->lfe_ch] = 7;
s->num_exp_groups[s->lfe_ch] = 2;
s->channel_in_cpl[s->lfe_ch] = 0;
}
if (s->bitstream_id <= 10) {
s->eac3 = 0;
s->snr_offset_strategy = 2;
s->block_switch_syntax = 1;
s->dither_flag_syntax = 1;
s->bit_allocation_syntax = 1;
s->fast_gain_syntax = 0;
s->first_cpl_leak = 0;
s->dba_syntax = 1;
s->skip_syntax = 1;
memset(s->channel_uses_aht, 0, sizeof(s->channel_uses_aht));
return ac3_parse_header(s);
} else if (CONFIG_EAC3_DECODER) {
s->eac3 = 1;
return ff_eac3_parse_header(s);
} else {
av_log(s->avctx, AV_LOG_ERROR, "E-AC-3 support not compiled in\n");
return AVERROR(ENOSYS);
}
}
static int set_downmix_coeffs(AC3DecodeContext *s)
{
int i;
float cmix = gain_levels[s-> center_mix_level];
float smix = gain_levels[s->surround_mix_level];
float norm0, norm1;
float downmix_coeffs[2][AC3_MAX_CHANNELS];
if (!s->downmix_coeffs[0]) {
s->downmix_coeffs[0] = av_malloc_array(2 * AC3_MAX_CHANNELS,
sizeof(**s->downmix_coeffs));
if (!s->downmix_coeffs[0])
return AVERROR(ENOMEM);
s->downmix_coeffs[1] = s->downmix_coeffs[0] + AC3_MAX_CHANNELS;
}
for (i = 0; i < s->fbw_channels; i++) {
downmix_coeffs[0][i] = gain_levels[ac3_default_coeffs[s->channel_mode][i][0]];
downmix_coeffs[1][i] = gain_levels[ac3_default_coeffs[s->channel_mode][i][1]];
}
if (s->channel_mode > 1 && s->channel_mode & 1) {
downmix_coeffs[0][1] = downmix_coeffs[1][1] = cmix;
}
if (s->channel_mode == AC3_CHMODE_2F1R || s->channel_mode == AC3_CHMODE_3F1R) {
int nf = s->channel_mode - 2;
downmix_coeffs[0][nf] = downmix_coeffs[1][nf] = smix * LEVEL_MINUS_3DB;
}
if (s->channel_mode == AC3_CHMODE_2F2R || s->channel_mode == AC3_CHMODE_3F2R) {
int nf = s->channel_mode - 4;
downmix_coeffs[0][nf] = downmix_coeffs[1][nf+1] = smix;
}
norm0 = norm1 = 0.0;
for (i = 0; i < s->fbw_channels; i++) {
norm0 += downmix_coeffs[0][i];
norm1 += downmix_coeffs[1][i];
}
norm0 = 1.0f / norm0;
norm1 = 1.0f / norm1;
for (i = 0; i < s->fbw_channels; i++) {
downmix_coeffs[0][i] *= norm0;
downmix_coeffs[1][i] *= norm1;
}
if (s->output_mode == AC3_CHMODE_MONO) {
for (i = 0; i < s->fbw_channels; i++)
downmix_coeffs[0][i] = (downmix_coeffs[0][i] +
downmix_coeffs[1][i]) * LEVEL_MINUS_3DB;
}
for (i = 0; i < s->fbw_channels; i++) {
s->downmix_coeffs[0][i] = FIXR12(downmix_coeffs[0][i]);
s->downmix_coeffs[1][i] = FIXR12(downmix_coeffs[1][i]);
}
return 0;
}
static int decode_exponents(AC3DecodeContext *s,
GetBitContext *gbc, int exp_strategy, int ngrps,
uint8_t absexp, int8_t *dexps)
{
int i, j, grp, group_size;
int dexp[256];
int expacc, prevexp;
group_size = exp_strategy + (exp_strategy == EXP_D45);
for (grp = 0, i = 0; grp < ngrps; grp++) {
expacc = get_bits(gbc, 7);
if (expacc >= 125) {
av_log(s->avctx, AV_LOG_ERROR, "expacc %d is out-of-range\n", expacc);
return AVERROR_INVALIDDATA;
}
dexp[i++] = ungroup_3_in_7_bits_tab[expacc][0];
dexp[i++] = ungroup_3_in_7_bits_tab[expacc][1];
dexp[i++] = ungroup_3_in_7_bits_tab[expacc][2];
}
prevexp = absexp;
for (i = 0, j = 0; i < ngrps * 3; i++) {
prevexp += dexp[i] - 2;
if (prevexp > 24U) {
av_log(s->avctx, AV_LOG_ERROR, "exponent %d is out-of-range\n", prevexp);
return AVERROR_INVALIDDATA;
}
switch (group_size) {
case 4: dexps[j++] = prevexp;
dexps[j++] = prevexp;
case 2: dexps[j++] = prevexp;
case 1: dexps[j++] = prevexp;
}
}
return 0;
}
static void calc_transform_coeffs_cpl(AC3DecodeContext *s)
{
int bin, band, ch;
bin = s->start_freq[CPL_CH];
for (band = 0; band < s->num_cpl_bands; band++) {
int band_start = bin;
int band_end = bin + s->cpl_band_sizes[band];
for (ch = 1; ch <= s->fbw_channels; ch++) {
if (s->channel_in_cpl[ch]) {
int cpl_coord = s->cpl_coords[ch][band] << 5;
for (bin = band_start; bin < band_end; bin++) {
s->fixed_coeffs[ch][bin] =
MULH(s->fixed_coeffs[CPL_CH][bin] * (1 << 4), cpl_coord);
}
if (ch == 2 && s->phase_flags[band]) {
for (bin = band_start; bin < band_end; bin++)
s->fixed_coeffs[2][bin] = -s->fixed_coeffs[2][bin];
}
}
}
bin = band_end;
}
}
typedef struct mant_groups {
int b1_mant[2];
int b2_mant[2];
int b4_mant;
int b1;
int b2;
int b4;
} mant_groups;
static void ac3_decode_transform_coeffs_ch(AC3DecodeContext *s, int ch_index, mant_groups *m)
{
int start_freq = s->start_freq[ch_index];
int end_freq = s->end_freq[ch_index];
uint8_t *baps = s->bap[ch_index];
int8_t *exps = s->dexps[ch_index];
int32_t *coeffs = s->fixed_coeffs[ch_index];
int dither = (ch_index == CPL_CH) || s->dither_flag[ch_index];
GetBitContext *gbc = &s->gbc;
int freq;
for (freq = start_freq; freq < end_freq; freq++) {
int bap = baps[freq];
int mantissa;
switch (bap) {
case 0:
if (dither)
mantissa = (((av_lfg_get(&s->dith_state)>>8)*181)>>8) - 5931008;
else
mantissa = 0;
break;
case 1:
if (m->b1) {
m->b1--;
mantissa = m->b1_mant[m->b1];
} else {
int bits = get_bits(gbc, 5);
mantissa = b1_mantissas[bits][0];
m->b1_mant[1] = b1_mantissas[bits][1];
m->b1_mant[0] = b1_mantissas[bits][2];
m->b1 = 2;
}
break;
case 2:
if (m->b2) {
m->b2--;
mantissa = m->b2_mant[m->b2];
} else {
int bits = get_bits(gbc, 7);
mantissa = b2_mantissas[bits][0];
m->b2_mant[1] = b2_mantissas[bits][1];
m->b2_mant[0] = b2_mantissas[bits][2];
m->b2 = 2;
}
break;
case 3:
mantissa = b3_mantissas[get_bits(gbc, 3)];
break;
case 4:
if (m->b4) {
m->b4 = 0;
mantissa = m->b4_mant;
} else {
int bits = get_bits(gbc, 7);
mantissa = b4_mantissas[bits][0];
m->b4_mant = b4_mantissas[bits][1];
m->b4 = 1;
}
break;
case 5:
mantissa = b5_mantissas[get_bits(gbc, 4)];
break;
default: 
if (bap > 15) {
av_log(s->avctx, AV_LOG_ERROR, "bap %d is invalid in plain AC-3\n", bap);
bap = 15;
}
mantissa = (unsigned)get_sbits(gbc, quantization_tab[bap]) << (24 - quantization_tab[bap]);
break;
}
coeffs[freq] = mantissa >> exps[freq];
}
}
static void remove_dithering(AC3DecodeContext *s) {
int ch, i;
for (ch = 1; ch <= s->fbw_channels; ch++) {
if (!s->dither_flag[ch] && s->channel_in_cpl[ch]) {
for (i = s->start_freq[CPL_CH]; i < s->end_freq[CPL_CH]; i++) {
if (!s->bap[CPL_CH][i])
s->fixed_coeffs[ch][i] = 0;
}
}
}
}
static inline void decode_transform_coeffs_ch(AC3DecodeContext *s, int blk,
int ch, mant_groups *m)
{
if (!s->channel_uses_aht[ch]) {
ac3_decode_transform_coeffs_ch(s, ch, m);
} else {
int bin;
if (CONFIG_EAC3_DECODER && !blk)
ff_eac3_decode_transform_coeffs_aht_ch(s, ch);
for (bin = s->start_freq[ch]; bin < s->end_freq[ch]; bin++) {
s->fixed_coeffs[ch][bin] = s->pre_mantissa[ch][bin][blk] >> s->dexps[ch][bin];
}
}
}
static inline void decode_transform_coeffs(AC3DecodeContext *s, int blk)
{
int ch, end;
int got_cplchan = 0;
mant_groups m;
m.b1 = m.b2 = m.b4 = 0;
for (ch = 1; ch <= s->channels; ch++) {
decode_transform_coeffs_ch(s, blk, ch, &m);
if (s->channel_in_cpl[ch]) {
if (!got_cplchan) {
decode_transform_coeffs_ch(s, blk, CPL_CH, &m);
calc_transform_coeffs_cpl(s);
got_cplchan = 1;
}
end = s->end_freq[CPL_CH];
} else {
end = s->end_freq[ch];
}
do
s->fixed_coeffs[ch][end] = 0;
while (++end < 256);
}
remove_dithering(s);
}
static void do_rematrixing(AC3DecodeContext *s)
{
int bnd, i;
int end, bndend;
end = FFMIN(s->end_freq[1], s->end_freq[2]);
for (bnd = 0; bnd < s->num_rematrixing_bands; bnd++) {
if (s->rematrixing_flags[bnd]) {
bndend = FFMIN(end, ff_ac3_rematrix_band_tab[bnd + 1]);
for (i = ff_ac3_rematrix_band_tab[bnd]; i < bndend; i++) {
int tmp0 = s->fixed_coeffs[1][i];
s->fixed_coeffs[1][i] += s->fixed_coeffs[2][i];
s->fixed_coeffs[2][i] = tmp0 - s->fixed_coeffs[2][i];
}
}
}
}
static inline void do_imdct(AC3DecodeContext *s, int channels, int offset)
{
int ch;
for (ch = 1; ch <= channels; ch++) {
if (s->block_switch[ch]) {
int i;
FFTSample *x = s->tmp_output + 128;
for (i = 0; i < 128; i++)
x[i] = s->transform_coeffs[ch][2 * i];
s->imdct_256.imdct_half(&s->imdct_256, s->tmp_output, x);
#if USE_FIXED
s->fdsp->vector_fmul_window_scaled(s->outptr[ch - 1], s->delay[ch - 1 + offset],
s->tmp_output, s->window, 128, 8);
#else
s->fdsp->vector_fmul_window(s->outptr[ch - 1], s->delay[ch - 1 + offset],
s->tmp_output, s->window, 128);
#endif
for (i = 0; i < 128; i++)
x[i] = s->transform_coeffs[ch][2 * i + 1];
s->imdct_256.imdct_half(&s->imdct_256, s->delay[ch - 1 + offset], x);
} else {
s->imdct_512.imdct_half(&s->imdct_512, s->tmp_output, s->transform_coeffs[ch]);
#if USE_FIXED
s->fdsp->vector_fmul_window_scaled(s->outptr[ch - 1], s->delay[ch - 1 + offset],
s->tmp_output, s->window, 128, 8);
#else
s->fdsp->vector_fmul_window(s->outptr[ch - 1], s->delay[ch - 1 + offset],
s->tmp_output, s->window, 128);
#endif
memcpy(s->delay[ch - 1 + offset], s->tmp_output + 128, 128 * sizeof(FFTSample));
}
}
}
static void ac3_upmix_delay(AC3DecodeContext *s)
{
int channel_data_size = sizeof(s->delay[0]);
switch (s->channel_mode) {
case AC3_CHMODE_DUALMONO:
case AC3_CHMODE_STEREO:
memcpy(s->delay[1], s->delay[0], channel_data_size);
break;
case AC3_CHMODE_2F2R:
memset(s->delay[3], 0, channel_data_size);
case AC3_CHMODE_2F1R:
memset(s->delay[2], 0, channel_data_size);
break;
case AC3_CHMODE_3F2R:
memset(s->delay[4], 0, channel_data_size);
case AC3_CHMODE_3F1R:
memset(s->delay[3], 0, channel_data_size);
case AC3_CHMODE_3F:
memcpy(s->delay[2], s->delay[1], channel_data_size);
memset(s->delay[1], 0, channel_data_size);
break;
}
}
static void decode_band_structure(GetBitContext *gbc, int blk, int eac3,
int ecpl, int start_subband, int end_subband,
const uint8_t *default_band_struct,
int *num_bands, uint8_t *band_sizes,
uint8_t *band_struct, int band_struct_size)
{
int subbnd, bnd, n_subbands, n_bands=0;
uint8_t bnd_sz[22];
n_subbands = end_subband - start_subband;
if (!blk)
memcpy(band_struct, default_band_struct, band_struct_size);
av_assert0(band_struct_size >= start_subband + n_subbands);
band_struct += start_subband + 1;
if (!eac3 || get_bits1(gbc)) {
for (subbnd = 0; subbnd < n_subbands - 1; subbnd++) {
band_struct[subbnd] = get_bits1(gbc);
}
}
if (num_bands || band_sizes ) {
n_bands = n_subbands;
bnd_sz[0] = ecpl ? 6 : 12;
for (bnd = 0, subbnd = 1; subbnd < n_subbands; subbnd++) {
int subbnd_size = (ecpl && subbnd < 4) ? 6 : 12;
if (band_struct[subbnd - 1]) {
n_bands--;
bnd_sz[bnd] += subbnd_size;
} else {
bnd_sz[++bnd] = subbnd_size;
}
}
}
if (num_bands)
*num_bands = n_bands;
if (band_sizes)
memcpy(band_sizes, bnd_sz, n_bands);
}
static inline int spx_strategy(AC3DecodeContext *s, int blk)
{
GetBitContext *bc = &s->gbc;
int fbw_channels = s->fbw_channels;
int dst_start_freq, dst_end_freq, src_start_freq,
start_subband, end_subband, ch;
if (s->channel_mode == AC3_CHMODE_MONO) {
s->channel_uses_spx[1] = 1;
} else {
for (ch = 1; ch <= fbw_channels; ch++)
s->channel_uses_spx[ch] = get_bits1(bc);
}
dst_start_freq = get_bits(bc, 2);
start_subband = get_bits(bc, 3) + 2;
if (start_subband > 7)
start_subband += start_subband - 7;
end_subband = get_bits(bc, 3) + 5;
#if USE_FIXED
s->spx_dst_end_freq = end_freq_inv_tab[end_subband-5];
#endif
if (end_subband > 7)
end_subband += end_subband - 7;
dst_start_freq = dst_start_freq * 12 + 25;
src_start_freq = start_subband * 12 + 25;
dst_end_freq = end_subband * 12 + 25;
if (start_subband >= end_subband) {
av_log(s->avctx, AV_LOG_ERROR, "invalid spectral extension "
"range (%d >= %d)\n", start_subband, end_subband);
return AVERROR_INVALIDDATA;
}
if (dst_start_freq >= src_start_freq) {
av_log(s->avctx, AV_LOG_ERROR, "invalid spectral extension "
"copy start bin (%d >= %d)\n", dst_start_freq, src_start_freq);
return AVERROR_INVALIDDATA;
}
s->spx_dst_start_freq = dst_start_freq;
s->spx_src_start_freq = src_start_freq;
if (!USE_FIXED)
s->spx_dst_end_freq = dst_end_freq;
decode_band_structure(bc, blk, s->eac3, 0,
start_subband, end_subband,
ff_eac3_default_spx_band_struct,
&s->num_spx_bands,
s->spx_band_sizes,
s->spx_band_struct, sizeof(s->spx_band_struct));
return 0;
}
static inline void spx_coordinates(AC3DecodeContext *s)
{
GetBitContext *bc = &s->gbc;
int fbw_channels = s->fbw_channels;
int ch, bnd;
for (ch = 1; ch <= fbw_channels; ch++) {
if (s->channel_uses_spx[ch]) {
if (s->first_spx_coords[ch] || get_bits1(bc)) {
INTFLOAT spx_blend;
int bin, master_spx_coord;
s->first_spx_coords[ch] = 0;
spx_blend = AC3_SPX_BLEND(get_bits(bc, 5));
master_spx_coord = get_bits(bc, 2) * 3;
bin = s->spx_src_start_freq;
for (bnd = 0; bnd < s->num_spx_bands; bnd++) {
int bandsize = s->spx_band_sizes[bnd];
int spx_coord_exp, spx_coord_mant;
INTFLOAT nratio, sblend, nblend;
#if USE_FIXED
int64_t accu = ((bin << 23) + (bandsize << 22))
* (int64_t)s->spx_dst_end_freq;
nratio = (int)(accu >> 32);
nratio -= spx_blend << 18;
if (nratio < 0) {
nblend = 0;
sblend = 0x800000;
} else if (nratio > 0x7fffff) {
nblend = 14529495; 
sblend = 0;
} else {
nblend = fixed_sqrt(nratio, 23);
accu = (int64_t)nblend * 1859775393;
nblend = (int)((accu + (1<<29)) >> 30);
sblend = fixed_sqrt(0x800000 - nratio, 23);
}
#else
float spx_coord;
nratio = ((float)((bin + (bandsize >> 1))) / s->spx_dst_end_freq) - spx_blend;
nratio = av_clipf(nratio, 0.0f, 1.0f);
nblend = sqrtf(3.0f * nratio); 
sblend = sqrtf(1.0f - nratio);
#endif
bin += bandsize;
spx_coord_exp = get_bits(bc, 4);
spx_coord_mant = get_bits(bc, 2);
if (spx_coord_exp == 15) spx_coord_mant <<= 1;
else spx_coord_mant += 4;
spx_coord_mant <<= (25 - spx_coord_exp - master_spx_coord);
#if USE_FIXED
accu = (int64_t)nblend * spx_coord_mant;
s->spx_noise_blend[ch][bnd] = (int)((accu + (1<<22)) >> 23);
accu = (int64_t)sblend * spx_coord_mant;
s->spx_signal_blend[ch][bnd] = (int)((accu + (1<<22)) >> 23);
#else
spx_coord = spx_coord_mant * (1.0f / (1 << 23));
s->spx_noise_blend [ch][bnd] = nblend * spx_coord;
s->spx_signal_blend[ch][bnd] = sblend * spx_coord;
#endif
}
}
} else {
s->first_spx_coords[ch] = 1;
}
}
}
static inline int coupling_strategy(AC3DecodeContext *s, int blk,
uint8_t *bit_alloc_stages)
{
GetBitContext *bc = &s->gbc;
int fbw_channels = s->fbw_channels;
int channel_mode = s->channel_mode;
int ch;
memset(bit_alloc_stages, 3, AC3_MAX_CHANNELS);
if (!s->eac3)
s->cpl_in_use[blk] = get_bits1(bc);
if (s->cpl_in_use[blk]) {
int cpl_start_subband, cpl_end_subband;
if (channel_mode < AC3_CHMODE_STEREO) {
av_log(s->avctx, AV_LOG_ERROR, "coupling not allowed in mono or dual-mono\n");
return AVERROR_INVALIDDATA;
}
if (s->eac3 && get_bits1(bc)) {
avpriv_request_sample(s->avctx, "Enhanced coupling");
return AVERROR_PATCHWELCOME;
}
if (s->eac3 && s->channel_mode == AC3_CHMODE_STEREO) {
s->channel_in_cpl[1] = 1;
s->channel_in_cpl[2] = 1;
} else {
for (ch = 1; ch <= fbw_channels; ch++)
s->channel_in_cpl[ch] = get_bits1(bc);
}
if (channel_mode == AC3_CHMODE_STEREO)
s->phase_flags_in_use = get_bits1(bc);
cpl_start_subband = get_bits(bc, 4);
cpl_end_subband = s->spx_in_use ? (s->spx_src_start_freq - 37) / 12 :
get_bits(bc, 4) + 3;
if (cpl_start_subband >= cpl_end_subband) {
av_log(s->avctx, AV_LOG_ERROR, "invalid coupling range (%d >= %d)\n",
cpl_start_subband, cpl_end_subband);
return AVERROR_INVALIDDATA;
}
s->start_freq[CPL_CH] = cpl_start_subband * 12 + 37;
s->end_freq[CPL_CH] = cpl_end_subband * 12 + 37;
decode_band_structure(bc, blk, s->eac3, 0, cpl_start_subband,
cpl_end_subband,
ff_eac3_default_cpl_band_struct,
&s->num_cpl_bands, s->cpl_band_sizes,
s->cpl_band_struct, sizeof(s->cpl_band_struct));
} else {
for (ch = 1; ch <= fbw_channels; ch++) {
s->channel_in_cpl[ch] = 0;
s->first_cpl_coords[ch] = 1;
}
s->first_cpl_leak = s->eac3;
s->phase_flags_in_use = 0;
}
return 0;
}
static inline int coupling_coordinates(AC3DecodeContext *s, int blk)
{
GetBitContext *bc = &s->gbc;
int fbw_channels = s->fbw_channels;
int ch, bnd;
int cpl_coords_exist = 0;
for (ch = 1; ch <= fbw_channels; ch++) {
if (s->channel_in_cpl[ch]) {
if ((s->eac3 && s->first_cpl_coords[ch]) || get_bits1(bc)) {
int master_cpl_coord, cpl_coord_exp, cpl_coord_mant;
s->first_cpl_coords[ch] = 0;
cpl_coords_exist = 1;
master_cpl_coord = 3 * get_bits(bc, 2);
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
cpl_coord_exp = get_bits(bc, 4);
cpl_coord_mant = get_bits(bc, 4);
if (cpl_coord_exp == 15)
s->cpl_coords[ch][bnd] = cpl_coord_mant << 22;
else
s->cpl_coords[ch][bnd] = (cpl_coord_mant + 16) << 21;
s->cpl_coords[ch][bnd] >>= (cpl_coord_exp + master_cpl_coord);
}
} else if (!blk) {
av_log(s->avctx, AV_LOG_ERROR, "new coupling coordinates must "
"be present in block 0\n");
return AVERROR_INVALIDDATA;
}
} else {
s->first_cpl_coords[ch] = 1;
}
}
if (s->channel_mode == AC3_CHMODE_STEREO && cpl_coords_exist) {
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
s->phase_flags[bnd] = s->phase_flags_in_use ? get_bits1(bc) : 0;
}
}
return 0;
}
static int decode_audio_block(AC3DecodeContext *s, int blk, int offset)
{
int fbw_channels = s->fbw_channels;
int channel_mode = s->channel_mode;
int i, bnd, seg, ch, ret;
int different_transforms;
int downmix_output;
int cpl_in_use;
GetBitContext *gbc = &s->gbc;
uint8_t bit_alloc_stages[AC3_MAX_CHANNELS] = { 0 };
different_transforms = 0;
if (s->block_switch_syntax) {
for (ch = 1; ch <= fbw_channels; ch++) {
s->block_switch[ch] = get_bits1(gbc);
if (ch > 1 && s->block_switch[ch] != s->block_switch[1])
different_transforms = 1;
}
}
if (s->dither_flag_syntax) {
for (ch = 1; ch <= fbw_channels; ch++) {
s->dither_flag[ch] = get_bits1(gbc);
}
}
i = !s->channel_mode;
do {
if (get_bits1(gbc)) {
int range_bits = get_bits(gbc, 8);
INTFLOAT range = AC3_RANGE(range_bits);
if (range_bits <= 127 || s->drc_scale <= 1.0)
s->dynamic_range[i] = AC3_DYNAMIC_RANGE(range);
else
s->dynamic_range[i] = range;
} else if (blk == 0) {
s->dynamic_range[i] = AC3_DYNAMIC_RANGE1;
}
} while (i--);
if (s->eac3 && (!blk || get_bits1(gbc))) {
s->spx_in_use = get_bits1(gbc);
if (s->spx_in_use) {
if ((ret = spx_strategy(s, blk)) < 0)
return ret;
}
}
if (!s->eac3 || !s->spx_in_use) {
s->spx_in_use = 0;
for (ch = 1; ch <= fbw_channels; ch++) {
s->channel_uses_spx[ch] = 0;
s->first_spx_coords[ch] = 1;
}
}
if (s->spx_in_use)
spx_coordinates(s);
if (s->eac3 ? s->cpl_strategy_exists[blk] : get_bits1(gbc)) {
if ((ret = coupling_strategy(s, blk, bit_alloc_stages)) < 0)
return ret;
} else if (!s->eac3) {
if (!blk) {
av_log(s->avctx, AV_LOG_ERROR, "new coupling strategy must "
"be present in block 0\n");
return AVERROR_INVALIDDATA;
} else {
s->cpl_in_use[blk] = s->cpl_in_use[blk-1];
}
}
cpl_in_use = s->cpl_in_use[blk];
if (cpl_in_use) {
if ((ret = coupling_coordinates(s, blk)) < 0)
return ret;
}
if (channel_mode == AC3_CHMODE_STEREO) {
if ((s->eac3 && !blk) || get_bits1(gbc)) {
s->num_rematrixing_bands = 4;
if (cpl_in_use && s->start_freq[CPL_CH] <= 61) {
s->num_rematrixing_bands -= 1 + (s->start_freq[CPL_CH] == 37);
} else if (s->spx_in_use && s->spx_src_start_freq <= 61) {
s->num_rematrixing_bands--;
}
for (bnd = 0; bnd < s->num_rematrixing_bands; bnd++)
s->rematrixing_flags[bnd] = get_bits1(gbc);
} else if (!blk) {
av_log(s->avctx, AV_LOG_WARNING, "Warning: "
"new rematrixing strategy not present in block 0\n");
s->num_rematrixing_bands = 0;
}
}
for (ch = !cpl_in_use; ch <= s->channels; ch++) {
if (!s->eac3)
s->exp_strategy[blk][ch] = get_bits(gbc, 2 - (ch == s->lfe_ch));
if (s->exp_strategy[blk][ch] != EXP_REUSE)
bit_alloc_stages[ch] = 3;
}
for (ch = 1; ch <= fbw_channels; ch++) {
s->start_freq[ch] = 0;
if (s->exp_strategy[blk][ch] != EXP_REUSE) {
int group_size;
int prev = s->end_freq[ch];
if (s->channel_in_cpl[ch])
s->end_freq[ch] = s->start_freq[CPL_CH];
else if (s->channel_uses_spx[ch])
s->end_freq[ch] = s->spx_src_start_freq;
else {
int bandwidth_code = get_bits(gbc, 6);
if (bandwidth_code > 60) {
av_log(s->avctx, AV_LOG_ERROR, "bandwidth code = %d > 60\n", bandwidth_code);
return AVERROR_INVALIDDATA;
}
s->end_freq[ch] = bandwidth_code * 3 + 73;
}
group_size = 3 << (s->exp_strategy[blk][ch] - 1);
s->num_exp_groups[ch] = (s->end_freq[ch] + group_size-4) / group_size;
if (blk > 0 && s->end_freq[ch] != prev)
memset(bit_alloc_stages, 3, AC3_MAX_CHANNELS);
}
}
if (cpl_in_use && s->exp_strategy[blk][CPL_CH] != EXP_REUSE) {
s->num_exp_groups[CPL_CH] = (s->end_freq[CPL_CH] - s->start_freq[CPL_CH]) /
(3 << (s->exp_strategy[blk][CPL_CH] - 1));
}
for (ch = !cpl_in_use; ch <= s->channels; ch++) {
if (s->exp_strategy[blk][ch] != EXP_REUSE) {
s->dexps[ch][0] = get_bits(gbc, 4) << !ch;
if (decode_exponents(s, gbc, s->exp_strategy[blk][ch],
s->num_exp_groups[ch], s->dexps[ch][0],
&s->dexps[ch][s->start_freq[ch]+!!ch])) {
return AVERROR_INVALIDDATA;
}
if (ch != CPL_CH && ch != s->lfe_ch)
skip_bits(gbc, 2); 
}
}
if (s->bit_allocation_syntax) {
if (get_bits1(gbc)) {
s->bit_alloc_params.slow_decay = ff_ac3_slow_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
s->bit_alloc_params.fast_decay = ff_ac3_fast_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
s->bit_alloc_params.slow_gain = ff_ac3_slow_gain_tab[get_bits(gbc, 2)];
s->bit_alloc_params.db_per_bit = ff_ac3_db_per_bit_tab[get_bits(gbc, 2)];
s->bit_alloc_params.floor = ff_ac3_floor_tab[get_bits(gbc, 3)];
for (ch = !cpl_in_use; ch <= s->channels; ch++)
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
} else if (!blk) {
av_log(s->avctx, AV_LOG_ERROR, "new bit allocation info must "
"be present in block 0\n");
return AVERROR_INVALIDDATA;
}
}
if (!s->eac3 || !blk) {
if (s->snr_offset_strategy && get_bits1(gbc)) {
int snr = 0;
int csnr;
csnr = (get_bits(gbc, 6) - 15) << 4;
for (i = ch = !cpl_in_use; ch <= s->channels; ch++) {
if (ch == i || s->snr_offset_strategy == 2)
snr = (csnr + get_bits(gbc, 4)) << 2;
if (blk && s->snr_offset[ch] != snr) {
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 1);
}
s->snr_offset[ch] = snr;
if (!s->eac3) {
int prev = s->fast_gain[ch];
s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
if (blk && prev != s->fast_gain[ch])
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
}
}
} else if (!s->eac3 && !blk) {
av_log(s->avctx, AV_LOG_ERROR, "new snr offsets must be present in block 0\n");
return AVERROR_INVALIDDATA;
}
}
if (s->fast_gain_syntax && get_bits1(gbc)) {
for (ch = !cpl_in_use; ch <= s->channels; ch++) {
int prev = s->fast_gain[ch];
s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
if (blk && prev != s->fast_gain[ch])
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
}
} else if (s->eac3 && !blk) {
for (ch = !cpl_in_use; ch <= s->channels; ch++)
s->fast_gain[ch] = ff_ac3_fast_gain_tab[4];
}
if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT && get_bits1(gbc)) {
skip_bits(gbc, 10); 
}
if (cpl_in_use) {
if (s->first_cpl_leak || get_bits1(gbc)) {
int fl = get_bits(gbc, 3);
int sl = get_bits(gbc, 3);
if (blk && (fl != s->bit_alloc_params.cpl_fast_leak ||
sl != s->bit_alloc_params.cpl_slow_leak)) {
bit_alloc_stages[CPL_CH] = FFMAX(bit_alloc_stages[CPL_CH], 2);
}
s->bit_alloc_params.cpl_fast_leak = fl;
s->bit_alloc_params.cpl_slow_leak = sl;
} else if (!s->eac3 && !blk) {
av_log(s->avctx, AV_LOG_ERROR, "new coupling leak info must "
"be present in block 0\n");
return AVERROR_INVALIDDATA;
}
s->first_cpl_leak = 0;
}
if (s->dba_syntax && get_bits1(gbc)) {
for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
s->dba_mode[ch] = get_bits(gbc, 2);
if (s->dba_mode[ch] == DBA_RESERVED) {
av_log(s->avctx, AV_LOG_ERROR, "delta bit allocation strategy reserved\n");
return AVERROR_INVALIDDATA;
}
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
}
for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
if (s->dba_mode[ch] == DBA_NEW) {
s->dba_nsegs[ch] = get_bits(gbc, 3) + 1;
for (seg = 0; seg < s->dba_nsegs[ch]; seg++) {
s->dba_offsets[ch][seg] = get_bits(gbc, 5);
s->dba_lengths[ch][seg] = get_bits(gbc, 4);
s->dba_values[ch][seg] = get_bits(gbc, 3);
}
bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
}
}
} else if (blk == 0) {
for (ch = 0; ch <= s->channels; ch++) {
s->dba_mode[ch] = DBA_NONE;
}
}
for (ch = !cpl_in_use; ch <= s->channels; ch++) {
if (bit_alloc_stages[ch] > 2) {
ff_ac3_bit_alloc_calc_psd(s->dexps[ch],
s->start_freq[ch], s->end_freq[ch],
s->psd[ch], s->band_psd[ch]);
}
if (bit_alloc_stages[ch] > 1) {
if (ff_ac3_bit_alloc_calc_mask(&s->bit_alloc_params, s->band_psd[ch],
s->start_freq[ch], s->end_freq[ch],
s->fast_gain[ch], (ch == s->lfe_ch),
s->dba_mode[ch], s->dba_nsegs[ch],
s->dba_offsets[ch], s->dba_lengths[ch],
s->dba_values[ch], s->mask[ch])) {
av_log(s->avctx, AV_LOG_ERROR, "error in bit allocation\n");
return AVERROR_INVALIDDATA;
}
}
if (bit_alloc_stages[ch] > 0) {
const uint8_t *bap_tab = s->channel_uses_aht[ch] ?
ff_eac3_hebap_tab : ff_ac3_bap_tab;
s->ac3dsp.bit_alloc_calc_bap(s->mask[ch], s->psd[ch],
s->start_freq[ch], s->end_freq[ch],
s->snr_offset[ch],
s->bit_alloc_params.floor,
bap_tab, s->bap[ch]);
}
}
if (s->skip_syntax && get_bits1(gbc)) {
int skipl = get_bits(gbc, 9);
skip_bits_long(gbc, 8 * skipl);
}
decode_transform_coeffs(s, blk);
if (s->channel_mode == AC3_CHMODE_STEREO)
do_rematrixing(s);
for (ch = 1; ch <= s->channels; ch++) {
int audio_channel = 0;
INTFLOAT gain;
if (s->channel_mode == AC3_CHMODE_DUALMONO && ch <= 2)
audio_channel = 2-ch;
if (s->heavy_compression && s->compression_exists[audio_channel])
gain = s->heavy_dynamic_range[audio_channel];
else
gain = s->dynamic_range[audio_channel];
#if USE_FIXED
scale_coefs(s->transform_coeffs[ch], s->fixed_coeffs[ch], gain, 256);
#else
if (s->target_level != 0)
gain = gain * s->level_gain[audio_channel];
gain *= 1.0 / 4194304.0f;
s->fmt_conv.int32_to_float_fmul_scalar(s->transform_coeffs[ch],
s->fixed_coeffs[ch], gain, 256);
#endif
}
if (CONFIG_EAC3_DECODER && s->spx_in_use) {
ff_eac3_apply_spectral_extension(s);
}
downmix_output = s->channels != s->out_channels &&
!((s->output_mode & AC3_OUTPUT_LFEON) &&
s->fbw_channels == s->out_channels);
if (different_transforms) {
if (s->downmixed) {
s->downmixed = 0;
ac3_upmix_delay(s);
}
do_imdct(s, s->channels, offset);
if (downmix_output) {
#if USE_FIXED
ac3_downmix_c_fixed16(s->outptr, s->downmix_coeffs,
s->out_channels, s->fbw_channels, 256);
#else
ff_ac3dsp_downmix(&s->ac3dsp, s->outptr, s->downmix_coeffs,
s->out_channels, s->fbw_channels, 256);
#endif
}
} else {
if (downmix_output) {
AC3_RENAME(ff_ac3dsp_downmix)(&s->ac3dsp, s->xcfptr + 1, s->downmix_coeffs,
s->out_channels, s->fbw_channels, 256);
}
if (downmix_output && !s->downmixed) {
s->downmixed = 1;
AC3_RENAME(ff_ac3dsp_downmix)(&s->ac3dsp, s->dlyptr, s->downmix_coeffs,
s->out_channels, s->fbw_channels, 128);
}
do_imdct(s, s->out_channels, offset);
}
return 0;
}
static int ac3_decode_frame(AVCodecContext * avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int buf_size, full_buf_size = avpkt->size;
AC3DecodeContext *s = avctx->priv_data;
int blk, ch, err, offset, ret;
int i;
int skip = 0, got_independent_frame = 0;
const uint8_t *channel_map;
uint8_t extended_channel_map[EAC3_MAX_CHANNELS];
const SHORTFLOAT *output[AC3_MAX_CHANNELS];
enum AVMatrixEncoding matrix_encoding;
AVDownmixInfo *downmix_info;
s->superframe_size = 0;
buf_size = full_buf_size;
for (i = 1; i < buf_size; i += 2) {
if (buf[i] == 0x77 || buf[i] == 0x0B) {
if ((buf[i] ^ buf[i-1]) == (0x77 ^ 0x0B)) {
i--;
break;
} else if ((buf[i] ^ buf[i+1]) == (0x77 ^ 0x0B)) {
break;
}
}
}
if (i >= buf_size)
return AVERROR_INVALIDDATA;
if (i > 10)
return i;
buf += i;
buf_size -= i;
if (buf_size >= 2 && AV_RB16(buf) == 0x770B) {
int cnt = FFMIN(buf_size, AC3_FRAME_BUFFER_SIZE) >> 1;
s->bdsp.bswap16_buf((uint16_t *) s->input_buffer,
(const uint16_t *) buf, cnt);
} else
memcpy(s->input_buffer, buf, FFMIN(buf_size, AC3_FRAME_BUFFER_SIZE));
if (s->consistent_noise_generation)
av_lfg_init_from_data(&s->dith_state, s->input_buffer, FFMIN(buf_size, AC3_FRAME_BUFFER_SIZE));
buf = s->input_buffer;
dependent_frame:
if ((ret = init_get_bits8(&s->gbc, buf, buf_size)) < 0)
return ret;
err = parse_frame_header(s);
if (err) {
switch (err) {
case AAC_AC3_PARSE_ERROR_SYNC:
av_log(avctx, AV_LOG_ERROR, "frame sync error\n");
return AVERROR_INVALIDDATA;
case AAC_AC3_PARSE_ERROR_BSID:
av_log(avctx, AV_LOG_ERROR, "invalid bitstream id\n");
break;
case AAC_AC3_PARSE_ERROR_SAMPLE_RATE:
av_log(avctx, AV_LOG_ERROR, "invalid sample rate\n");
break;
case AAC_AC3_PARSE_ERROR_FRAME_SIZE:
av_log(avctx, AV_LOG_ERROR, "invalid frame size\n");
break;
case AAC_AC3_PARSE_ERROR_FRAME_TYPE:
if (s->substreamid) {
av_log(avctx, AV_LOG_DEBUG,
"unsupported substream %d: skipping frame\n",
s->substreamid);
*got_frame_ptr = 0;
return buf_size;
} else {
av_log(avctx, AV_LOG_ERROR, "invalid frame type\n");
}
break;
case AAC_AC3_PARSE_ERROR_CRC:
case AAC_AC3_PARSE_ERROR_CHANNEL_CFG:
break;
default: 
*got_frame_ptr = 0;
return err;
}
} else {
if (s->frame_size > buf_size) {
av_log(avctx, AV_LOG_ERROR, "incomplete frame\n");
err = AAC_AC3_PARSE_ERROR_FRAME_SIZE;
} else if (avctx->err_recognition & (AV_EF_CRCCHECK|AV_EF_CAREFUL)) {
if (av_crc(av_crc_get_table(AV_CRC_16_ANSI), 0, &buf[2],
s->frame_size - 2)) {
av_log(avctx, AV_LOG_ERROR, "frame CRC mismatch\n");
if (avctx->err_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
err = AAC_AC3_PARSE_ERROR_CRC;
}
}
}
if (s->frame_type == EAC3_FRAME_TYPE_DEPENDENT && !got_independent_frame) {
av_log(avctx, AV_LOG_WARNING, "Ignoring dependent frame without independent frame.\n");
*got_frame_ptr = 0;
return FFMIN(full_buf_size, s->frame_size);
}
if (!err || (s->channels && s->out_channels != s->channels)) {
s->out_channels = s->channels;
s->output_mode = s->channel_mode;
if (s->lfe_on)
s->output_mode |= AC3_OUTPUT_LFEON;
if (s->channels > 1 &&
avctx->request_channel_layout == AV_CH_LAYOUT_MONO) {
s->out_channels = 1;
s->output_mode = AC3_CHMODE_MONO;
} else if (s->channels > 2 &&
avctx->request_channel_layout == AV_CH_LAYOUT_STEREO) {
s->out_channels = 2;
s->output_mode = AC3_CHMODE_STEREO;
}
s->loro_center_mix_level = gain_levels[s-> center_mix_level];
s->loro_surround_mix_level = gain_levels[s->surround_mix_level];
s->ltrt_center_mix_level = LEVEL_MINUS_3DB;
s->ltrt_surround_mix_level = LEVEL_MINUS_3DB;
if (s->channels != s->out_channels && !((s->output_mode & AC3_OUTPUT_LFEON) &&
s->fbw_channels == s->out_channels)) {
if ((ret = set_downmix_coeffs(s)) < 0) {
av_log(avctx, AV_LOG_ERROR, "error setting downmix coeffs\n");
return ret;
}
}
} else if (!s->channels) {
av_log(avctx, AV_LOG_ERROR, "unable to determine channel mode\n");
return AVERROR_INVALIDDATA;
}
avctx->channels = s->out_channels;
avctx->channel_layout = avpriv_ac3_channel_layout_tab[s->output_mode & ~AC3_OUTPUT_LFEON];
if (s->output_mode & AC3_OUTPUT_LFEON)
avctx->channel_layout |= AV_CH_LOW_FREQUENCY;
avctx->audio_service_type = s->bitstream_mode;
if (s->bitstream_mode == 0x7 && s->channels > 1)
avctx->audio_service_type = AV_AUDIO_SERVICE_TYPE_KARAOKE;
channel_map = ff_ac3_dec_channel_map[s->output_mode & ~AC3_OUTPUT_LFEON][s->lfe_on];
offset = s->frame_type == EAC3_FRAME_TYPE_DEPENDENT ? AC3_MAX_CHANNELS : 0;
for (ch = 0; ch < AC3_MAX_CHANNELS; ch++) {
output[ch] = s->output[ch + offset];
s->outptr[ch] = s->output[ch + offset];
}
for (ch = 0; ch < s->channels; ch++) {
if (ch < s->out_channels)
s->outptr[channel_map[ch]] = s->output_buffer[ch + offset];
}
for (blk = 0; blk < s->num_blocks; blk++) {
if (!err && decode_audio_block(s, blk, offset)) {
av_log(avctx, AV_LOG_ERROR, "error decoding the audio block\n");
err = 1;
}
if (err)
for (ch = 0; ch < s->out_channels; ch++)
memcpy(s->output_buffer[ch + offset] + AC3_BLOCK_SIZE*blk, output[ch], AC3_BLOCK_SIZE*sizeof(SHORTFLOAT));
for (ch = 0; ch < s->out_channels; ch++)
output[ch] = s->outptr[channel_map[ch]];
for (ch = 0; ch < s->out_channels; ch++) {
if (!ch || channel_map[ch])
s->outptr[channel_map[ch]] += AC3_BLOCK_SIZE;
}
}
for (ch = 0; ch < s->out_channels; ch++)
memcpy(s->output[ch + offset], output[ch], AC3_BLOCK_SIZE*sizeof(SHORTFLOAT));
if (buf_size > s->frame_size) {
AC3HeaderInfo hdr;
int err;
if (buf_size - s->frame_size <= 16) {
skip = buf_size - s->frame_size;
goto skip;
}
if ((ret = init_get_bits8(&s->gbc, buf + s->frame_size, buf_size - s->frame_size)) < 0)
return ret;
err = ff_ac3_parse_header(&s->gbc, &hdr);
if (err)
return err;
if (hdr.frame_type == EAC3_FRAME_TYPE_DEPENDENT) {
if (hdr.num_blocks != s->num_blocks || s->sample_rate != hdr.sample_rate) {
av_log(avctx, AV_LOG_WARNING, "Ignoring non-compatible dependent frame.\n");
} else {
buf += s->frame_size;
buf_size -= s->frame_size;
s->prev_output_mode = s->output_mode;
s->prev_bit_rate = s->bit_rate;
got_independent_frame = 1;
goto dependent_frame;
}
}
}
skip:
frame->decode_error_flags = err ? FF_DECODE_ERROR_INVALID_BITSTREAM : 0;
if (!err) {
avctx->sample_rate = s->sample_rate;
avctx->bit_rate = s->bit_rate + s->prev_bit_rate;
}
for (ch = 0; ch < EAC3_MAX_CHANNELS; ch++)
extended_channel_map[ch] = ch;
if (s->frame_type == EAC3_FRAME_TYPE_DEPENDENT) {
uint64_t ich_layout = avpriv_ac3_channel_layout_tab[s->prev_output_mode & ~AC3_OUTPUT_LFEON];
int channel_map_size = ff_ac3_channels_tab[s->output_mode & ~AC3_OUTPUT_LFEON] + s->lfe_on;
uint64_t channel_layout;
int extend = 0;
if (s->prev_output_mode & AC3_OUTPUT_LFEON)
ich_layout |= AV_CH_LOW_FREQUENCY;
channel_layout = ich_layout;
for (ch = 0; ch < 16; ch++) {
if (s->channel_map & (1 << (EAC3_MAX_CHANNELS - ch - 1))) {
channel_layout |= ff_eac3_custom_channel_map_locations[ch][1];
}
}
if (av_get_channel_layout_nb_channels(channel_layout) > EAC3_MAX_CHANNELS) {
av_log(avctx, AV_LOG_ERROR, "Too many channels (%d) coded\n",
av_get_channel_layout_nb_channels(channel_layout));
return AVERROR_INVALIDDATA;
}
avctx->channel_layout = channel_layout;
avctx->channels = av_get_channel_layout_nb_channels(channel_layout);
for (ch = 0; ch < EAC3_MAX_CHANNELS; ch++) {
if (s->channel_map & (1 << (EAC3_MAX_CHANNELS - ch - 1))) {
if (ff_eac3_custom_channel_map_locations[ch][0]) {
int index = av_get_channel_layout_channel_index(channel_layout,
ff_eac3_custom_channel_map_locations[ch][1]);
if (index < 0)
return AVERROR_INVALIDDATA;
if (extend >= channel_map_size)
return AVERROR_INVALIDDATA;
extended_channel_map[index] = offset + channel_map[extend++];
} else {
int i;
for (i = 0; i < 64; i++) {
if ((1ULL << i) & ff_eac3_custom_channel_map_locations[ch][1]) {
int index = av_get_channel_layout_channel_index(channel_layout,
1ULL << i);
if (index < 0)
return AVERROR_INVALIDDATA;
if (extend >= channel_map_size)
return AVERROR_INVALIDDATA;
extended_channel_map[index] = offset + channel_map[extend++];
}
}
}
}
}
}
frame->nb_samples = s->num_blocks * AC3_BLOCK_SIZE;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
for (ch = 0; ch < avctx->channels; ch++) {
int map = extended_channel_map[ch];
av_assert0(ch>=AV_NUM_DATA_POINTERS || frame->extended_data[ch] == frame->data[ch]);
memcpy((SHORTFLOAT *)frame->extended_data[ch],
s->output_buffer[map],
s->num_blocks * AC3_BLOCK_SIZE * sizeof(SHORTFLOAT));
}
matrix_encoding = AV_MATRIX_ENCODING_NONE;
if (s->channel_mode == AC3_CHMODE_STEREO &&
s->channel_mode == (s->output_mode & ~AC3_OUTPUT_LFEON)) {
if (s->dolby_surround_mode == AC3_DSURMOD_ON)
matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
else if (s->dolby_headphone_mode == AC3_DHEADPHONMOD_ON)
matrix_encoding = AV_MATRIX_ENCODING_DOLBYHEADPHONE;
} else if (s->channel_mode >= AC3_CHMODE_2F2R &&
s->channel_mode == (s->output_mode & ~AC3_OUTPUT_LFEON)) {
switch (s->dolby_surround_ex_mode) {
case AC3_DSUREXMOD_ON: 
matrix_encoding = AV_MATRIX_ENCODING_DOLBYEX;
break;
case AC3_DSUREXMOD_PLIIZ:
matrix_encoding = AV_MATRIX_ENCODING_DPLIIZ;
break;
default: 
break;
}
}
if ((ret = ff_side_data_update_matrix_encoding(frame, matrix_encoding)) < 0)
return ret;
if ((downmix_info = av_downmix_info_update_side_data(frame))) {
switch (s->preferred_downmix) {
case AC3_DMIXMOD_LTRT:
downmix_info->preferred_downmix_type = AV_DOWNMIX_TYPE_LTRT;
break;
case AC3_DMIXMOD_LORO:
downmix_info->preferred_downmix_type = AV_DOWNMIX_TYPE_LORO;
break;
case AC3_DMIXMOD_DPLII:
downmix_info->preferred_downmix_type = AV_DOWNMIX_TYPE_DPLII;
break;
default:
downmix_info->preferred_downmix_type = AV_DOWNMIX_TYPE_UNKNOWN;
break;
}
downmix_info->center_mix_level = gain_levels[s-> center_mix_level];
downmix_info->center_mix_level_ltrt = gain_levels[s-> center_mix_level_ltrt];
downmix_info->surround_mix_level = gain_levels[s-> surround_mix_level];
downmix_info->surround_mix_level_ltrt = gain_levels[s->surround_mix_level_ltrt];
if (s->lfe_mix_level_exists)
downmix_info->lfe_mix_level = gain_levels_lfe[s->lfe_mix_level];
else
downmix_info->lfe_mix_level = 0.0; 
} else
return AVERROR(ENOMEM);
*got_frame_ptr = 1;
if (!s->superframe_size)
return FFMIN(full_buf_size, s->frame_size + skip);
return FFMIN(full_buf_size, s->superframe_size + skip);
}
static av_cold int ac3_decode_end(AVCodecContext *avctx)
{
AC3DecodeContext *s = avctx->priv_data;
ff_mdct_end(&s->imdct_512);
ff_mdct_end(&s->imdct_256);
av_freep(&s->fdsp);
av_freep(&s->downmix_coeffs[0]);
return 0;
}
#define OFFSET(x) offsetof(AC3DecodeContext, x)
#define PAR (AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_AUDIO_PARAM)
