



















#include "dcadec.h"
#include "dcadata.h"
#include "dcamath.h"
#include "dca_syncwords.h"
#include "unary.h"

static int get_linear(GetBitContext *gb, int n)
{
unsigned int v = get_bits_long(gb, n);
return (v >> 1) ^ -(v & 1);
}

static int get_rice_un(GetBitContext *gb, int k)
{
unsigned int v = get_unary(gb, 1, get_bits_left(gb));
return (v << k) | get_bits_long(gb, k);
}

static int get_rice(GetBitContext *gb, int k)
{
unsigned int v = get_rice_un(gb, k);
return (v >> 1) ^ -(v & 1);
}

static void get_array(GetBitContext *gb, int32_t *array, int size, int n)
{
int i;

for (i = 0; i < size; i++)
array[i] = get_bits(gb, n);
}

static void get_linear_array(GetBitContext *gb, int32_t *array, int size, int n)
{
int i;

if (n == 0)
memset(array, 0, sizeof(*array) * size);
else for (i = 0; i < size; i++)
array[i] = get_linear(gb, n);
}

static void get_rice_array(GetBitContext *gb, int32_t *array, int size, int k)
{
int i;

for (i = 0; i < size; i++)
array[i] = get_rice(gb, k);
}

static int parse_dmix_coeffs(DCAXllDecoder *s, DCAXllChSet *c)
{

int m = c->primary_chset ? ff_dca_dmix_primary_nch[c->dmix_type] : c->hier_ofs;
int i, j, *coeff_ptr = c->dmix_coeff;

for (i = 0; i < m; i++) {
int code, sign, coeff, scale, scale_inv = 0;
unsigned int index;


if (!c->primary_chset) {
code = get_bits(&s->gb, 9);
sign = (code >> 8) - 1;
index = (code & 0xff) - FF_DCA_DMIXTABLE_OFFSET;
if (index >= FF_DCA_INV_DMIXTABLE_SIZE) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL downmix scale index\n");
return AVERROR_INVALIDDATA;
}
scale = ff_dca_dmixtable[index + FF_DCA_DMIXTABLE_OFFSET];
scale_inv = ff_dca_inv_dmixtable[index];
c->dmix_scale[i] = (scale ^ sign) - sign;
c->dmix_scale_inv[i] = (scale_inv ^ sign) - sign;
}


for (j = 0; j < c->nchannels; j++) {
code = get_bits(&s->gb, 9);
sign = (code >> 8) - 1;
index = code & 0xff;
if (index >= FF_DCA_DMIXTABLE_SIZE) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL downmix coefficient index\n");
return AVERROR_INVALIDDATA;
}
coeff = ff_dca_dmixtable[index];
if (!c->primary_chset)

coeff = mul16(scale_inv, coeff);
*coeff_ptr++ = (coeff ^ sign) - sign;
}
}

return 0;
}

static int chs_parse_header(DCAXllDecoder *s, DCAXllChSet *c, DCAExssAsset *asset)
{
int i, j, k, ret, band, header_size, header_pos = get_bits_count(&s->gb);
DCAXllChSet *p = &s->chset[0];
DCAXllBand *b;


header_size = get_bits(&s->gb, 10) + 1;


if (ff_dca_check_crc(s->avctx, &s->gb, header_pos, header_pos + header_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL sub-header checksum\n");
return AVERROR_INVALIDDATA;
}


c->nchannels = get_bits(&s->gb, 4) + 1;
if (c->nchannels > DCA_XLL_CHANNELS_MAX) {
avpriv_request_sample(s->avctx, "%d XLL channels", c->nchannels);
return AVERROR_PATCHWELCOME;
}


c->residual_encode = get_bits(&s->gb, c->nchannels);


c->pcm_bit_res = get_bits(&s->gb, 5) + 1;


c->storage_bit_res = get_bits(&s->gb, 5) + 1;
if (c->storage_bit_res != 16 && c->storage_bit_res != 20 && c->storage_bit_res != 24) {
avpriv_request_sample(s->avctx, "%d-bit XLL storage resolution", c->storage_bit_res);
return AVERROR_PATCHWELCOME;
}

if (c->pcm_bit_res > c->storage_bit_res) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid PCM bit resolution for XLL channel set (%d > %d)\n", c->pcm_bit_res, c->storage_bit_res);
return AVERROR_INVALIDDATA;
}


c->freq = ff_dca_sampling_freqs[get_bits(&s->gb, 4)];
if (c->freq > 192000) {
avpriv_request_sample(s->avctx, "%d Hz XLL sampling frequency", c->freq);
return AVERROR_PATCHWELCOME;
}


if (get_bits(&s->gb, 2)) {
avpriv_request_sample(s->avctx, "XLL sampling frequency modifier");
return AVERROR_PATCHWELCOME;
}


if (get_bits(&s->gb, 2)) {
avpriv_request_sample(s->avctx, "XLL replacement set");
return AVERROR_PATCHWELCOME;
}

if (asset->one_to_one_map_ch_to_spkr) {

c->primary_chset = get_bits1(&s->gb);
if (c->primary_chset != (c == p)) {
av_log(s->avctx, AV_LOG_ERROR, "The first (and only) XLL channel set must be primary\n");
return AVERROR_INVALIDDATA;
}


c->dmix_coeffs_present = get_bits1(&s->gb);


c->dmix_embedded = c->dmix_coeffs_present && get_bits1(&s->gb);


if (c->dmix_coeffs_present && c->primary_chset) {
c->dmix_type = get_bits(&s->gb, 3);
if (c->dmix_type >= DCA_DMIX_TYPE_COUNT) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL primary channel set downmix type\n");
return AVERROR_INVALIDDATA;
}
}


c->hier_chset = get_bits1(&s->gb);
if (!c->hier_chset && s->nchsets != 1) {
avpriv_request_sample(s->avctx, "XLL channel set outside of hierarchy");
return AVERROR_PATCHWELCOME;
}


if (c->dmix_coeffs_present && (ret = parse_dmix_coeffs(s, c)) < 0)
return ret;


if (!get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx, "Disabled XLL channel mask");
return AVERROR_PATCHWELCOME;
}


