


























#if !defined(AVCODEC_AACENC_UTILS_H)
#define AVCODEC_AACENC_UTILS_H

#include "libavutil/ffmath.h"
#include "aac.h"
#include "aacenctab.h"
#include "aactab.h"

#define ROUND_STANDARD 0.4054f
#define ROUND_TO_ZERO 0.1054f
#define C_QUANT 0.4054f

static inline void abs_pow34_v(float *out, const float *in, const int size)
{
int i;
for (i = 0; i < size; i++) {
float a = fabsf(in[i]);
out[i] = sqrtf(a * sqrtf(a));
}
}

static inline float pos_pow34(float a)
{
return sqrtf(a * sqrtf(a));
}






static inline int quant(float coef, const float Q, const float rounding)
{
float a = coef * Q;
return sqrtf(a * sqrtf(a)) + rounding;
}

static inline void quantize_bands(int *out, const float *in, const float *scaled,
int size, int is_signed, int maxval, const float Q34,
const float rounding)
{
int i;
for (i = 0; i < size; i++) {
float qc = scaled[i] * Q34;
int tmp = (int)FFMIN(qc + rounding, (float)maxval);
if (is_signed && in[i] < 0.0f) {
tmp = -tmp;
}
out[i] = tmp;
}
}

static inline float find_max_val(int group_len, int swb_size, const float *scaled)
{
float maxval = 0.0f;
int w2, i;
for (w2 = 0; w2 < group_len; w2++) {
for (i = 0; i < swb_size; i++) {
maxval = FFMAX(maxval, scaled[w2*128+i]);
}
}
return maxval;
}

static inline int find_min_book(float maxval, int sf)
{
float Q34 = ff_aac_pow34sf_tab[POW_SF2_ZERO - sf + SCALE_ONE_POS - SCALE_DIV_512];
int qmaxval, cb;
qmaxval = maxval * Q34 + C_QUANT;
if (qmaxval >= (FF_ARRAY_ELEMS(aac_maxval_cb)))
cb = 11;
else
cb = aac_maxval_cb[qmaxval];
return cb;
}

static inline float find_form_factor(int group_len, int swb_size, float thresh,
const float *scaled, float nzslope) {
const float iswb_size = 1.0f / swb_size;
const float iswb_sizem1 = 1.0f / (swb_size - 1);
const float ethresh = thresh;
float form = 0.0f, weight = 0.0f;
int w2, i;
for (w2 = 0; w2 < group_len; w2++) {
float e = 0.0f, e2 = 0.0f, var = 0.0f, maxval = 0.0f;
float nzl = 0;
for (i = 0; i < swb_size; i++) {
float s = fabsf(scaled[w2*128+i]);
maxval = FFMAX(maxval, s);
e += s;
e2 += s *= s;




if (s >= ethresh) {
nzl += 1.0f;
} else {
if (nzslope == 2.f)
nzl += (s / ethresh) * (s / ethresh);
else
nzl += ff_fast_powf(s / ethresh, nzslope);
}
}
if (e2 > thresh) {
float frm;
e *= iswb_size;


for (i = 0; i < swb_size; i++) {
float d = fabsf(scaled[w2*128+i]) - e;
var += d*d;
}
var = sqrtf(var * iswb_sizem1);

e2 *= iswb_size;
frm = e / FFMIN(e+4*var,maxval);
form += e2 * sqrtf(frm) / FFMAX(0.5f,nzl);
weight += e2;
}
}
if (weight > 0) {
return form / weight;
} else {
return 1.0f;
}
}


static inline uint8_t coef2minsf(float coef)
{
return av_clip_uint8(log2f(coef)*4 - 69 + SCALE_ONE_POS - SCALE_DIV_512);
}


static inline uint8_t coef2maxsf(float coef)
{
return av_clip_uint8(log2f(coef)*4 + 6 + SCALE_ONE_POS - SCALE_DIV_512);
}




static inline int quant_array_idx(const float val, const float *arr, const int num)
{
int i, index = 0;
float quant_min_err = INFINITY;
for (i = 0; i < num; i++) {
float error = (val - arr[i])*(val - arr[i]);
if (error < quant_min_err) {
quant_min_err = error;
index = i;
}
}
return index;
}




static av_always_inline float bval2bmax(float b)
{
return 0.001f + 0.0035f * (b*b*b) / (15.5f*15.5f*15.5f);
}







static inline void ff_init_nextband_map(const SingleChannelElement *sce, uint8_t *nextband)
{
unsigned char prevband = 0;
int w, g;

for (g = 0; g < 128; g++)
nextband[g] = g;


for (w = 0; w < sce->ics.num_windows; w += sce->ics.group_len[w]) {
for (g = 0; g < sce->ics.num_swb; g++) {
if (!sce->zeroes[w*16+g] && sce->band_type[w*16+g] < RESERVED_BT)
prevband = nextband[prevband] = w*16+g;
}
}
nextband[prevband] = prevband; 
}





static inline void ff_nextband_remove(uint8_t *nextband, int prevband, int band)
{
nextband[prevband] = nextband[band];
}







static inline int ff_sfdelta_can_remove_band(const SingleChannelElement *sce,
const uint8_t *nextband, int prev_sf, int band)
{
return prev_sf >= 0
&& sce->sf_idx[nextband[band]] >= (prev_sf - SCALE_MAX_DIFF)
&& sce->sf_idx[nextband[band]] <= (prev_sf + SCALE_MAX_DIFF);
}







static inline int ff_sfdelta_can_replace(const SingleChannelElement *sce,
const uint8_t *nextband, int prev_sf, int new_sf, int band)
{
return new_sf >= (prev_sf - SCALE_MAX_DIFF)
&& new_sf <= (prev_sf + SCALE_MAX_DIFF)
&& sce->sf_idx[nextband[band]] >= (new_sf - SCALE_MAX_DIFF)
&& sce->sf_idx[nextband[band]] <= (new_sf + SCALE_MAX_DIFF);
}








static av_always_inline int lcg_random(unsigned previous_val)
{
union { unsigned u; int s; } v = { previous_val * 1664525u + 1013904223 };
return v.s;
}

#define ERROR_IF(cond, ...) if (cond) { av_log(avctx, AV_LOG_ERROR, __VA_ARGS__); return AVERROR(EINVAL); }





#define WARN_IF(cond, ...) if (cond) { av_log(avctx, AV_LOG_WARNING, __VA_ARGS__); }




#endif 
