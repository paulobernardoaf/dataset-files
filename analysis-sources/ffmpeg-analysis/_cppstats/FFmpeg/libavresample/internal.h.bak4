



















#if !defined(AVRESAMPLE_INTERNAL_H)
#define AVRESAMPLE_INTERNAL_H

#include "libavutil/audio_fifo.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
#include "avresample.h"

typedef struct AudioData AudioData;
typedef struct AudioConvert AudioConvert;
typedef struct AudioMix AudioMix;
typedef struct ResampleContext ResampleContext;

enum RemapPoint {
REMAP_NONE,
REMAP_IN_COPY,
REMAP_IN_CONVERT,
REMAP_OUT_COPY,
REMAP_OUT_CONVERT,
};

typedef struct ChannelMapInfo {
int channel_map[AVRESAMPLE_MAX_CHANNELS]; 
int do_remap; 
int channel_copy[AVRESAMPLE_MAX_CHANNELS]; 
int do_copy; 
int channel_zero[AVRESAMPLE_MAX_CHANNELS]; 
int do_zero; 
int input_map[AVRESAMPLE_MAX_CHANNELS]; 
} ChannelMapInfo;

struct AVAudioResampleContext {
const AVClass *av_class; 

uint64_t in_channel_layout; 
enum AVSampleFormat in_sample_fmt; 
int in_sample_rate; 
uint64_t out_channel_layout; 
enum AVSampleFormat out_sample_fmt; 
int out_sample_rate; 
enum AVSampleFormat internal_sample_fmt; 
enum AVMixCoeffType mix_coeff_type; 
double center_mix_level; 
double surround_mix_level; 
double lfe_mix_level; 
int normalize_mix_level; 
int force_resampling; 
int filter_size; 
int phase_shift; 
int linear_interp; 
double cutoff; 
enum AVResampleFilterType filter_type; 
int kaiser_beta; 
enum AVResampleDitherMethod dither_method; 

int in_channels; 
int out_channels; 
int resample_channels; 
int downmix_needed; 
int upmix_needed; 
int mixing_needed; 
int resample_needed; 
int in_convert_needed; 
int out_convert_needed; 
int in_copy_needed; 

AudioData *in_buffer; 
AudioData *resample_out_buffer; 
AudioData *out_buffer; 
AVAudioFifo *out_fifo; 

AudioConvert *ac_in; 
AudioConvert *ac_out; 
ResampleContext *resample; 
AudioMix *am; 
enum AVMatrixEncoding matrix_encoding; 





double *mix_matrix;

int use_channel_map;
enum RemapPoint remap_point;
ChannelMapInfo ch_map_info;
};


void ff_audio_resample_init_aarch64(ResampleContext *c,
enum AVSampleFormat sample_fmt);
void ff_audio_resample_init_arm(ResampleContext *c,
enum AVSampleFormat sample_fmt);

#endif 