c->ch_mask = get_bits_long(&s->gb, s->ch_mask_nbits);
if (av_popcount(c->ch_mask) != c->nchannels) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL channel mask\n");
return AVERROR_INVALIDDATA;
}


for (i = 0, j = 0; i < s->ch_mask_nbits; i++)
if (c->ch_mask & (1U << i))
c->ch_remap[j++] = i;
} else {

if (c->nchannels != 2 || s->nchsets != 1 || get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx, "Custom XLL channel to speaker mapping");
return AVERROR_PATCHWELCOME;
}


c->primary_chset = 1;
c->dmix_coeffs_present = 0;
c->dmix_embedded = 0;
c->hier_chset = 0;
c->ch_mask = DCA_SPEAKER_LAYOUT_STEREO;
c->ch_remap[0] = DCA_SPEAKER_L;
c->ch_remap[1] = DCA_SPEAKER_R;
}

if (c->freq > 96000) {

if (get_bits1(&s->gb)) {
avpriv_request_sample(s->avctx, "Extra XLL frequency bands");
return AVERROR_PATCHWELCOME;
}
c->nfreqbands = 2;
} else {
c->nfreqbands = 1;
}



c->freq >>= c->nfreqbands - 1;


if (c != p && (c->nfreqbands != p->nfreqbands || c->freq != p->freq
|| c->pcm_bit_res != p->pcm_bit_res
|| c->storage_bit_res != p->storage_bit_res)) {
avpriv_request_sample(s->avctx, "Different XLL audio characteristics");
return AVERROR_PATCHWELCOME;
}


if (c->storage_bit_res > 16)
c->nabits = 5;
else if (c->storage_bit_res > 8)
c->nabits = 4;
else
c->nabits = 3;


if ((s->nchsets > 1 || c->nfreqbands > 1) && c->nabits < 5)
c->nabits++;

for (band = 0, b = c->bands; band < c->nfreqbands; band++, b++) {

if ((b->decor_enabled = get_bits1(&s->gb)) && c->nchannels > 1) {
int ch_nbits = av_ceil_log2(c->nchannels);


for (i = 0; i < c->nchannels; i++) {
b->orig_order[i] = get_bits(&s->gb, ch_nbits);
if (b->orig_order[i] >= c->nchannels) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL original channel order\n");
return AVERROR_INVALIDDATA;
}
}


for (i = 0; i < c->nchannels / 2; i++)
b->decor_coeff[i] = get_bits1(&s->gb) ? get_linear(&s->gb, 7) : 0;
} else {
for (i = 0; i < c->nchannels; i++)
b->orig_order[i] = i;
for (i = 0; i < c->nchannels / 2; i++)
b->decor_coeff[i] = 0;
}


b->highest_pred_order = 0;
for (i = 0; i < c->nchannels; i++) {
b->adapt_pred_order[i] = get_bits(&s->gb, 4);
if (b->adapt_pred_order[i] > b->highest_pred_order)
b->highest_pred_order = b->adapt_pred_order[i];
}
if (b->highest_pred_order > s->nsegsamples) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL adaptive predicition order\n");
return AVERROR_INVALIDDATA;
}


for (i = 0; i < c->nchannels; i++)
b->fixed_pred_order[i] = b->adapt_pred_order[i] ? 0 : get_bits(&s->gb, 2);


for (i = 0; i < c->nchannels; i++) {
for (j = 0; j < b->adapt_pred_order[i]; j++) {
k = get_linear(&s->gb, 8);
if (k == -128) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL reflection coefficient index\n");
return AVERROR_INVALIDDATA;
}
if (k < 0)
b->adapt_refl_coeff[i][j] = -(int)ff_dca_xll_refl_coeff[-k];
else
b->adapt_refl_coeff[i][j] = (int)ff_dca_xll_refl_coeff[ k];
}
}


b->dmix_embedded = c->dmix_embedded && (band == 0 || get_bits1(&s->gb));


if ((band == 0 && s->scalable_lsbs) || (band != 0 && get_bits1(&s->gb))) {

b->lsb_section_size = get_bits_long(&s->gb, s->seg_size_nbits);
if (b->lsb_section_size < 0 || b->lsb_section_size > s->frame_size) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid LSB section size\n");
return AVERROR_INVALIDDATA;
}


if (b->lsb_section_size && (s->band_crc_present > 2 ||
(band == 0 && s->band_crc_present > 1)))
b->lsb_section_size += 2;


for (i = 0; i < c->nchannels; i++) {
b->nscalablelsbs[i] = get_bits(&s->gb, 4);
if (b->nscalablelsbs[i] && !b->lsb_section_size) {
av_log(s->avctx, AV_LOG_ERROR, "LSB section missing with non-zero LSB width\n");
return AVERROR_INVALIDDATA;
}
}
} else {
b->lsb_section_size = 0;
for (i = 0; i < c->nchannels; i++)
b->nscalablelsbs[i] = 0;
}


if ((band == 0 && s->scalable_lsbs) || (band != 0 && get_bits1(&s->gb))) {

for (i = 0; i < c->nchannels; i++)
b->bit_width_adjust[i] = get_bits(&s->gb, 4);
} else {
for (i = 0; i < c->nchannels; i++)
b->bit_width_adjust[i] = 0;
}
}




if (ff_dca_seek_bits(&s->gb, header_pos + header_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL sub-header\n");
return AVERROR_INVALIDDATA;
}

return 0;
}

