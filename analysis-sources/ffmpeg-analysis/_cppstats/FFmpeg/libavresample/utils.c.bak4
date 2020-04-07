



















#include "libavutil/common.h"
#include "libavutil/dict.h"

#include "libavutil/frame.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"

#include "avresample.h"
#include "internal.h"
#include "audio_data.h"
#include "audio_convert.h"
#include "audio_mix.h"
#include "resample.h"

int avresample_open(AVAudioResampleContext *avr)
{
int ret;

if (avresample_is_open(avr)) {
av_log(avr, AV_LOG_ERROR, "The resampling context is already open.\n");
return AVERROR(EINVAL);
}


avr->in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
if (avr->in_channels <= 0 || avr->in_channels > AVRESAMPLE_MAX_CHANNELS) {
av_log(avr, AV_LOG_ERROR, "Invalid input channel layout: %"PRIu64"\n",
avr->in_channel_layout);
return AVERROR(EINVAL);
}
avr->out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);
if (avr->out_channels <= 0 || avr->out_channels > AVRESAMPLE_MAX_CHANNELS) {
av_log(avr, AV_LOG_ERROR, "Invalid output channel layout: %"PRIu64"\n",
avr->out_channel_layout);
return AVERROR(EINVAL);
}
avr->resample_channels = FFMIN(avr->in_channels, avr->out_channels);
avr->downmix_needed = avr->in_channels > avr->out_channels;
avr->upmix_needed = avr->out_channels > avr->in_channels ||
(!avr->downmix_needed && (avr->mix_matrix ||
avr->in_channel_layout != avr->out_channel_layout));
avr->mixing_needed = avr->downmix_needed || avr->upmix_needed;


avr->resample_needed = avr->in_sample_rate != avr->out_sample_rate ||
avr->force_resampling;


if (avr->internal_sample_fmt == AV_SAMPLE_FMT_NONE &&
(avr->mixing_needed || avr->resample_needed)) {
enum AVSampleFormat in_fmt = av_get_planar_sample_fmt(avr->in_sample_fmt);
enum AVSampleFormat out_fmt = av_get_planar_sample_fmt(avr->out_sample_fmt);
int max_bps = FFMAX(av_get_bytes_per_sample(in_fmt),
av_get_bytes_per_sample(out_fmt));
if (max_bps <= 2) {
avr->internal_sample_fmt = AV_SAMPLE_FMT_S16P;
} else if (avr->mixing_needed) {
avr->internal_sample_fmt = AV_SAMPLE_FMT_FLTP;
} else {
if (max_bps <= 4) {
if (in_fmt == AV_SAMPLE_FMT_S32P ||
out_fmt == AV_SAMPLE_FMT_S32P) {
if (in_fmt == AV_SAMPLE_FMT_FLTP ||
out_fmt == AV_SAMPLE_FMT_FLTP) {

avr->internal_sample_fmt = AV_SAMPLE_FMT_DBLP;
} else {

avr->internal_sample_fmt = AV_SAMPLE_FMT_S32P;
}
} else {

avr->internal_sample_fmt = AV_SAMPLE_FMT_FLTP;
}
} else {

avr->internal_sample_fmt = AV_SAMPLE_FMT_DBLP;
}
}
av_log(avr, AV_LOG_DEBUG, "Using %s as internal sample format\n",
av_get_sample_fmt_name(avr->internal_sample_fmt));
}



if (avr->use_channel_map && !avr->mixing_needed && !avr->resample_needed &&
!ff_sample_fmt_is_planar(avr->out_sample_fmt, avr->out_channels)) {
avr->internal_sample_fmt = av_get_planar_sample_fmt(avr->out_sample_fmt);
}


if (avr->resample_needed || avr->mixing_needed)
avr->in_convert_needed = avr->in_sample_fmt != avr->internal_sample_fmt;
else
avr->in_convert_needed = avr->use_channel_map &&
!ff_sample_fmt_is_planar(avr->out_sample_fmt, avr->out_channels);

if (avr->resample_needed || avr->mixing_needed || avr->in_convert_needed)
avr->out_convert_needed = avr->internal_sample_fmt != avr->out_sample_fmt;
else
avr->out_convert_needed = avr->in_sample_fmt != avr->out_sample_fmt;

