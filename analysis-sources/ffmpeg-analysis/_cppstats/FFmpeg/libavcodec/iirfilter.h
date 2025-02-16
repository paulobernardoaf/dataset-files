#include <stddef.h>
#include <stdint.h>
struct FFIIRFilterCoeffs;
struct FFIIRFilterState;
enum IIRFilterType{
FF_FILTER_TYPE_BESSEL,
FF_FILTER_TYPE_BIQUAD,
FF_FILTER_TYPE_BUTTERWORTH,
FF_FILTER_TYPE_CHEBYSHEV,
FF_FILTER_TYPE_ELLIPTIC,
};
enum IIRFilterMode{
FF_FILTER_MODE_LOWPASS,
FF_FILTER_MODE_HIGHPASS,
FF_FILTER_MODE_BANDPASS,
FF_FILTER_MODE_BANDSTOP,
};
typedef struct FFIIRFilterContext {
void (*filter_flt)(const struct FFIIRFilterCoeffs *coeffs,
struct FFIIRFilterState *state, int size,
const float *src, ptrdiff_t sstep, float *dst, ptrdiff_t dstep);
} FFIIRFilterContext;
void ff_iir_filter_init(FFIIRFilterContext *f);
void ff_iir_filter_init_mips(FFIIRFilterContext *f);
struct FFIIRFilterCoeffs* ff_iir_filter_init_coeffs(void *avc,
enum IIRFilterType filt_type,
enum IIRFilterMode filt_mode,
int order, float cutoff_ratio,
float stopband, float ripple);
struct FFIIRFilterState* ff_iir_filter_init_state(int order);
void ff_iir_filter_free_coeffsp(struct FFIIRFilterCoeffs **coeffs);
void ff_iir_filter_free_statep(struct FFIIRFilterState **state);
void ff_iir_filter(const struct FFIIRFilterCoeffs *coeffs, struct FFIIRFilterState *state,
int size, const int16_t *src, ptrdiff_t sstep, int16_t *dst, ptrdiff_t dstep);
void ff_iir_filter_flt(const struct FFIIRFilterCoeffs *coeffs,
struct FFIIRFilterState *state, int size,
const float *src, ptrdiff_t sstep,
float *dst, ptrdiff_t dstep);