static int chs_alloc_msb_band_data(DCAXllDecoder *s, DCAXllChSet *c)
{
int ndecisamples = c->nfreqbands > 1 ? DCA_XLL_DECI_HISTORY_MAX : 0;
int nchsamples = s->nframesamples + ndecisamples;
int i, j, nsamples = nchsamples * c->nchannels * c->nfreqbands;
int32_t *ptr;


av_fast_malloc(&c->sample_buffer[0], &c->sample_size[0], nsamples * sizeof(int32_t));
if (!c->sample_buffer[0])
return AVERROR(ENOMEM);

ptr = c->sample_buffer[0] + ndecisamples;
for (i = 0; i < c->nfreqbands; i++) {
for (j = 0; j < c->nchannels; j++) {
c->bands[i].msb_sample_buffer[j] = ptr;
ptr += nchsamples;
}
}

return 0;
}

static int chs_alloc_lsb_band_data(DCAXllDecoder *s, DCAXllChSet *c)
{
int i, j, nsamples = 0;
int32_t *ptr;


for (i = 0; i < c->nfreqbands; i++)
if (c->bands[i].lsb_section_size)
nsamples += s->nframesamples * c->nchannels;
if (!nsamples)
return 0;


av_fast_malloc(&c->sample_buffer[1], &c->sample_size[1], nsamples * sizeof(int32_t));
if (!c->sample_buffer[1])
return AVERROR(ENOMEM);

ptr = c->sample_buffer[1];
for (i = 0; i < c->nfreqbands; i++) {
if (c->bands[i].lsb_section_size) {
for (j = 0; j < c->nchannels; j++) {
c->bands[i].lsb_sample_buffer[j] = ptr;
ptr += s->nframesamples;
}
} else {
for (j = 0; j < c->nchannels; j++)
c->bands[i].lsb_sample_buffer[j] = NULL;
}
}

return 0;
}

static int chs_parse_band_data(DCAXllDecoder *s, DCAXllChSet *c, int band, int seg, int band_data_end)
{
DCAXllBand *b = &c->bands[band];
int i, j, k;


if (!(seg && get_bits1(&s->gb))) {



c->seg_common = get_bits1(&s->gb);


k = c->seg_common ? 1 : c->nchannels;


for (i = 0; i < k; i++) {


c->rice_code_flag[i] = get_bits1(&s->gb);


if (!c->seg_common && c->rice_code_flag[i] && get_bits1(&s->gb))

c->bitalloc_hybrid_linear[i] = get_bits(&s->gb, c->nabits) + 1;
else

c->bitalloc_hybrid_linear[i] = 0;
}


for (i = 0; i < k; i++) {
if (seg == 0) {

c->bitalloc_part_a[i] = get_bits(&s->gb, c->nabits);


if (!c->rice_code_flag[i] && c->bitalloc_part_a[i])
c->bitalloc_part_a[i]++;

if (!c->seg_common)
c->nsamples_part_a[i] = b->adapt_pred_order[i];
else
c->nsamples_part_a[i] = b->highest_pred_order;
} else {
c->bitalloc_part_a[i] = 0;
c->nsamples_part_a[i] = 0;
}


c->bitalloc_part_b[i] = get_bits(&s->gb, c->nabits);


if (!c->rice_code_flag[i] && c->bitalloc_part_b[i])
c->bitalloc_part_b[i]++;
}
}


for (i = 0; i < c->nchannels; i++) {
int32_t *part_a, *part_b;
int nsamples_part_b;


k = c->seg_common ? 0 : i;


part_a = b->msb_sample_buffer[i] + seg * s->nsegsamples;
part_b = part_a + c->nsamples_part_a[k];
nsamples_part_b = s->nsegsamples - c->nsamples_part_a[k];

if (get_bits_left(&s->gb) < 0)
return AVERROR_INVALIDDATA;

if (!c->rice_code_flag[k]) {


get_linear_array(&s->gb, part_a, c->nsamples_part_a[k],
c->bitalloc_part_a[k]);


get_linear_array(&s->gb, part_b, nsamples_part_b,
c->bitalloc_part_b[k]);
} else {


get_rice_array(&s->gb, part_a, c->nsamples_part_a[k],
c->bitalloc_part_a[k]);

if (c->bitalloc_hybrid_linear[k]) {


int nisosamples = get_bits(&s->gb, s->nsegsamples_log2);


memset(part_b, 0, sizeof(*part_b) * nsamples_part_b);


for (j = 0; j < nisosamples; j++) {
int loc = get_bits(&s->gb, s->nsegsamples_log2);
if (loc >= nsamples_part_b) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid isolated sample location\n");
return AVERROR_INVALIDDATA;
}
part_b[loc] = -1;
}


for (j = 0; j < nsamples_part_b; j++) {
if (part_b[j])
part_b[j] = get_linear(&s->gb, c->bitalloc_hybrid_linear[k]);
else
part_b[j] = get_rice(&s->gb, c->bitalloc_part_b[k]);
}
} else {


get_rice_array(&s->gb, part_b, nsamples_part_b, c->bitalloc_part_b[k]);
}
}
}


if (seg == 0 && band == 1) {
int nbits = get_bits(&s->gb, 5) + 1;
for (i = 0; i < c->nchannels; i++)
for (j = 1; j < DCA_XLL_DECI_HISTORY_MAX; j++)
c->deci_history[i][j] = get_sbits_long(&s->gb, nbits);
}


if (b->lsb_section_size) {

if (ff_dca_seek_bits(&s->gb, band_data_end - b->lsb_section_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL band data\n");
return AVERROR_INVALIDDATA;
}


for (i = 0; i < c->nchannels; i++) {
if (b->nscalablelsbs[i]) {
get_array(&s->gb,
b->lsb_sample_buffer[i] + seg * s->nsegsamples,
s->nsegsamples, b->nscalablelsbs[i]);
}
}
}


if (ff_dca_seek_bits(&s->gb, band_data_end)) {
av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL band data\n");
return AVERROR_INVALIDDATA;
}

return 0;
}

