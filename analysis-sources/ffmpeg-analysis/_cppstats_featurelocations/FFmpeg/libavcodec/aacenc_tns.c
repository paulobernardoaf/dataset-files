


























#include "libavutil/libm.h"
#include "aacenc.h"
#include "aacenc_tns.h"
#include "aactab.h"
#include "aacenc_utils.h"
#include "aacenc_quantization.h"


#define TNS_Q_BITS 4


#define TNS_Q_BITS_IS8 4


#define TNS_ENABLE_COEF_COMPRESSION


#define TNS_GAIN_THRESHOLD_LOW 1.4f
#define TNS_GAIN_THRESHOLD_HIGH 1.16f*TNS_GAIN_THRESHOLD_LOW

static inline int compress_coeffs(int *coef, int order, int c_bits)
{
int i;
const int low_idx = c_bits ? 4 : 2;
const int shift_val = c_bits ? 8 : 4;
const int high_idx = c_bits ? 11 : 5;
#if !defined(TNS_ENABLE_COEF_COMPRESSION)
return 0;
#endif 
for (i = 0; i < order; i++)
if (coef[i] >= low_idx && coef[i] <= high_idx)
return 0;
for (i = 0; i < order; i++)
coef[i] -= (coef[i] > high_idx) ? shift_val : 0;
return 1;
}






void ff_aac_encode_tns_info(AACEncContext *s, SingleChannelElement *sce)
{
TemporalNoiseShaping *tns = &sce->tns;
int i, w, filt, coef_compress = 0, coef_len;
const int is8 = sce->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE;
const int c_bits = is8 ? TNS_Q_BITS_IS8 == 4 : TNS_Q_BITS == 4;

if (!sce->tns.present)
return;

for (i = 0; i < sce->ics.num_windows; i++) {
put_bits(&s->pb, 2 - is8, sce->tns.n_filt[i]);
if (!tns->n_filt[i])
continue;
put_bits(&s->pb, 1, c_bits);
for (filt = 0; filt < tns->n_filt[i]; filt++) {
put_bits(&s->pb, 6 - 2 * is8, tns->length[i][filt]);
put_bits(&s->pb, 5 - 2 * is8, tns->order[i][filt]);
if (!tns->order[i][filt])
continue;
put_bits(&s->pb, 1, tns->direction[i][filt]);
coef_compress = compress_coeffs(tns->coef_idx[i][filt],
tns->order[i][filt], c_bits);
put_bits(&s->pb, 1, coef_compress);
coef_len = c_bits + 3 - coef_compress;
for (w = 0; w < tns->order[i][filt]; w++)
put_bits(&s->pb, coef_len, tns->coef_idx[i][filt][w]);
}
}
}


void ff_aac_apply_tns(AACEncContext *s, SingleChannelElement *sce)
{
TemporalNoiseShaping *tns = &sce->tns;
IndividualChannelStream *ics = &sce->ics;
int w, filt, m, i, top, order, bottom, start, end, size, inc;
const int mmm = FFMIN(ics->tns_max_bands, ics->max_sfb);
float lpc[TNS_MAX_ORDER];

for (w = 0; w < ics->num_windows; w++) {
bottom = ics->num_swb;
for (filt = 0; filt < tns->n_filt[w]; filt++) {
top = bottom;
bottom = FFMAX(0, top - tns->length[w][filt]);
order = tns->order[w][filt];
if (order == 0)
continue;


compute_lpc_coefs(tns->coef[w][filt], order, lpc, 0, 0, 0);

start = ics->swb_offset[FFMIN(bottom, mmm)];
end = ics->swb_offset[FFMIN( top, mmm)];
if ((size = end - start) <= 0)
continue;
if (tns->direction[w][filt]) {
inc = -1;
start = end - 1;
} else {
inc = 1;
}
start += w * 128;


for (m = 0; m < size; m++, start += inc) {
for (i = 1; i <= FFMIN(m, order); i++) {
sce->coeffs[start] += lpc[i-1]*sce->pcoeffs[start - i*inc];
}
}
}
}
}




static inline void quantize_coefs(double *coef, int *idx, float *lpc, int order,
int c_bits)
{
int i;
const float *quant_arr = tns_tmp2_map[c_bits];
for (i = 0; i < order; i++) {
idx[i] = quant_array_idx(coef[i], quant_arr, c_bits ? 16 : 8);
lpc[i] = quant_arr[idx[i]];
}
}




void ff_aac_search_for_tns(AACEncContext *s, SingleChannelElement *sce)
{
TemporalNoiseShaping *tns = &sce->tns;
int w, g, count = 0;
double gain, coefs[MAX_LPC_ORDER];
const int mmm = FFMIN(sce->ics.tns_max_bands, sce->ics.max_sfb);
const int is8 = sce->ics.window_sequence[0] == EIGHT_SHORT_SEQUENCE;
const int c_bits = is8 ? TNS_Q_BITS_IS8 == 4 : TNS_Q_BITS == 4;
const int sfb_start = av_clip(tns_min_sfb[is8][s->samplerate_index], 0, mmm);
const int sfb_end = av_clip(sce->ics.num_swb, 0, mmm);
const int order = is8 ? 7 : s->profile == FF_PROFILE_AAC_LOW ? 12 : TNS_MAX_ORDER;
const int slant = sce->ics.window_sequence[0] == LONG_STOP_SEQUENCE ? 1 :
sce->ics.window_sequence[0] == LONG_START_SEQUENCE ? 0 : 2;
const int sfb_len = sfb_end - sfb_start;
const int coef_len = sce->ics.swb_offset[sfb_end] - sce->ics.swb_offset[sfb_start];

if (coef_len <= 0 || sfb_len <= 0) {
sce->tns.present = 0;
return;
}

for (w = 0; w < sce->ics.num_windows; w++) {
float en[2] = {0.0f, 0.0f};
int oc_start = 0, os_start = 0;
int coef_start = sce->ics.swb_offset[sfb_start];

for (g = sfb_start; g < sce->ics.num_swb && g <= sfb_end; g++) {
FFPsyBand *band = &s->psy.ch[s->cur_channel].psy_bands[w*16+g];
if (g > sfb_start + (sfb_len/2))
en[1] += band->energy;
else
en[0] += band->energy;
}


gain = ff_lpc_calc_ref_coefs_f(&s->lpc, &sce->coeffs[w*128 + coef_start],
coef_len, order, coefs);

if (!order || !isfinite(gain) || gain < TNS_GAIN_THRESHOLD_LOW || gain > TNS_GAIN_THRESHOLD_HIGH)
continue;

tns->n_filt[w] = is8 ? 1 : order != TNS_MAX_ORDER ? 2 : 3;
for (g = 0; g < tns->n_filt[w]; g++) {
tns->direction[w][g] = slant != 2 ? slant : en[g] < en[!g];
tns->order[w][g] = g < tns->n_filt[w] ? order/tns->n_filt[w] : order - oc_start;
tns->length[w][g] = g < tns->n_filt[w] ? sfb_len/tns->n_filt[w] : sfb_len - os_start;
quantize_coefs(&coefs[oc_start], tns->coef_idx[w][g], tns->coef[w][g],
tns->order[w][g], c_bits);
oc_start += tns->order[w][g];
os_start += tns->length[w][g];
}
count++;
}
sce->tns.present = !!count;
}
