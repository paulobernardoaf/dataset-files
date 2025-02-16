#include "swresample.h"
#include "libavutil/channel_layout.h"
#include "config.h"
#define SWR_CH_MAX 64
#define SQRT3_2 1.22474487139158904909 
#define NS_TAPS 20
#if ARCH_X86_64
typedef int64_t integer;
#else
typedef int integer;
#endif
typedef void (mix_1_1_func_type)(void *out, const void *in, void *coeffp, integer index, integer len);
typedef void (mix_2_1_func_type)(void *out, const void *in1, const void *in2, void *coeffp, integer index1, integer index2, integer len);
typedef void (mix_any_func_type)(uint8_t **out, const uint8_t **in1, void *coeffp, integer len);
typedef struct AudioData{
uint8_t *ch[SWR_CH_MAX]; 
uint8_t *data; 
int ch_count; 
int bps; 
int count; 
int planar; 
enum AVSampleFormat fmt; 
} AudioData;
struct DitherContext {
int method;
int noise_pos;
float scale;
float noise_scale; 
int ns_taps; 
float ns_scale; 
float ns_scale_1; 
int ns_pos; 
float ns_coeffs[NS_TAPS]; 
float ns_errors[SWR_CH_MAX][2*NS_TAPS];
AudioData noise; 
AudioData temp; 
int output_sample_bits; 
};
typedef struct ResampleContext * (* resample_init_func)(struct ResampleContext *c, int out_rate, int in_rate, int filter_size, int phase_shift, int linear,
double cutoff, enum AVSampleFormat format, enum SwrFilterType filter_type, double kaiser_beta, double precision, int cheby, int exact_rational);
typedef void (* resample_free_func)(struct ResampleContext **c);
typedef int (* multiple_resample_func)(struct ResampleContext *c, AudioData *dst, int dst_size, AudioData *src, int src_size, int *consumed);
typedef int (* resample_flush_func)(struct SwrContext *c);
typedef int (* set_compensation_func)(struct ResampleContext *c, int sample_delta, int compensation_distance);
typedef int64_t (* get_delay_func)(struct SwrContext *s, int64_t base);
typedef int (* invert_initial_buffer_func)(struct ResampleContext *c, AudioData *dst, const AudioData *src, int src_size, int *dst_idx, int *dst_count);
typedef int64_t (* get_out_samples_func)(struct SwrContext *s, int in_samples);
struct Resampler {
resample_init_func init;
resample_free_func free;
multiple_resample_func multiple_resample;
resample_flush_func flush;
set_compensation_func set_compensation;
get_delay_func get_delay;
invert_initial_buffer_func invert_initial_buffer;
get_out_samples_func get_out_samples;
};
extern struct Resampler const swri_resampler;
extern struct Resampler const swri_soxr_resampler;
struct SwrContext {
const AVClass *av_class; 
int log_level_offset; 
void *log_ctx; 
enum AVSampleFormat in_sample_fmt; 
enum AVSampleFormat int_sample_fmt; 
enum AVSampleFormat out_sample_fmt; 
int64_t in_ch_layout; 
int64_t out_ch_layout; 
int in_sample_rate; 
int out_sample_rate; 
int flags; 
float slev; 
float clev; 
float lfe_mix_level; 
float rematrix_volume; 
float rematrix_maxval; 
int matrix_encoding; 
const int *channel_map; 
int used_ch_count; 
int engine;
int user_in_ch_count; 
int user_out_ch_count; 
int user_used_ch_count; 
int64_t user_in_ch_layout; 
int64_t user_out_ch_layout; 
enum AVSampleFormat user_int_sample_fmt; 
int user_dither_method; 
struct DitherContext dither;
int filter_size; 
int phase_shift; 
int linear_interp; 
int exact_rational; 
double cutoff; 
int filter_type; 
double kaiser_beta; 
double precision; 
int cheby; 
float min_compensation; 
float min_hard_compensation; 
float soft_compensation_duration; 
float max_soft_compensation; 
float async; 
int64_t firstpts_in_samples; 
int resample_first; 
int rematrix; 
int rematrix_custom; 
AudioData in; 
AudioData postin; 
AudioData midbuf; 
AudioData preout; 
AudioData out; 
AudioData in_buffer; 
AudioData silence; 
AudioData drop_temp; 
int in_buffer_index; 
int in_buffer_count; 
int resample_in_constraint; 
int flushed; 
int64_t outpts; 
int64_t firstpts; 
int drop_output; 
double delayed_samples_fixup; 
struct AudioConvert *in_convert; 
struct AudioConvert *out_convert; 
struct AudioConvert *full_convert; 
struct ResampleContext *resample; 
struct Resampler const *resampler; 
double matrix[SWR_CH_MAX][SWR_CH_MAX]; 
float matrix_flt[SWR_CH_MAX][SWR_CH_MAX]; 
uint8_t *native_matrix;
uint8_t *native_one;
uint8_t *native_simd_one;
uint8_t *native_simd_matrix;
int32_t matrix32[SWR_CH_MAX][SWR_CH_MAX]; 
uint8_t matrix_ch[SWR_CH_MAX][SWR_CH_MAX+1]; 
mix_1_1_func_type *mix_1_1_f;
mix_1_1_func_type *mix_1_1_simd;
mix_2_1_func_type *mix_2_1_f;
mix_2_1_func_type *mix_2_1_simd;
mix_any_func_type *mix_any_f;
};
av_warn_unused_result
int swri_realloc_audio(AudioData *a, int count);
void swri_noise_shaping_int16 (SwrContext *s, AudioData *dsts, const AudioData *srcs, const AudioData *noises, int count);
void swri_noise_shaping_int32 (SwrContext *s, AudioData *dsts, const AudioData *srcs, const AudioData *noises, int count);
void swri_noise_shaping_float (SwrContext *s, AudioData *dsts, const AudioData *srcs, const AudioData *noises, int count);
void swri_noise_shaping_double(SwrContext *s, AudioData *dsts, const AudioData *srcs, const AudioData *noises, int count);
av_warn_unused_result
int swri_rematrix_init(SwrContext *s);
void swri_rematrix_free(SwrContext *s);
int swri_rematrix(SwrContext *s, AudioData *out, AudioData *in, int len, int mustcopy);
int swri_rematrix_init_x86(struct SwrContext *s);
av_warn_unused_result
int swri_get_dither(SwrContext *s, void *dst, int len, unsigned seed, enum AVSampleFormat noise_fmt);
av_warn_unused_result
int swri_dither_init(SwrContext *s, enum AVSampleFormat out_fmt, enum AVSampleFormat in_fmt);
void swri_audio_convert_init_aarch64(struct AudioConvert *ac,
enum AVSampleFormat out_fmt,
enum AVSampleFormat in_fmt,
int channels);
void swri_audio_convert_init_arm(struct AudioConvert *ac,
enum AVSampleFormat out_fmt,
enum AVSampleFormat in_fmt,
int channels);
void swri_audio_convert_init_x86(struct AudioConvert *ac,
enum AVSampleFormat out_fmt,
enum AVSampleFormat in_fmt,
int channels);