static av_cold void chs_clear_band_data(DCAXllDecoder *s, DCAXllChSet *c, int band, int seg)
{
DCAXllBand *b = &c->bands[band];
int i, offset, nsamples;

if (seg < 0) {
offset = 0;
nsamples = s->nframesamples;
} else {
offset = seg * s->nsegsamples;
nsamples = s->nsegsamples;
}

for (i = 0; i < c->nchannels; i++) {
memset(b->msb_sample_buffer[i] + offset, 0, nsamples * sizeof(int32_t));
if (b->lsb_section_size)
memset(b->lsb_sample_buffer[i] + offset, 0, nsamples * sizeof(int32_t));
}

if (seg <= 0 && band)
memset(c->deci_history, 0, sizeof(c->deci_history));

if (seg < 0) {
memset(b->nscalablelsbs, 0, sizeof(b->nscalablelsbs));
memset(b->bit_width_adjust, 0, sizeof(b->bit_width_adjust));
}
}

static void chs_filter_band_data(DCAXllDecoder *s, DCAXllChSet *c, int band)
{
DCAXllBand *b = &c->bands[band];
int nsamples = s->nframesamples;
int i, j, k;


for (i = 0; i < c->nchannels; i++) {
int32_t *buf = b->msb_sample_buffer[i];
int order = b->adapt_pred_order[i];
if (order > 0) {
int coeff[DCA_XLL_ADAPT_PRED_ORDER_MAX];

for (j = 0; j < order; j++) {
int rc = b->adapt_refl_coeff[i][j];
for (k = 0; k < (j + 1) / 2; k++) {
int tmp1 = coeff[ k ];
int tmp2 = coeff[j - k - 1];
coeff[ k ] = tmp1 + mul16(rc, tmp2);
coeff[j - k - 1] = tmp2 + mul16(rc, tmp1);
}
coeff[j] = rc;
}

for (j = 0; j < nsamples - order; j++) {
int64_t err = 0;
for (k = 0; k < order; k++)
err += (int64_t)buf[j + k] * coeff[order - k - 1];
buf[j + k] -= (SUINT)clip23(norm16(err));
}
} else {

for (j = 0; j < b->fixed_pred_order[i]; j++)
for (k = 1; k < nsamples; k++)
buf[k] += (unsigned)buf[k - 1];
}
}


if (b->decor_enabled) {
int32_t *tmp[DCA_XLL_CHANNELS_MAX];

for (i = 0; i < c->nchannels / 2; i++) {
int coeff = b->decor_coeff[i];
if (coeff) {
s->dcadsp->decor(b->msb_sample_buffer[i * 2 + 1],
b->msb_sample_buffer[i * 2 ],
coeff, nsamples);
}
}


for (i = 0; i < c->nchannels; i++)
tmp[i] = b->msb_sample_buffer[i];

for (i = 0; i < c->nchannels; i++)
b->msb_sample_buffer[b->orig_order[i]] = tmp[i];
}


if (c->nfreqbands == 1)
for (i = 0; i < c->nchannels; i++)
s->output_samples[c->ch_remap[i]] = b->msb_sample_buffer[i];
}

static int chs_get_lsb_width(DCAXllDecoder *s, DCAXllChSet *c, int band, int ch)
{
int adj = c->bands[band].bit_width_adjust[ch];
int shift = c->bands[band].nscalablelsbs[ch];

if (s->fixed_lsb_width)
shift = s->fixed_lsb_width;
else if (shift && adj)
shift += adj - 1;
else
shift += adj;

return shift;
}

static void chs_assemble_msbs_lsbs(DCAXllDecoder *s, DCAXllChSet *c, int band)
{
DCAXllBand *b = &c->bands[band];
int n, ch, nsamples = s->nframesamples;

for (ch = 0; ch < c->nchannels; ch++) {
int shift = chs_get_lsb_width(s, c, band, ch);
if (shift) {
int32_t *msb = b->msb_sample_buffer[ch];
if (b->nscalablelsbs[ch]) {
int32_t *lsb = b->lsb_sample_buffer[ch];
int adj = b->bit_width_adjust[ch];
for (n = 0; n < nsamples; n++)
msb[n] = msb[n] * (SUINT)(1 << shift) + (lsb[n] << adj);
} else {
for (n = 0; n < nsamples; n++)
msb[n] = msb[n] * (SUINT)(1 << shift);
}
}
}
}

static int chs_assemble_freq_bands(DCAXllDecoder *s, DCAXllChSet *c)
{
int ch, nsamples = s->nframesamples;
int32_t *ptr;

av_assert1(c->nfreqbands > 1);


av_fast_malloc(&c->sample_buffer[2], &c->sample_size[2],
2 * nsamples * c->nchannels * sizeof(int32_t));
if (!c->sample_buffer[2])
return AVERROR(ENOMEM);


ptr = c->sample_buffer[2];
for (ch = 0; ch < c->nchannels; ch++) {
int32_t *band0 = c->bands[0].msb_sample_buffer[ch];
int32_t *band1 = c->bands[1].msb_sample_buffer[ch];


memcpy(band0 - DCA_XLL_DECI_HISTORY_MAX,
c->deci_history[ch], sizeof(c->deci_history[0]));


s->dcadsp->assemble_freq_bands(ptr, band0, band1,
ff_dca_xll_band_coeff,
nsamples);


s->output_samples[c->ch_remap[ch]] = ptr;
ptr += nsamples * 2;
}

return 0;
}

