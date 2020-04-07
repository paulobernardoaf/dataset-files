














































#include <alsa/asoundlib.h>

#include "libavutil/internal.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"

#include "libavformat/internal.h"

#include "avdevice.h"
#include "alsa.h"

static av_cold int audio_read_header(AVFormatContext *s1)
{
AlsaData *s = s1->priv_data;
AVStream *st;
int ret;
enum AVCodecID codec_id;

st = avformat_new_stream(s1, NULL);
if (!st) {
av_log(s1, AV_LOG_ERROR, "Cannot add stream\n");

return AVERROR(ENOMEM);
}
codec_id = s1->audio_codec_id;

ret = ff_alsa_open(s1, SND_PCM_STREAM_CAPTURE, &s->sample_rate, s->channels,
&codec_id);
if (ret < 0) {
return AVERROR(EIO);
}


st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = codec_id;
st->codecpar->sample_rate = s->sample_rate;
st->codecpar->channels = s->channels;
st->codecpar->frame_size = s->frame_size;
avpriv_set_pts_info(st, 64, 1, 1000000); 

s->timefilter = ff_timefilter_new(1000000.0 / s->sample_rate,
s->period_size, 1.5E-6);
if (!s->timefilter)
goto fail;

return 0;

fail:
snd_pcm_close(s->h);
return AVERROR(EIO);
}

static int audio_read_packet(AVFormatContext *s1, AVPacket *pkt)
{
AlsaData *s = s1->priv_data;
int res;
int64_t dts;
snd_pcm_sframes_t delay = 0;

if (av_new_packet(pkt, s->period_size * s->frame_size) < 0) {
return AVERROR(EIO);
}

while ((res = snd_pcm_readi(s->h, pkt->data, s->period_size)) < 0) {
if (res == -EAGAIN) {
av_packet_unref(pkt);

return AVERROR(EAGAIN);
}
if (ff_alsa_xrun_recover(s1, res) < 0) {
av_log(s1, AV_LOG_ERROR, "ALSA read error: %s\n",
snd_strerror(res));
av_packet_unref(pkt);

return AVERROR(EIO);
}
ff_timefilter_reset(s->timefilter);
}

dts = av_gettime();
snd_pcm_delay(s->h, &delay);
dts -= av_rescale(delay + res, 1000000, s->sample_rate);
pkt->pts = ff_timefilter_update(s->timefilter, dts, s->last_period);
s->last_period = res;

pkt->size = res * s->frame_size;

return 0;
}

static int audio_get_device_list(AVFormatContext *h, AVDeviceInfoList *device_list)
{
return ff_alsa_get_device_list(device_list, SND_PCM_STREAM_CAPTURE);
}

static const AVOption options[] = {
{ "sample_rate", "", offsetof(AlsaData, sample_rate), AV_OPT_TYPE_INT, {.i64 = 48000}, 1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
{ "channels", "", offsetof(AlsaData, channels), AV_OPT_TYPE_INT, {.i64 = 2}, 1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
{ NULL },
};

static const AVClass alsa_demuxer_class = {
.class_name = "ALSA indev",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT,
};

AVInputFormat ff_alsa_demuxer = {
.name = "alsa",
.long_name = NULL_IF_CONFIG_SMALL("ALSA audio input"),
.priv_data_size = sizeof(AlsaData),
.read_header = audio_read_header,
.read_packet = audio_read_packet,
.read_close = ff_alsa_close,
.get_device_list = audio_get_device_list,
.flags = AVFMT_NOFILE,
.priv_class = &alsa_demuxer_class,
};