avr->in_copy_needed = !avr->in_convert_needed && (avr->mixing_needed ||
(avr->use_channel_map && avr->resample_needed));

if (avr->use_channel_map) {
if (avr->in_copy_needed) {
avr->remap_point = REMAP_IN_COPY;
av_log(avr, AV_LOG_TRACE, "remap channels during in_copy\n");
} else if (avr->in_convert_needed) {
avr->remap_point = REMAP_IN_CONVERT;
av_log(avr, AV_LOG_TRACE, "remap channels during in_convert\n");
} else if (avr->out_convert_needed) {
avr->remap_point = REMAP_OUT_CONVERT;
av_log(avr, AV_LOG_TRACE, "remap channels during out_convert\n");
} else {
avr->remap_point = REMAP_OUT_COPY;
av_log(avr, AV_LOG_TRACE, "remap channels during out_copy\n");
}

#if defined(DEBUG)
{
int ch;
av_log(avr, AV_LOG_TRACE, "output map: ");
if (avr->ch_map_info.do_remap)
for (ch = 0; ch < avr->in_channels; ch++)
av_log(avr, AV_LOG_TRACE, " % 2d", avr->ch_map_info.channel_map[ch]);
else
av_log(avr, AV_LOG_TRACE, "n/a");
av_log(avr, AV_LOG_TRACE, "\n");
av_log(avr, AV_LOG_TRACE, "copy map: ");
if (avr->ch_map_info.do_copy)
for (ch = 0; ch < avr->in_channels; ch++)
av_log(avr, AV_LOG_TRACE, " % 2d", avr->ch_map_info.channel_copy[ch]);
else
av_log(avr, AV_LOG_TRACE, "n/a");
av_log(avr, AV_LOG_TRACE, "\n");
av_log(avr, AV_LOG_TRACE, "zero map: ");
if (avr->ch_map_info.do_zero)
for (ch = 0; ch < avr->in_channels; ch++)
av_log(avr, AV_LOG_TRACE, " % 2d", avr->ch_map_info.channel_zero[ch]);
else
av_log(avr, AV_LOG_TRACE, "n/a");
av_log(avr, AV_LOG_TRACE, "\n");
av_log(avr, AV_LOG_TRACE, "input map: ");
for (ch = 0; ch < avr->in_channels; ch++)
av_log(avr, AV_LOG_TRACE, " % 2d", avr->ch_map_info.input_map[ch]);
av_log(avr, AV_LOG_TRACE, "\n");
}
#endif
} else
avr->remap_point = REMAP_NONE;


if (avr->in_copy_needed || avr->in_convert_needed) {
avr->in_buffer = ff_audio_data_alloc(FFMAX(avr->in_channels, avr->out_channels),
0, avr->internal_sample_fmt,
"in_buffer");
if (!avr->in_buffer) {
ret = AVERROR(EINVAL);
goto error;
}
}
if (avr->resample_needed) {
avr->resample_out_buffer = ff_audio_data_alloc(avr->out_channels,
1024, avr->internal_sample_fmt,
"resample_out_buffer");
if (!avr->resample_out_buffer) {
ret = AVERROR(EINVAL);
goto error;
}
}
if (avr->out_convert_needed) {
avr->out_buffer = ff_audio_data_alloc(avr->out_channels, 0,
avr->out_sample_fmt, "out_buffer");
if (!avr->out_buffer) {
ret = AVERROR(EINVAL);
goto error;
}
}
avr->out_fifo = av_audio_fifo_alloc(avr->out_sample_fmt, avr->out_channels,
1024);
if (!avr->out_fifo) {
ret = AVERROR(ENOMEM);
goto error;
}