static int parse_common_header(DCAXllDecoder *s)
{
int stream_ver, header_size, frame_size_nbits, nframesegs_log2;


if (get_bits_long(&s->gb, 32) != DCA_SYNCWORD_XLL) {
av_log(s->avctx, AV_LOG_VERBOSE, "Invalid XLL sync word\n");
return AVERROR(EAGAIN);
}


stream_ver = get_bits(&s->gb, 4) + 1;
if (stream_ver > 1) {
avpriv_request_sample(s->avctx, "XLL stream version %d", stream_ver);
return AVERROR_PATCHWELCOME;
}


header_size = get_bits(&s->gb, 8) + 1;


if (ff_dca_check_crc(s->avctx, &s->gb, 32, header_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL common header checksum\n");
return AVERROR_INVALIDDATA;
}


frame_size_nbits = get_bits(&s->gb, 5) + 1;


s->frame_size = get_bits_long(&s->gb, frame_size_nbits);
if (s->frame_size < 0 || s->frame_size >= DCA_XLL_PBR_BUFFER_MAX) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid XLL frame size (%d bytes)\n", s->frame_size);
return AVERROR_INVALIDDATA;
}
s->frame_size++;


s->nchsets = get_bits(&s->gb, 4) + 1;
if (s->nchsets > DCA_XLL_CHSETS_MAX) {
avpriv_request_sample(s->avctx, "%d XLL channel sets", s->nchsets);
return AVERROR_PATCHWELCOME;
}


nframesegs_log2 = get_bits(&s->gb, 4);
s->nframesegs = 1 << nframesegs_log2;
if (s->nframesegs > 1024) {
av_log(s->avctx, AV_LOG_ERROR, "Too many segments per XLL frame\n");
return AVERROR_INVALIDDATA;
}




s->nsegsamples_log2 = get_bits(&s->gb, 4);
if (!s->nsegsamples_log2) {
av_log(s->avctx, AV_LOG_ERROR, "Too few samples per XLL segment\n");
return AVERROR_INVALIDDATA;
}
s->nsegsamples = 1 << s->nsegsamples_log2;
if (s->nsegsamples > 512) {
av_log(s->avctx, AV_LOG_ERROR, "Too many samples per XLL segment\n");
return AVERROR_INVALIDDATA;
}


s->nframesamples_log2 = s->nsegsamples_log2 + nframesegs_log2;
s->nframesamples = 1 << s->nframesamples_log2;
if (s->nframesamples > 65536) {
av_log(s->avctx, AV_LOG_ERROR, "Too many samples per XLL frame\n");
return AVERROR_INVALIDDATA;
}


s->seg_size_nbits = get_bits(&s->gb, 5) + 1;






s->band_crc_present = get_bits(&s->gb, 2);


s->scalable_lsbs = get_bits1(&s->gb);


s->ch_mask_nbits = get_bits(&s->gb, 5) + 1;


if (s->scalable_lsbs)
s->fixed_lsb_width = get_bits(&s->gb, 4);
else
s->fixed_lsb_width = 0;




if (ff_dca_seek_bits(&s->gb, header_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL common header\n");
return AVERROR_INVALIDDATA;
}

return 0;
}

static int is_hier_dmix_chset(DCAXllChSet *c)
{
return !c->primary_chset && c->dmix_embedded && c->hier_chset;
}

static DCAXllChSet *find_next_hier_dmix_chset(DCAXllDecoder *s, DCAXllChSet *c)
{
if (c->hier_chset)
while (++c < &s->chset[s->nchsets])
if (is_hier_dmix_chset(c))
return c;

return NULL;
}

static void prescale_down_mix(DCAXllChSet *c, DCAXllChSet *o)
{
int i, j, *coeff_ptr = c->dmix_coeff;

for (i = 0; i < c->hier_ofs; i++) {
int scale = o->dmix_scale[i];
int scale_inv = o->dmix_scale_inv[i];
c->dmix_scale[i] = mul15(c->dmix_scale[i], scale);
c->dmix_scale_inv[i] = mul16(c->dmix_scale_inv[i], scale_inv);
for (j = 0; j < c->nchannels; j++) {
int coeff = mul16(*coeff_ptr, scale_inv);
*coeff_ptr++ = mul15(coeff, o->dmix_scale[c->hier_ofs + j]);
}
}
}

static int parse_sub_headers(DCAXllDecoder *s, DCAExssAsset *asset)
{
DCAContext *dca = s->avctx->priv_data;
DCAXllChSet *c;
int i, ret;


s->nfreqbands = 0;
s->nchannels = 0;
s->nreschsets = 0;
for (i = 0, c = s->chset; i < s->nchsets; i++, c++) {
c->hier_ofs = s->nchannels;
if ((ret = chs_parse_header(s, c, asset)) < 0)
return ret;
if (c->nfreqbands > s->nfreqbands)
s->nfreqbands = c->nfreqbands;
if (c->hier_chset)
s->nchannels += c->nchannels;
if (c->residual_encode != (1 << c->nchannels) - 1)
s->nreschsets++;
}


for (i = s->nchsets - 1, c = &s->chset[i]; i > 0; i--, c--) {
if (is_hier_dmix_chset(c)) {
DCAXllChSet *o = find_next_hier_dmix_chset(s, c);
if (o)
prescale_down_mix(c, o);
}
}


switch (dca->request_channel_layout) {
case DCA_SPEAKER_LAYOUT_STEREO:
s->nactivechsets = 1;
break;
case DCA_SPEAKER_LAYOUT_5POINT0:
case DCA_SPEAKER_LAYOUT_5POINT1:
s->nactivechsets = (s->chset[0].nchannels < 5 && s->nchsets > 1) ? 2 : 1;
break;
default:
s->nactivechsets = s->nchsets;
break;
}

return 0;
}

static int parse_navi_table(DCAXllDecoder *s)
{
int chs, seg, band, navi_nb, navi_pos, *navi_ptr;
DCAXllChSet *c;


navi_nb = s->nfreqbands * s->nframesegs * s->nchsets;
if (navi_nb > 1024) {
av_log(s->avctx, AV_LOG_ERROR, "Too many NAVI entries (%d)\n", navi_nb);
return AVERROR_INVALIDDATA;
}


av_fast_malloc(&s->navi, &s->navi_size, navi_nb * sizeof(*s->navi));
if (!s->navi)
return AVERROR(ENOMEM);


navi_pos = get_bits_count(&s->gb);
navi_ptr = s->navi;
for (band = 0; band < s->nfreqbands; band++) {
for (seg = 0; seg < s->nframesegs; seg++) {
for (chs = 0, c = s->chset; chs < s->nchsets; chs++, c++) {
int size = 0;
if (c->nfreqbands > band) {
size = get_bits_long(&s->gb, s->seg_size_nbits);
if (size < 0 || size >= s->frame_size) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI segment size (%d bytes)\n", size);
return AVERROR_INVALIDDATA;
}
size++;
}
*navi_ptr++ = size;
}
}
}



skip_bits(&s->gb, -get_bits_count(&s->gb) & 7);
skip_bits(&s->gb, 16);


if (ff_dca_check_crc(s->avctx, &s->gb, navi_pos, get_bits_count(&s->gb))) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI checksum\n");
return AVERROR_INVALIDDATA;
}

