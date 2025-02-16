


























#if !defined(AVCODEC_AACENC_QUANTIZATION_MISC_H)
#define AVCODEC_AACENC_QUANTIZATION_MISC_H

static inline float quantize_band_cost_cached(struct AACEncContext *s, int w, int g, const float *in,
const float *scaled, int size, int scale_idx,
int cb, const float lambda, const float uplim,
int *bits, float *energy, int rtz)
{
AACQuantizeBandCostCacheEntry *entry;
av_assert1(scale_idx >= 0 && scale_idx < 256);
entry = &s->quantize_band_cost_cache[scale_idx][w*16+g];
if (entry->generation != s->quantize_band_cost_cache_generation || entry->cb != cb || entry->rtz != rtz) {
entry->rd = quantize_band_cost(s, in, scaled, size, scale_idx,
cb, lambda, uplim, &entry->bits, &entry->energy, rtz);
entry->cb = cb;
entry->rtz = rtz;
entry->generation = s->quantize_band_cost_cache_generation;
}
if (bits)
*bits = entry->bits;
if (energy)
*energy = entry->energy;
return entry->rd;
}

#endif 
