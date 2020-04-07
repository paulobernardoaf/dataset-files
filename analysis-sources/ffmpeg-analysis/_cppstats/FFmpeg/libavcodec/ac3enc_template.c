#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/internal.h"
#include "audiodsp.h"
#include "internal.h"
#include "ac3enc.h"
#include "eac3enc.h"
int AC3_NAME(allocate_sample_buffers)(AC3EncodeContext *s)
{
int ch;
FF_ALLOC_OR_GOTO(s->avctx, s->windowed_samples, AC3_WINDOW_SIZE *
sizeof(*s->windowed_samples), alloc_fail);
FF_ALLOC_ARRAY_OR_GOTO(s->avctx, s->planar_samples, s->channels, sizeof(*s->planar_samples),
alloc_fail);
for (ch = 0; ch < s->channels; ch++) {
FF_ALLOCZ_OR_GOTO(s->avctx, s->planar_samples[ch],
(AC3_FRAME_SIZE+AC3_BLOCK_SIZE) * sizeof(**s->planar_samples),
alloc_fail);
}
return 0;
alloc_fail:
return AVERROR(ENOMEM);
}
static void copy_input_samples(AC3EncodeContext *s, SampleType **samples)
{
int ch;
for (ch = 0; ch < s->channels; ch++) {
memcpy(&s->planar_samples[ch][0], &s->planar_samples[ch][AC3_BLOCK_SIZE * s->num_blocks],
AC3_BLOCK_SIZE * sizeof(s->planar_samples[0][0]));
memcpy(&s->planar_samples[ch][AC3_BLOCK_SIZE],
samples[s->channel_map[ch]],
AC3_BLOCK_SIZE * s->num_blocks * sizeof(s->planar_samples[0][0]));
}
}
static void apply_mdct(AC3EncodeContext *s)
{
int blk, ch;
for (ch = 0; ch < s->channels; ch++) {
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
const SampleType *input_samples = &s->planar_samples[ch][blk * AC3_BLOCK_SIZE];
#if CONFIG_AC3ENC_FLOAT
s->fdsp->vector_fmul(s->windowed_samples, input_samples,
s->mdct_window, AC3_WINDOW_SIZE);
#else
s->ac3dsp.apply_window_int16(s->windowed_samples, input_samples,
s->mdct_window, AC3_WINDOW_SIZE);
if (s->fixed_point)
block->coeff_shift[ch+1] = normalize_samples(s);
#endif
s->mdct.mdct_calcw(&s->mdct, block->mdct_coef[ch+1],
s->windowed_samples);
}
}
}
static void apply_channel_coupling(AC3EncodeContext *s)
{
LOCAL_ALIGNED_16(CoefType, cpl_coords, [AC3_MAX_BLOCKS], [AC3_MAX_CHANNELS][16]);
#if CONFIG_AC3ENC_FLOAT
LOCAL_ALIGNED_16(int32_t, fixed_cpl_coords, [AC3_MAX_BLOCKS], [AC3_MAX_CHANNELS][16]);
#else
int32_t (*fixed_cpl_coords)[AC3_MAX_CHANNELS][16] = cpl_coords;
#endif
int av_uninit(blk), ch, bnd, i, j;
CoefSumType energy[AC3_MAX_BLOCKS][AC3_MAX_CHANNELS][16] = {{{0}}};
int cpl_start, num_cpl_coefs;
memset(cpl_coords, 0, AC3_MAX_BLOCKS * sizeof(*cpl_coords));
#if CONFIG_AC3ENC_FLOAT
memset(fixed_cpl_coords, 0, AC3_MAX_BLOCKS * sizeof(*cpl_coords));
#endif
cpl_start = s->start_freq[CPL_CH] - 1;
num_cpl_coefs = FFALIGN(s->num_cpl_subbands * 12 + 1, 32);
cpl_start = FFMIN(256, cpl_start + num_cpl_coefs) - num_cpl_coefs;
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
CoefType *cpl_coef = &block->mdct_coef[CPL_CH][cpl_start];
if (!block->cpl_in_use)
continue;
memset(cpl_coef, 0, num_cpl_coefs * sizeof(*cpl_coef));
for (ch = 1; ch <= s->fbw_channels; ch++) {
CoefType *ch_coef = &block->mdct_coef[ch][cpl_start];
if (!block->channel_in_cpl[ch])
continue;
for (i = 0; i < num_cpl_coefs; i++)
cpl_coef[i] += ch_coef[i];
}
clip_coefficients(&s->adsp, cpl_coef, num_cpl_coefs);
}
bnd = 0;
i = s->start_freq[CPL_CH];
while (i < s->cpl_end_freq) {
int band_size = s->cpl_band_sizes[bnd];
for (ch = CPL_CH; ch <= s->fbw_channels; ch++) {
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
if (!block->cpl_in_use || (ch > CPL_CH && !block->channel_in_cpl[ch]))
continue;
for (j = 0; j < band_size; j++) {
CoefType v = block->mdct_coef[ch][i+j];
MAC_COEF(energy[blk][ch][bnd], v, v);
}
}
}
i += band_size;
bnd++;
}
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
if (!block->cpl_in_use)
continue;
for (ch = 1; ch <= s->fbw_channels; ch++) {
if (!block->channel_in_cpl[ch])
continue;
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
cpl_coords[blk][ch][bnd] = calc_cpl_coord(energy[blk][ch][bnd],
energy[blk][CPL_CH][bnd]);
}
}
}
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
AC3Block *block0 = blk ? &s->blocks[blk-1] : NULL;
memset(block->new_cpl_coords, 0, sizeof(block->new_cpl_coords));
if (block->cpl_in_use) {
if (blk == 0 || !block0->cpl_in_use) {
for (ch = 1; ch <= s->fbw_channels; ch++)
block->new_cpl_coords[ch] = 1;
} else {
for (ch = 1; ch <= s->fbw_channels; ch++) {
if (!block->channel_in_cpl[ch])
continue;
if (!block0->channel_in_cpl[ch]) {
block->new_cpl_coords[ch] = 1;
} else {
CoefSumType coord_diff = 0;
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
coord_diff += FFABS(cpl_coords[blk-1][ch][bnd] -
cpl_coords[blk ][ch][bnd]);
}
coord_diff /= s->num_cpl_bands;
if (coord_diff > NEW_CPL_COORD_THRESHOLD)
block->new_cpl_coords[ch] = 1;
}
}
}
}
}
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
blk = 0;
while (blk < s->num_blocks) {
int av_uninit(blk1);
AC3Block *block = &s->blocks[blk];
if (!block->cpl_in_use) {
blk++;
continue;
}
for (ch = 1; ch <= s->fbw_channels; ch++) {
CoefSumType energy_ch, energy_cpl;
if (!block->channel_in_cpl[ch])
continue;
energy_cpl = energy[blk][CPL_CH][bnd];
energy_ch = energy[blk][ch][bnd];
blk1 = blk+1;
while (blk1 < s->num_blocks && !s->blocks[blk1].new_cpl_coords[ch]) {
if (s->blocks[blk1].cpl_in_use) {
energy_cpl += energy[blk1][CPL_CH][bnd];
energy_ch += energy[blk1][ch][bnd];
}
blk1++;
}
cpl_coords[blk][ch][bnd] = calc_cpl_coord(energy_ch, energy_cpl);
}
blk = blk1;
}
}
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
if (!block->cpl_in_use)
continue;
#if CONFIG_AC3ENC_FLOAT
s->ac3dsp.float_to_fixed24(fixed_cpl_coords[blk][1],
cpl_coords[blk][1],
s->fbw_channels * 16);
#endif
s->ac3dsp.extract_exponents(block->cpl_coord_exp[1],
fixed_cpl_coords[blk][1],
s->fbw_channels * 16);
for (ch = 1; ch <= s->fbw_channels; ch++) {
int bnd, min_exp, max_exp, master_exp;
if (!block->new_cpl_coords[ch])
continue;
min_exp = max_exp = block->cpl_coord_exp[ch][0];
for (bnd = 1; bnd < s->num_cpl_bands; bnd++) {
int exp = block->cpl_coord_exp[ch][bnd];
min_exp = FFMIN(exp, min_exp);
max_exp = FFMAX(exp, max_exp);
}
master_exp = ((max_exp - 15) + 2) / 3;
master_exp = FFMAX(master_exp, 0);
while (min_exp < master_exp * 3)
master_exp--;
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
block->cpl_coord_exp[ch][bnd] = av_clip(block->cpl_coord_exp[ch][bnd] -
master_exp * 3, 0, 15);
}
block->cpl_master_exp[ch] = master_exp;
for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
int cpl_exp = block->cpl_coord_exp[ch][bnd];
int cpl_mant = (fixed_cpl_coords[blk][ch][bnd] << (5 + cpl_exp + master_exp * 3)) >> 24;
if (cpl_exp == 15)
cpl_mant >>= 1;
else
cpl_mant -= 16;
block->cpl_coord_mant[ch][bnd] = cpl_mant;
}
}
}
if (CONFIG_EAC3_ENCODER && s->eac3)
ff_eac3_set_cpl_states(s);
}
static void compute_rematrixing_strategy(AC3EncodeContext *s)
{
int nb_coefs;
int blk, bnd;
AC3Block *block, *block0 = NULL;
if (s->channel_mode != AC3_CHMODE_STEREO)
return;
for (blk = 0; blk < s->num_blocks; blk++) {
block = &s->blocks[blk];
block->new_rematrixing_strategy = !blk;
block->num_rematrixing_bands = 4;
if (block->cpl_in_use) {
block->num_rematrixing_bands -= (s->start_freq[CPL_CH] <= 61);
block->num_rematrixing_bands -= (s->start_freq[CPL_CH] == 37);
if (blk && block->num_rematrixing_bands != block0->num_rematrixing_bands)
block->new_rematrixing_strategy = 1;
}
nb_coefs = FFMIN(block->end_freq[1], block->end_freq[2]);
if (!s->rematrixing_enabled) {
block0 = block;
continue;
}
for (bnd = 0; bnd < block->num_rematrixing_bands; bnd++) {
int start = ff_ac3_rematrix_band_tab[bnd];
int end = FFMIN(nb_coefs, ff_ac3_rematrix_band_tab[bnd+1]);
CoefSumType sum[4];
sum_square_butterfly(s, sum, block->mdct_coef[1] + start,
block->mdct_coef[2] + start, end - start);
if (FFMIN(sum[2], sum[3]) < FFMIN(sum[0], sum[1]))
block->rematrixing_flags[bnd] = 1;
else
block->rematrixing_flags[bnd] = 0;
if (blk &&
block->rematrixing_flags[bnd] != block0->rematrixing_flags[bnd]) {
block->new_rematrixing_strategy = 1;
}
}
block0 = block;
}
}
int AC3_NAME(encode_frame)(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr)
{
AC3EncodeContext *s = avctx->priv_data;
int ret;
if (s->options.allow_per_frame_metadata) {
ret = ff_ac3_validate_metadata(s);
if (ret)
return ret;
}
if (s->bit_alloc.sr_code == 1 || s->eac3)
ff_ac3_adjust_frame_size(s);
copy_input_samples(s, (SampleType **)frame->extended_data);
apply_mdct(s);
if (s->fixed_point)
scale_coefficients(s);
clip_coefficients(&s->adsp, s->blocks[0].mdct_coef[1],
AC3_MAX_COEFS * s->num_blocks * s->channels);
s->cpl_on = s->cpl_enabled;
ff_ac3_compute_coupling_strategy(s);
if (s->cpl_on)
apply_channel_coupling(s);
compute_rematrixing_strategy(s);
if (!s->fixed_point)
scale_coefficients(s);
ff_ac3_apply_rematrixing(s);
ff_ac3_process_exponents(s);
ret = ff_ac3_compute_bit_allocation(s);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "Bit allocation failed. Try increasing the bitrate.\n");
return ret;
}
ff_ac3_group_exponents(s);
ff_ac3_quantize_mantissas(s);
if ((ret = ff_alloc_packet2(avctx, avpkt, s->frame_size, 0)) < 0)
return ret;
ff_ac3_output_frame(s, avpkt->data);
if (frame->pts != AV_NOPTS_VALUE)
avpkt->pts = frame->pts - ff_samples_to_time_base(avctx, avctx->initial_padding);
*got_packet_ptr = 1;
return 0;
}