return 0;
}

static int parse_band_data(DCAXllDecoder *s)
{
int ret, chs, seg, band, navi_pos, *navi_ptr;
DCAXllChSet *c;

for (chs = 0, c = s->chset; chs < s->nactivechsets; chs++, c++) {
if ((ret = chs_alloc_msb_band_data(s, c)) < 0)
return ret;
if ((ret = chs_alloc_lsb_band_data(s, c)) < 0)
return ret;
}

navi_pos = get_bits_count(&s->gb);
navi_ptr = s->navi;
for (band = 0; band < s->nfreqbands; band++) {
for (seg = 0; seg < s->nframesegs; seg++) {
for (chs = 0, c = s->chset; chs < s->nchsets; chs++, c++) {
if (c->nfreqbands > band) {
navi_pos += *navi_ptr * 8;
if (navi_pos > s->gb.size_in_bits) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid NAVI position\n");
return AVERROR_INVALIDDATA;
}
if (chs < s->nactivechsets &&
(ret = chs_parse_band_data(s, c, band, seg, navi_pos)) < 0) {
if (s->avctx->err_recognition & AV_EF_EXPLODE)
return ret;
chs_clear_band_data(s, c, band, seg);
}
skip_bits_long(&s->gb, navi_pos - get_bits_count(&s->gb));
}
navi_ptr++;
}
}
}

return 0;
}

static int parse_frame(DCAXllDecoder *s, uint8_t *data, int size, DCAExssAsset *asset)
{
int ret;

if ((ret = init_get_bits8(&s->gb, data, size)) < 0)
return ret;
if ((ret = parse_common_header(s)) < 0)
return ret;
if ((ret = parse_sub_headers(s, asset)) < 0)
return ret;
if ((ret = parse_navi_table(s)) < 0)
return ret;
if ((ret = parse_band_data(s)) < 0)
return ret;
if (ff_dca_seek_bits(&s->gb, s->frame_size * 8)) {
av_log(s->avctx, AV_LOG_ERROR, "Read past end of XLL frame\n");
return AVERROR_INVALIDDATA;
}
return ret;
}

static void clear_pbr(DCAXllDecoder *s)
{
s->pbr_length = 0;
s->pbr_delay = 0;
}

static int copy_to_pbr(DCAXllDecoder *s, uint8_t *data, int size, int delay)
{
if (size > DCA_XLL_PBR_BUFFER_MAX)
return AVERROR(ENOSPC);

if (!s->pbr_buffer && !(s->pbr_buffer = av_malloc(DCA_XLL_PBR_BUFFER_MAX + AV_INPUT_BUFFER_PADDING_SIZE)))
return AVERROR(ENOMEM);

memcpy(s->pbr_buffer, data, size);
s->pbr_length = size;
s->pbr_delay = delay;
return 0;
}

static int parse_frame_no_pbr(DCAXllDecoder *s, uint8_t *data, int size, DCAExssAsset *asset)
{
int ret = parse_frame(s, data, size, asset);



if (ret == AVERROR(EAGAIN) && asset->xll_sync_present && asset->xll_sync_offset < size) {

data += asset->xll_sync_offset;
size -= asset->xll_sync_offset;




if (asset->xll_delay_nframes > 0) {
if ((ret = copy_to_pbr(s, data, size, asset->xll_delay_nframes)) < 0)
return ret;
return AVERROR(EAGAIN);
}


ret = parse_frame(s, data, size, asset);
}

if (ret < 0)
return ret;

if (s->frame_size > size)
return AVERROR(EINVAL);


if (s->frame_size < size)
if ((ret = copy_to_pbr(s, data + s->frame_size, size - s->frame_size, 0)) < 0)
return ret;

return 0;
}

static int parse_frame_pbr(DCAXllDecoder *s, uint8_t *data, int size, DCAExssAsset *asset)
{
int ret;

if (size > DCA_XLL_PBR_BUFFER_MAX - s->pbr_length) {
ret = AVERROR(ENOSPC);
goto fail;
}

memcpy(s->pbr_buffer + s->pbr_length, data, size);
s->pbr_length += size;


if (s->pbr_delay > 0 && --s->pbr_delay)
return AVERROR(EAGAIN);

if ((ret = parse_frame(s, s->pbr_buffer, s->pbr_length, asset)) < 0)
goto fail;

if (s->frame_size > s->pbr_length) {
ret = AVERROR(EINVAL);
goto fail;
}

if (s->frame_size == s->pbr_length) {

clear_pbr(s);
} else {
s->pbr_length -= s->frame_size;
memmove(s->pbr_buffer, s->pbr_buffer + s->frame_size, s->pbr_length);
}

return 0;

fail:


clear_pbr(s);
return ret;
}

