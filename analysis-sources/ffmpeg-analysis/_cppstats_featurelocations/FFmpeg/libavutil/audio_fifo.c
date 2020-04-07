

























#include "avutil.h"
#include "audio_fifo.h"
#include "common.h"
#include "fifo.h"
#include "mem.h"
#include "samplefmt.h"

struct AVAudioFifo {
AVFifoBuffer **buf; 
int nb_buffers; 
int nb_samples; 
int allocated_samples; 

int channels; 
enum AVSampleFormat sample_fmt; 
int sample_size; 
};

void av_audio_fifo_free(AVAudioFifo *af)
{
if (af) {
if (af->buf) {
int i;
for (i = 0; i < af->nb_buffers; i++) {
av_fifo_freep(&af->buf[i]);
}
av_freep(&af->buf);
}
av_free(af);
}
}

AVAudioFifo *av_audio_fifo_alloc(enum AVSampleFormat sample_fmt, int channels,
int nb_samples)
{
AVAudioFifo *af;
int buf_size, i;


if (av_samples_get_buffer_size(&buf_size, channels, nb_samples, sample_fmt, 1) < 0)
return NULL;

af = av_mallocz(sizeof(*af));
if (!af)
return NULL;

af->channels = channels;
af->sample_fmt = sample_fmt;
af->sample_size = buf_size / nb_samples;
af->nb_buffers = av_sample_fmt_is_planar(sample_fmt) ? channels : 1;

af->buf = av_mallocz_array(af->nb_buffers, sizeof(*af->buf));
if (!af->buf)
goto error;

for (i = 0; i < af->nb_buffers; i++) {
af->buf[i] = av_fifo_alloc(buf_size);
if (!af->buf[i])
goto error;
}
af->allocated_samples = nb_samples;

return af;

error:
av_audio_fifo_free(af);
return NULL;
}

int av_audio_fifo_realloc(AVAudioFifo *af, int nb_samples)
{
int i, ret, buf_size;

if ((ret = av_samples_get_buffer_size(&buf_size, af->channels, nb_samples,
af->sample_fmt, 1)) < 0)
return ret;

for (i = 0; i < af->nb_buffers; i++) {
if ((ret = av_fifo_realloc2(af->buf[i], buf_size)) < 0)
return ret;
}
af->allocated_samples = nb_samples;
return 0;
}

int av_audio_fifo_write(AVAudioFifo *af, void **data, int nb_samples)
{
int i, ret, size;


if (av_audio_fifo_space(af) < nb_samples) {
int current_size = av_audio_fifo_size(af);

if (INT_MAX / 2 - current_size < nb_samples)
return AVERROR(EINVAL);

if ((ret = av_audio_fifo_realloc(af, 2 * (current_size + nb_samples))) < 0)
return ret;
}

size = nb_samples * af->sample_size;
for (i = 0; i < af->nb_buffers; i++) {
ret = av_fifo_generic_write(af->buf[i], data[i], size, NULL);
if (ret != size)
return AVERROR_BUG;
}
af->nb_samples += nb_samples;

return nb_samples;
}

int av_audio_fifo_peek(AVAudioFifo *af, void **data, int nb_samples)
{
int i, ret, size;

if (nb_samples < 0)
return AVERROR(EINVAL);
nb_samples = FFMIN(nb_samples, af->nb_samples);
if (!nb_samples)
return 0;

size = nb_samples * af->sample_size;
for (i = 0; i < af->nb_buffers; i++) {
if ((ret = av_fifo_generic_peek(af->buf[i], data[i], size, NULL)) < 0)
return AVERROR_BUG;
}

return nb_samples;
}

int av_audio_fifo_peek_at(AVAudioFifo *af, void **data, int nb_samples, int offset)
{
int i, ret, size;

if (offset < 0 || offset >= af->nb_samples)
return AVERROR(EINVAL);
if (nb_samples < 0)
return AVERROR(EINVAL);
nb_samples = FFMIN(nb_samples, af->nb_samples);
if (!nb_samples)
return 0;
if (offset > af->nb_samples - nb_samples)
return AVERROR(EINVAL);

offset *= af->sample_size;
size = nb_samples * af->sample_size;
for (i = 0; i < af->nb_buffers; i++) {
if ((ret = av_fifo_generic_peek_at(af->buf[i], data[i], offset, size, NULL)) < 0)
return AVERROR_BUG;
}

return nb_samples;
}

int av_audio_fifo_read(AVAudioFifo *af, void **data, int nb_samples)
{
int i, size;

if (nb_samples < 0)
return AVERROR(EINVAL);
nb_samples = FFMIN(nb_samples, af->nb_samples);
if (!nb_samples)
return 0;

size = nb_samples * af->sample_size;
for (i = 0; i < af->nb_buffers; i++) {
if (av_fifo_generic_read(af->buf[i], data[i], size, NULL) < 0)
return AVERROR_BUG;
}
af->nb_samples -= nb_samples;

return nb_samples;
}

int av_audio_fifo_drain(AVAudioFifo *af, int nb_samples)
{
int i, size;

if (nb_samples < 0)
return AVERROR(EINVAL);
nb_samples = FFMIN(nb_samples, af->nb_samples);

if (nb_samples) {
size = nb_samples * af->sample_size;
for (i = 0; i < af->nb_buffers; i++)
av_fifo_drain(af->buf[i], size);
af->nb_samples -= nb_samples;
}
return 0;
}

void av_audio_fifo_reset(AVAudioFifo *af)
{
int i;

for (i = 0; i < af->nb_buffers; i++)
av_fifo_reset(af->buf[i]);

af->nb_samples = 0;
}

int av_audio_fifo_size(AVAudioFifo *af)
{
return af->nb_samples;
}

int av_audio_fifo_space(AVAudioFifo *af)
{
return af->allocated_samples - af->nb_samples;
}