if (avr->in_convert_needed) {
avr->ac_in = ff_audio_convert_alloc(avr, avr->internal_sample_fmt,
avr->in_sample_fmt, avr->in_channels,
avr->in_sample_rate,
avr->remap_point == REMAP_IN_CONVERT);
if (!avr->ac_in) {
ret = AVERROR(ENOMEM);
goto error;
}
}
if (avr->out_convert_needed) {
enum AVSampleFormat src_fmt;
if (avr->in_convert_needed)
src_fmt = avr->internal_sample_fmt;
else
src_fmt = avr->in_sample_fmt;
avr->ac_out = ff_audio_convert_alloc(avr, avr->out_sample_fmt, src_fmt,
avr->out_channels,
avr->out_sample_rate,
avr->remap_point == REMAP_OUT_CONVERT);
if (!avr->ac_out) {
ret = AVERROR(ENOMEM);
goto error;
}
}
if (avr->resample_needed) {
avr->resample = ff_audio_resample_init(avr);
if (!avr->resample) {
ret = AVERROR(ENOMEM);
goto error;
}
}
if (avr->mixing_needed) {
avr->am = ff_audio_mix_alloc(avr);
if (!avr->am) {
ret = AVERROR(ENOMEM);
goto error;
}
}

return 0;

error:
avresample_close(avr);
return ret;
}

int avresample_is_open(AVAudioResampleContext *avr)
{
return !!avr->out_fifo;
}

void avresample_close(AVAudioResampleContext *avr)
{
ff_audio_data_free(&avr->in_buffer);
ff_audio_data_free(&avr->resample_out_buffer);
ff_audio_data_free(&avr->out_buffer);
av_audio_fifo_free(avr->out_fifo);
avr->out_fifo = NULL;
ff_audio_convert_free(&avr->ac_in);
ff_audio_convert_free(&avr->ac_out);
ff_audio_resample_free(&avr->resample);
ff_audio_mix_free(&avr->am);
av_freep(&avr->mix_matrix);

avr->use_channel_map = 0;
}

void avresample_free(AVAudioResampleContext **avr)
{
if (!*avr)
return;
avresample_close(*avr);
av_opt_free(*avr);
av_freep(avr);
}

static int handle_buffered_output(AVAudioResampleContext *avr,
AudioData *output, AudioData *converted)
{
int ret;

if (!output || av_audio_fifo_size(avr->out_fifo) > 0 ||
(converted && output->allocated_samples < converted->nb_samples)) {
if (converted) {



av_log(avr, AV_LOG_TRACE, "[FIFO] add %s to out_fifo\n", converted->name);
ret = ff_audio_data_add_to_fifo(avr->out_fifo, converted, 0,
converted->nb_samples);
if (ret < 0)
return ret;
}



if (output && output->allocated_samples > 0) {
av_log(avr, AV_LOG_TRACE, "[FIFO] read from out_fifo to output\n");
av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return ff_audio_data_read_from_fifo(avr->out_fifo, output,
output->allocated_samples);
}
} else if (converted) {


av_log(avr, AV_LOG_TRACE, "[copy] %s to output\n", converted->name);
output->nb_samples = 0;
ret = ff_audio_data_copy(output, converted,
avr->remap_point == REMAP_OUT_COPY ?
&avr->ch_map_info : NULL);
if (ret < 0)
return ret;
av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return output->nb_samples;
}
av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return 0;
}