int ff_dca_xll_parse(DCAXllDecoder *s, uint8_t *data, DCAExssAsset *asset)
{
int ret;

if (s->hd_stream_id != asset->hd_stream_id) {
clear_pbr(s);
s->hd_stream_id = asset->hd_stream_id;
}

if (s->pbr_length)
ret = parse_frame_pbr(s, data + asset->xll_offset, asset->xll_size, asset);
else
ret = parse_frame_no_pbr(s, data + asset->xll_offset, asset->xll_size, asset);

return ret;
}

static void undo_down_mix(DCAXllDecoder *s, DCAXllChSet *o, int band)
{
int i, j, k, nchannels = 0, *coeff_ptr = o->dmix_coeff;
DCAXllChSet *c;

for (i = 0, c = s->chset; i < s->nactivechsets; i++, c++) {
if (!c->hier_chset)
continue;

av_assert1(band < c->nfreqbands);
for (j = 0; j < c->nchannels; j++) {
for (k = 0; k < o->nchannels; k++) {
int coeff = *coeff_ptr++;
if (coeff) {
s->dcadsp->dmix_sub(c->bands[band].msb_sample_buffer[j],
o->bands[band].msb_sample_buffer[k],
coeff, s->nframesamples);
if (band)
s->dcadsp->dmix_sub(c->deci_history[j],
o->deci_history[k],
coeff, DCA_XLL_DECI_HISTORY_MAX);
}
}
}

nchannels += c->nchannels;
if (nchannels >= o->hier_ofs)
break;
}
}

static void scale_down_mix(DCAXllDecoder *s, DCAXllChSet *o, int band)
{
int i, j, nchannels = 0;
DCAXllChSet *c;

for (i = 0, c = s->chset; i < s->nactivechsets; i++, c++) {
if (!c->hier_chset)
continue;

av_assert1(band < c->nfreqbands);
for (j = 0; j < c->nchannels; j++) {
int scale = o->dmix_scale[nchannels++];
if (scale != (1 << 15)) {
s->dcadsp->dmix_scale(c->bands[band].msb_sample_buffer[j],
scale, s->nframesamples);
if (band)
s->dcadsp->dmix_scale(c->deci_history[j],
scale, DCA_XLL_DECI_HISTORY_MAX);
}
}

if (nchannels >= o->hier_ofs)
break;
}
}



static av_cold void force_lossy_output(DCAXllDecoder *s, DCAXllChSet *c)
{
DCAContext *dca = s->avctx->priv_data;
int band, ch;

for (band = 0; band < c->nfreqbands; band++)
chs_clear_band_data(s, c, band, -1);

for (ch = 0; ch < c->nchannels; ch++) {
if (!(c->residual_encode & (1 << ch)))
continue;
if (ff_dca_core_map_spkr(&dca->core, c->ch_remap[ch]) < 0)
continue;
c->residual_encode &= ~(1 << ch);
}
}

static int combine_residual_frame(DCAXllDecoder *s, DCAXllChSet *c)
{
DCAContext *dca = s->avctx->priv_data;
int ch, nsamples = s->nframesamples;
DCAXllChSet *o;


if (!(dca->packet & DCA_PACKET_CORE)) {
av_log(s->avctx, AV_LOG_ERROR, "Residual encoded channels are present without core\n");
return AVERROR(EINVAL);
}

if (c->freq != dca->core.output_rate) {
av_log(s->avctx, AV_LOG_WARNING, "Sample rate mismatch between core (%d Hz) and XLL (%d Hz)\n", dca->core.output_rate, c->freq);
return AVERROR_INVALIDDATA;
}

if (nsamples != dca->core.npcmsamples) {
av_log(s->avctx, AV_LOG_WARNING, "Number of samples per frame mismatch between core (%d) and XLL (%d)\n", dca->core.npcmsamples, nsamples);
return AVERROR_INVALIDDATA;
}




o = find_next_hier_dmix_chset(s, c);


for (ch = 0; ch < c->nchannels; ch++) {
int n, spkr, shift, round;
int32_t *src, *dst;

if (c->residual_encode & (1 << ch))
continue;


spkr = ff_dca_core_map_spkr(&dca->core, c->ch_remap[ch]);
if (spkr < 0) {
av_log(s->avctx, AV_LOG_WARNING, "Residual encoded channel (%d) references unavailable core channel\n", c->ch_remap[ch]);
return AVERROR_INVALIDDATA;
}


shift = 24 - c->pcm_bit_res + chs_get_lsb_width(s, c, 0, ch);
if (shift > 24) {
av_log(s->avctx, AV_LOG_WARNING, "Invalid core shift (%d bits)\n", shift);
return AVERROR_INVALIDDATA;
}

round = shift > 0 ? 1 << (shift - 1) : 0;

src = dca->core.output_samples[spkr];
dst = c->bands[0].msb_sample_buffer[ch];
if (o) {

int scale_inv = o->dmix_scale_inv[c->hier_ofs + ch];
for (n = 0; n < nsamples; n++)
dst[n] += (SUINT)clip23((mul16(src[n], scale_inv) + round) >> shift);
} else {

for (n = 0; n < nsamples; n++)
dst[n] += (unsigned)((src[n] + round) >> shift);
}
}

return 0;
}

