#include "fft.h"
#include "dcadct.h"
typedef struct SynthFilterContext {
void (*synth_filter_float)(FFTContext *imdct,
float *synth_buf_ptr, int *synth_buf_offset,
float synth_buf2[32], const float window[512],
float out[32], const float in[32],
float scale);
void (*synth_filter_float_64)(FFTContext *imdct,
float *synth_buf_ptr, int *synth_buf_offset,
float synth_buf2[64], const float window[1024],
float out[64], const float in[64], float scale);
void (*synth_filter_fixed)(DCADCTContext *imdct,
int32_t *synth_buf_ptr, int *synth_buf_offset,
int32_t synth_buf2[32], const int32_t window[512],
int32_t out[32], const int32_t in[32]);
void (*synth_filter_fixed_64)(DCADCTContext *imdct,
int32_t *synth_buf_ptr, int *synth_buf_offset,
int32_t synth_buf2[64], const int32_t window[1024],
int32_t out[64], const int32_t in[64]);
} SynthFilterContext;
void ff_synth_filter_init(SynthFilterContext *c);
void ff_synth_filter_init_aarch64(SynthFilterContext *c);
void ff_synth_filter_init_arm(SynthFilterContext *c);
void ff_synth_filter_init_x86(SynthFilterContext *c);