int attribute_align_arg avresample_convert(AVAudioResampleContext *avr,
uint8_t **output, int out_plane_size,
int out_samples,
uint8_t * const *input,
int in_plane_size, int in_samples)
{
AudioData input_buffer;
AudioData output_buffer;
AudioData *current_buffer;
int ret, direct_output;


if (avr->in_buffer) {
avr->in_buffer->nb_samples = 0;
ff_audio_data_set_channels(avr->in_buffer,
avr->in_buffer->allocated_channels);
}
if (avr->resample_out_buffer) {
avr->resample_out_buffer->nb_samples = 0;
ff_audio_data_set_channels(avr->resample_out_buffer,
avr->resample_out_buffer->allocated_channels);
}
if (avr->out_buffer) {
avr->out_buffer->nb_samples = 0;
ff_audio_data_set_channels(avr->out_buffer,
avr->out_buffer->allocated_channels);
}

av_log(avr, AV_LOG_TRACE, "[start conversion]\n");


direct_output = output && av_audio_fifo_size(avr->out_fifo) == 0;
if (output) {
ret = ff_audio_data_init(&output_buffer, output, out_plane_size,
avr->out_channels, out_samples,
avr->out_sample_fmt, 0, "output");
if (ret < 0)
return ret;
output_buffer.nb_samples = 0;
}

if (input) {

ret = ff_audio_data_init(&input_buffer, input, in_plane_size,
avr->in_channels, in_samples,
avr->in_sample_fmt, 1, "input");
if (ret < 0)
return ret;
current_buffer = &input_buffer;

if (avr->upmix_needed && !avr->in_convert_needed && !avr->resample_needed &&
!avr->out_convert_needed && direct_output && out_samples >= in_samples) {


av_log(avr, AV_LOG_TRACE, "[copy] %s to output\n", current_buffer->name);
ret = ff_audio_data_copy(&output_buffer, current_buffer,
avr->remap_point == REMAP_OUT_COPY ?
&avr->ch_map_info : NULL);
if (ret < 0)
return ret;
current_buffer = &output_buffer;
} else if (avr->remap_point == REMAP_OUT_COPY &&
(!direct_output || out_samples < in_samples)) {



av_log(avr, AV_LOG_TRACE, "[copy] %s to out_buffer\n", current_buffer->name);
ret = ff_audio_data_copy(avr->out_buffer, current_buffer,
&avr->ch_map_info);
if (ret < 0)
return ret;
current_buffer = avr->out_buffer;
} else if (avr->in_copy_needed || avr->in_convert_needed) {


if (avr->in_convert_needed) {
ret = ff_audio_data_realloc(avr->in_buffer,
current_buffer->nb_samples);
if (ret < 0)
return ret;
av_log(avr, AV_LOG_TRACE, "[convert] %s to in_buffer\n", current_buffer->name);
ret = ff_audio_convert(avr->ac_in, avr->in_buffer,
current_buffer);
if (ret < 0)
return ret;
} else {
av_log(avr, AV_LOG_TRACE, "[copy] %s to in_buffer\n", current_buffer->name);
ret = ff_audio_data_copy(avr->in_buffer, current_buffer,
avr->remap_point == REMAP_IN_COPY ?
&avr->ch_map_info : NULL);
if (ret < 0)
return ret;
}
ff_audio_data_set_channels(avr->in_buffer, avr->in_channels);
if (avr->downmix_needed) {
av_log(avr, AV_LOG_TRACE, "[downmix] in_buffer\n");
ret = ff_audio_mix(avr->am, avr->in_buffer);
if (ret < 0)
return ret;
}
current_buffer = avr->in_buffer;
}
} else {

if (!avr->resample_needed)
return handle_buffered_output(avr, output ? &output_buffer : NULL,
NULL);
current_buffer = NULL;
}

if (avr->resample_needed) {
AudioData *resample_out;

if (!avr->out_convert_needed && direct_output && out_samples > 0)
resample_out = &output_buffer;
else
resample_out = avr->resample_out_buffer;
av_log(avr, AV_LOG_TRACE, "[resample] %s to %s\n",
current_buffer ? current_buffer->name : "null",
resample_out->name);
ret = ff_audio_resample(avr->resample, resample_out,
current_buffer);
if (ret < 0)
return ret;


if (resample_out->nb_samples == 0) {
av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return 0;
}

current_buffer = resample_out;
}

if (avr->upmix_needed) {
av_log(avr, AV_LOG_TRACE, "[upmix] %s\n", current_buffer->name);
ret = ff_audio_mix(avr->am, current_buffer);
if (ret < 0)
return ret;
}


if (current_buffer == &output_buffer) {
av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return current_buffer->nb_samples;
}

if (avr->out_convert_needed) {
if (direct_output && out_samples >= current_buffer->nb_samples) {

av_log(avr, AV_LOG_TRACE, "[convert] %s to output\n", current_buffer->name);
ret = ff_audio_convert(avr->ac_out, &output_buffer, current_buffer);
if (ret < 0)
return ret;

av_log(avr, AV_LOG_TRACE, "[end conversion]\n");
return output_buffer.nb_samples;
} else {
ret = ff_audio_data_realloc(avr->out_buffer,
current_buffer->nb_samples);
if (ret < 0)
return ret;
av_log(avr, AV_LOG_TRACE, "[convert] %s to out_buffer\n", current_buffer->name);
ret = ff_audio_convert(avr->ac_out, avr->out_buffer,
current_buffer);
if (ret < 0)
return ret;
current_buffer = avr->out_buffer;
}
}

return handle_buffered_output(avr, output ? &output_buffer : NULL,
current_buffer);
}