int ff_dca_xll_filter_frame(DCAXllDecoder *s, AVFrame *frame)
{
AVCodecContext *avctx = s->avctx;
DCAContext *dca = avctx->priv_data;
DCAExssAsset *asset = &dca->exss.assets[0];
DCAXllChSet *p = &s->chset[0], *c;
enum AVMatrixEncoding matrix_encoding = AV_MATRIX_ENCODING_NONE;
int i, j, k, ret, shift, nsamples, request_mask;
int ch_remap[DCA_SPEAKER_COUNT];


if (dca->packet & DCA_PACKET_RECOVERY) {
for (i = 0, c = s->chset; i < s->nchsets; i++, c++) {
if (i < s->nactivechsets)
force_lossy_output(s, c);

if (!c->primary_chset)
c->dmix_embedded = 0;
}

s->scalable_lsbs = 0;
s->fixed_lsb_width = 0;
}


s->output_mask = 0;
for (i = 0, c = s->chset; i < s->nactivechsets; i++, c++) {
chs_filter_band_data(s, c, 0);

if (c->residual_encode != (1 << c->nchannels) - 1
&& (ret = combine_residual_frame(s, c)) < 0)
return ret;

if (s->scalable_lsbs)
chs_assemble_msbs_lsbs(s, c, 0);

if (c->nfreqbands > 1) {
chs_filter_band_data(s, c, 1);
chs_assemble_msbs_lsbs(s, c, 1);
}

s->output_mask |= c->ch_mask;
}


for (i = 1, c = &s->chset[1]; i < s->nchsets; i++, c++) {
if (!is_hier_dmix_chset(c))
continue;

if (i >= s->nactivechsets) {
for (j = 0; j < c->nfreqbands; j++)
if (c->bands[j].dmix_embedded)
scale_down_mix(s, c, j);
break;
}

for (j = 0; j < c->nfreqbands; j++)
if (c->bands[j].dmix_embedded)
undo_down_mix(s, c, j);
}


if (s->nfreqbands > 1) {
for (i = 0; i < s->nactivechsets; i++)
if ((ret = chs_assemble_freq_bands(s, &s->chset[i])) < 0)
return ret;
}


if (dca->request_channel_layout) {
if (s->output_mask & DCA_SPEAKER_MASK_Lss) {
s->output_samples[DCA_SPEAKER_Ls] = s->output_samples[DCA_SPEAKER_Lss];
s->output_mask = (s->output_mask & ~DCA_SPEAKER_MASK_Lss) | DCA_SPEAKER_MASK_Ls;
}
if (s->output_mask & DCA_SPEAKER_MASK_Rss) {
s->output_samples[DCA_SPEAKER_Rs] = s->output_samples[DCA_SPEAKER_Rss];
s->output_mask = (s->output_mask & ~DCA_SPEAKER_MASK_Rss) | DCA_SPEAKER_MASK_Rs;
}
}


if (dca->request_channel_layout == DCA_SPEAKER_LAYOUT_STEREO
&& DCA_HAS_STEREO(s->output_mask) && p->dmix_embedded
&& (p->dmix_type == DCA_DMIX_TYPE_LoRo ||
p->dmix_type == DCA_DMIX_TYPE_LtRt))
request_mask = DCA_SPEAKER_LAYOUT_STEREO;
else
request_mask = s->output_mask;
if (!ff_dca_set_channel_layout(avctx, ch_remap, request_mask))
return AVERROR(EINVAL);

avctx->sample_rate = p->freq << (s->nfreqbands - 1);

switch (p->storage_bit_res) {
case 16:
avctx->sample_fmt = AV_SAMPLE_FMT_S16P;
shift = 16 - p->pcm_bit_res;
break;
case 20:
case 24:
avctx->sample_fmt = AV_SAMPLE_FMT_S32P;
shift = 24 - p->pcm_bit_res;
break;
default:
return AVERROR(EINVAL);
}

avctx->bits_per_raw_sample = p->storage_bit_res;
avctx->profile = FF_PROFILE_DTS_HD_MA;
avctx->bit_rate = 0;

frame->nb_samples = nsamples = s->nframesamples << (s->nfreqbands - 1);
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;


if (request_mask != s->output_mask) {
ff_dca_downmix_to_stereo_fixed(s->dcadsp, s->output_samples,
p->dmix_coeff, nsamples,
s->output_mask);
}

for (i = 0; i < avctx->channels; i++) {
int32_t *samples = s->output_samples[ch_remap[i]];
if (frame->format == AV_SAMPLE_FMT_S16P) {
int16_t *plane = (int16_t *)frame->extended_data[i];
for (k = 0; k < nsamples; k++)
plane[k] = av_clip_int16(samples[k] * (SUINT)(1 << shift));
} else {
int32_t *plane = (int32_t *)frame->extended_data[i];
for (k = 0; k < nsamples; k++)
plane[k] = clip23(samples[k] * (SUINT)(1 << shift)) * (1 << 8);
}
}

if (!asset->one_to_one_map_ch_to_spkr) {
if (asset->representation_type == DCA_REPR_TYPE_LtRt)
matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
else if (asset->representation_type == DCA_REPR_TYPE_LhRh)
matrix_encoding = AV_MATRIX_ENCODING_DOLBYHEADPHONE;
} else if (request_mask != s->output_mask && p->dmix_type == DCA_DMIX_TYPE_LtRt) {
matrix_encoding = AV_MATRIX_ENCODING_DOLBY;
}
if ((ret = ff_side_data_update_matrix_encoding(frame, matrix_encoding)) < 0)
return ret;

return 0;
}

av_cold void ff_dca_xll_flush(DCAXllDecoder *s)
{
clear_pbr(s);
}

av_cold void ff_dca_xll_close(DCAXllDecoder *s)
{
DCAXllChSet *c;
int i, j;

for (i = 0, c = s->chset; i < DCA_XLL_CHSETS_MAX; i++, c++) {
for (j = 0; j < DCA_XLL_SAMPLE_BUFFERS_MAX; j++) {
av_freep(&c->sample_buffer[j]);
c->sample_size[j] = 0;
}
}

av_freep(&s->navi);
s->navi_size = 0;

av_freep(&s->pbr_buffer);
clear_pbr(s);
}
