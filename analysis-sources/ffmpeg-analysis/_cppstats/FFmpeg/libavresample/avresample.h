#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/dict.h"
#include "libavutil/frame.h"
#include "libavutil/log.h"
#include "libavutil/mathematics.h"
#include "libavresample/version.h"
#define AVRESAMPLE_MAX_CHANNELS 32
typedef struct AVAudioResampleContext AVAudioResampleContext;
enum attribute_deprecated AVMixCoeffType {
AV_MIX_COEFF_TYPE_Q8, 
AV_MIX_COEFF_TYPE_Q15, 
AV_MIX_COEFF_TYPE_FLT, 
AV_MIX_COEFF_TYPE_NB, 
};
enum attribute_deprecated AVResampleFilterType {
AV_RESAMPLE_FILTER_TYPE_CUBIC, 
AV_RESAMPLE_FILTER_TYPE_BLACKMAN_NUTTALL, 
AV_RESAMPLE_FILTER_TYPE_KAISER, 
};
enum attribute_deprecated AVResampleDitherMethod {
AV_RESAMPLE_DITHER_NONE, 
AV_RESAMPLE_DITHER_RECTANGULAR, 
AV_RESAMPLE_DITHER_TRIANGULAR, 
AV_RESAMPLE_DITHER_TRIANGULAR_HP, 
AV_RESAMPLE_DITHER_TRIANGULAR_NS, 
AV_RESAMPLE_DITHER_NB, 
};
attribute_deprecated
unsigned avresample_version(void);
attribute_deprecated
const char *avresample_configuration(void);
attribute_deprecated
const char *avresample_license(void);
attribute_deprecated
const AVClass *avresample_get_class(void);
attribute_deprecated
AVAudioResampleContext *avresample_alloc_context(void);
attribute_deprecated
int avresample_open(AVAudioResampleContext *avr);
attribute_deprecated
int avresample_is_open(AVAudioResampleContext *avr);
attribute_deprecated
void avresample_close(AVAudioResampleContext *avr);
attribute_deprecated
void avresample_free(AVAudioResampleContext **avr);
attribute_deprecated
int avresample_build_matrix(uint64_t in_layout, uint64_t out_layout,
double center_mix_level, double surround_mix_level,
double lfe_mix_level, int normalize, double *matrix,
int stride, enum AVMatrixEncoding matrix_encoding);
attribute_deprecated
int avresample_get_matrix(AVAudioResampleContext *avr, double *matrix,
int stride);
attribute_deprecated
int avresample_set_matrix(AVAudioResampleContext *avr, const double *matrix,
int stride);
attribute_deprecated
int avresample_set_channel_mapping(AVAudioResampleContext *avr,
const int *channel_map);
attribute_deprecated
int avresample_set_compensation(AVAudioResampleContext *avr, int sample_delta,
int compensation_distance);
attribute_deprecated
int avresample_get_out_samples(AVAudioResampleContext *avr, int in_nb_samples);
attribute_deprecated
int avresample_convert(AVAudioResampleContext *avr, uint8_t **output,
int out_plane_size, int out_samples,
uint8_t * const *input, int in_plane_size,
int in_samples);
attribute_deprecated
int avresample_get_delay(AVAudioResampleContext *avr);
attribute_deprecated
int avresample_available(AVAudioResampleContext *avr);
attribute_deprecated
int avresample_read(AVAudioResampleContext *avr, uint8_t **output, int nb_samples);
attribute_deprecated
int avresample_convert_frame(AVAudioResampleContext *avr,
AVFrame *output, AVFrame *input);
attribute_deprecated
int avresample_config(AVAudioResampleContext *avr, AVFrame *out, AVFrame *in);