int avresample_config(AVAudioResampleContext *avr, AVFrame *out, AVFrame *in)
{
if (avresample_is_open(avr)) {
avresample_close(avr);
}

if (in) {
avr->in_channel_layout = in->channel_layout;
avr->in_sample_rate = in->sample_rate;
avr->in_sample_fmt = in->format;
}

if (out) {
avr->out_channel_layout = out->channel_layout;
avr->out_sample_rate = out->sample_rate;
avr->out_sample_fmt = out->format;
}

return 0;
}

static int config_changed(AVAudioResampleContext *avr,
AVFrame *out, AVFrame *in)
{
int ret = 0;

if (in) {
if (avr->in_channel_layout != in->channel_layout ||
avr->in_sample_rate != in->sample_rate ||
avr->in_sample_fmt != in->format) {
ret |= AVERROR_INPUT_CHANGED;
}
}

if (out) {
if (avr->out_channel_layout != out->channel_layout ||
avr->out_sample_rate != out->sample_rate ||
avr->out_sample_fmt != out->format) {
ret |= AVERROR_OUTPUT_CHANGED;
}
}

return ret;
}

static inline int convert_frame(AVAudioResampleContext *avr,
AVFrame *out, AVFrame *in)
{
int ret;
uint8_t **out_data = NULL, **in_data = NULL;
int out_linesize = 0, in_linesize = 0;
int out_nb_samples = 0, in_nb_samples = 0;

if (out) {
out_data = out->extended_data;
out_linesize = out->linesize[0];
out_nb_samples = out->nb_samples;
}

if (in) {
in_data = in->extended_data;
in_linesize = in->linesize[0];
in_nb_samples = in->nb_samples;
}

ret = avresample_convert(avr, out_data, out_linesize,
out_nb_samples,
in_data, in_linesize,
in_nb_samples);

if (ret < 0) {
if (out)
out->nb_samples = 0;
return ret;
}

if (out)
out->nb_samples = ret;

return 0;
}

static inline int available_samples(AVFrame *out)
{
int samples;
int bytes_per_sample = av_get_bytes_per_sample(out->format);
if (!bytes_per_sample)
return AVERROR(EINVAL);

samples = out->linesize[0] / bytes_per_sample;
if (av_sample_fmt_is_planar(out->format)) {
return samples;
} else {
int channels = av_get_channel_layout_nb_channels(out->channel_layout);
return samples / channels;
}
}

int avresample_convert_frame(AVAudioResampleContext *avr,
AVFrame *out, AVFrame *in)
{
int ret, setup = 0;

if (!avresample_is_open(avr)) {
if ((ret = avresample_config(avr, out, in)) < 0)
return ret;
if ((ret = avresample_open(avr)) < 0)
return ret;
setup = 1;
} else {

if ((ret = config_changed(avr, out, in)))
return ret;
}

if (out) {
if (!out->linesize[0]) {
out->nb_samples = avresample_get_out_samples(avr, in->nb_samples);
if ((ret = av_frame_get_buffer(out, 0)) < 0) {
if (setup)
avresample_close(avr);
return ret;
}
} else {
if (!out->nb_samples)
out->nb_samples = available_samples(out);
}
}

return convert_frame(avr, out, in);
}

int avresample_get_matrix(AVAudioResampleContext *avr, double *matrix,
int stride)
{
int in_channels, out_channels, i, o;

if (avr->am)
return ff_audio_mix_get_matrix(avr->am, matrix, stride);

in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);

if ( in_channels <= 0 || in_channels > AVRESAMPLE_MAX_CHANNELS ||
out_channels <= 0 || out_channels > AVRESAMPLE_MAX_CHANNELS) {
av_log(avr, AV_LOG_ERROR, "Invalid channel layouts\n");
return AVERROR(EINVAL);
}

if (!avr->mix_matrix) {
av_log(avr, AV_LOG_ERROR, "matrix is not set\n");
return AVERROR(EINVAL);
}

