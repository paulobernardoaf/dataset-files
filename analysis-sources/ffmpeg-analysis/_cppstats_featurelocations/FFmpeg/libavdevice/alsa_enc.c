






































#include <alsa/asoundlib.h>

#include "libavutil/internal.h"
#include "libavutil/time.h"


#include "libavformat/internal.h"
#include "avdevice.h"
#include "alsa.h"

static av_cold int audio_write_header(AVFormatContext *s1)
{
AlsaData *s = s1->priv_data;
AVStream *st = NULL;
unsigned int sample_rate;
enum AVCodecID codec_id;
int res;

if (s1->nb_streams != 1 || s1->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
av_log(s1, AV_LOG_ERROR, "Only a single audio stream is supported.\n");
return AVERROR(EINVAL);
}
st = s1->streams[0];

sample_rate = st->codecpar->sample_rate;
codec_id = st->codecpar->codec_id;
res = ff_alsa_open(s1, SND_PCM_STREAM_PLAYBACK, &sample_rate,
st->codecpar->channels, &codec_id);
if (sample_rate != st->codecpar->sample_rate) {
av_log(s1, AV_LOG_ERROR,
"sample rate %d not available, nearest is %d\n",
st->codecpar->sample_rate, sample_rate);
goto fail;
}
avpriv_set_pts_info(st, 64, 1, sample_rate);

return res;

fail:
snd_pcm_close(s->h);
return AVERROR(EIO);
}

static int audio_write_packet(AVFormatContext *s1, AVPacket *pkt)
{
AlsaData *s = s1->priv_data;
int res;
int size = pkt->size;
uint8_t *buf = pkt->data;

size /= s->frame_size;
if (pkt->dts != AV_NOPTS_VALUE)
s->timestamp = pkt->dts;
s->timestamp += pkt->duration ? pkt->duration : size;

if (s->reorder_func) {
if (size > s->reorder_buf_size)
if (ff_alsa_extend_reorder_buf(s, size))
return AVERROR(ENOMEM);
s->reorder_func(buf, s->reorder_buf, size);
buf = s->reorder_buf;
}
while ((res = snd_pcm_writei(s->h, buf, size)) < 0) {
if (res == -EAGAIN) {

return AVERROR(EAGAIN);
}

if (ff_alsa_xrun_recover(s1, res) < 0) {
av_log(s1, AV_LOG_ERROR, "ALSA write error: %s\n",
snd_strerror(res));

return AVERROR(EIO);
}
}

return 0;
}

static int audio_write_frame(AVFormatContext *s1, int stream_index,
AVFrame **frame, unsigned flags)
{
AlsaData *s = s1->priv_data;
AVPacket pkt;


if ((flags & AV_WRITE_UNCODED_FRAME_QUERY))
return av_sample_fmt_is_planar(s1->streams[stream_index]->codecpar->format) ?
AVERROR(EINVAL) : 0;

pkt.data = (*frame)->data[0];
pkt.size = (*frame)->nb_samples * s->frame_size;
pkt.dts = (*frame)->pkt_dts;
pkt.duration = (*frame)->pkt_duration;
return audio_write_packet(s1, &pkt);
}

static void
audio_get_output_timestamp(AVFormatContext *s1, int stream,
int64_t *dts, int64_t *wall)
{
AlsaData *s = s1->priv_data;
snd_pcm_sframes_t delay = 0;
*wall = av_gettime();
snd_pcm_delay(s->h, &delay);
*dts = s->timestamp - delay;
}

static int audio_get_device_list(AVFormatContext *h, AVDeviceInfoList *device_list)
{
return ff_alsa_get_device_list(device_list, SND_PCM_STREAM_PLAYBACK);
}

static const AVClass alsa_muxer_class = {
.class_name = "ALSA outdev",
.item_name = av_default_item_name,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
};

AVOutputFormat ff_alsa_muxer = {
.name = "alsa",
.long_name = NULL_IF_CONFIG_SMALL("ALSA audio output"),
.priv_data_size = sizeof(AlsaData),
.audio_codec = DEFAULT_CODEC_ID,
.video_codec = AV_CODEC_ID_NONE,
.write_header = audio_write_header,
.write_packet = audio_write_packet,
.write_trailer = ff_alsa_close,
.write_uncoded_frame = audio_write_frame,
.get_device_list = audio_get_device_list,
.get_output_timestamp = audio_get_output_timestamp,
.flags = AVFMT_NOFILE,
.priv_class = &alsa_muxer_class,
};