for (o = 0; o < out_channels; o++)
for (i = 0; i < in_channels; i++)
matrix[o * stride + i] = avr->mix_matrix[o * in_channels + i];

return 0;
}

int avresample_set_matrix(AVAudioResampleContext *avr, const double *matrix,
int stride)
{
int in_channels, out_channels, i, o;

if (avr->am)
return ff_audio_mix_set_matrix(avr->am, matrix, stride);

in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
out_channels = av_get_channel_layout_nb_channels(avr->out_channel_layout);

if ( in_channels <= 0 || in_channels > AVRESAMPLE_MAX_CHANNELS ||
out_channels <= 0 || out_channels > AVRESAMPLE_MAX_CHANNELS) {
av_log(avr, AV_LOG_ERROR, "Invalid channel layouts\n");
return AVERROR(EINVAL);
}

if (avr->mix_matrix)
av_freep(&avr->mix_matrix);
avr->mix_matrix = av_malloc(in_channels * out_channels *
sizeof(*avr->mix_matrix));
if (!avr->mix_matrix)
return AVERROR(ENOMEM);

for (o = 0; o < out_channels; o++)
for (i = 0; i < in_channels; i++)
avr->mix_matrix[o * in_channels + i] = matrix[o * stride + i];

return 0;
}

int avresample_set_channel_mapping(AVAudioResampleContext *avr,
const int *channel_map)
{
ChannelMapInfo *info = &avr->ch_map_info;
int in_channels, ch, i;

in_channels = av_get_channel_layout_nb_channels(avr->in_channel_layout);
if (in_channels <= 0 || in_channels > AVRESAMPLE_MAX_CHANNELS) {
av_log(avr, AV_LOG_ERROR, "Invalid input channel layout\n");
return AVERROR(EINVAL);
}

memset(info, 0, sizeof(*info));
memset(info->input_map, -1, sizeof(info->input_map));

for (ch = 0; ch < in_channels; ch++) {
if (channel_map[ch] >= in_channels) {
av_log(avr, AV_LOG_ERROR, "Invalid channel map\n");
return AVERROR(EINVAL);
}
if (channel_map[ch] < 0) {
info->channel_zero[ch] = 1;
info->channel_map[ch] = -1;
info->do_zero = 1;
} else if (info->input_map[channel_map[ch]] >= 0) {
info->channel_copy[ch] = info->input_map[channel_map[ch]];
info->channel_map[ch] = -1;
info->do_copy = 1;
} else {
info->channel_map[ch] = channel_map[ch];
info->input_map[channel_map[ch]] = ch;
info->do_remap = 1;
}
}



for (ch = 0, i = 0; ch < in_channels && i < in_channels; ch++, i++) {
while (ch < in_channels && info->input_map[ch] >= 0)
ch++;
while (i < in_channels && info->channel_map[i] >= 0)
i++;
if (ch >= in_channels || i >= in_channels)
break;
info->input_map[ch] = i;
}

avr->use_channel_map = 1;
return 0;
}

int avresample_available(AVAudioResampleContext *avr)
{
return av_audio_fifo_size(avr->out_fifo);
}

int avresample_get_out_samples(AVAudioResampleContext *avr, int in_nb_samples)
{
int64_t samples = avresample_get_delay(avr) + (int64_t)in_nb_samples;

if (avr->resample_needed) {
samples = av_rescale_rnd(samples,
avr->out_sample_rate,
avr->in_sample_rate,
AV_ROUND_UP);
}

samples += avresample_available(avr);

if (samples > INT_MAX)
return AVERROR(EINVAL);

return samples;
}

int avresample_read(AVAudioResampleContext *avr, uint8_t **output, int nb_samples)
{
if (!output)
return av_audio_fifo_drain(avr->out_fifo, nb_samples);
return av_audio_fifo_read(avr->out_fifo, (void**)output, nb_samples);
}

unsigned avresample_version(void)
{
return LIBAVRESAMPLE_VERSION_INT;
}

const char *avresample_license(void)
{
#define LICENSE_PREFIX "libavresample license: "
return LICENSE_PREFIX FFMPEG_LICENSE + sizeof(LICENSE_PREFIX) - 1;
}

const char *avresample_configuration(void)
{
return FFMPEG_CONFIGURATION;
}
