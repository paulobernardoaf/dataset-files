




















#include "config.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "libavutil/internal.h"

#include "libavcodec/avcodec.h"

#include "avdevice.h"
#include "libavformat/internal.h"

#include "oss.h"

static int audio_write_header(AVFormatContext *s1)
{
OSSAudioData *s = s1->priv_data;
AVStream *st;
int ret;

st = s1->streams[0];
s->sample_rate = st->codecpar->sample_rate;
s->channels = st->codecpar->channels;
ret = ff_oss_audio_open(s1, 1, s1->url);
if (ret < 0) {
return AVERROR(EIO);
} else {
return 0;
}
}

static int audio_write_packet(AVFormatContext *s1, AVPacket *pkt)
{
OSSAudioData *s = s1->priv_data;
int len, ret;
int size= pkt->size;
uint8_t *buf= pkt->data;

while (size > 0) {
len = FFMIN(OSS_AUDIO_BLOCK_SIZE - s->buffer_ptr, size);
memcpy(s->buffer + s->buffer_ptr, buf, len);
s->buffer_ptr += len;
if (s->buffer_ptr >= OSS_AUDIO_BLOCK_SIZE) {
for(;;) {
ret = write(s->fd, s->buffer, OSS_AUDIO_BLOCK_SIZE);
if (ret > 0)
break;
if (ret < 0 && (errno != EAGAIN && errno != EINTR))
return AVERROR(EIO);
}
s->buffer_ptr = 0;
}
buf += len;
size -= len;
}
return 0;
}

static int audio_write_trailer(AVFormatContext *s1)
{
OSSAudioData *s = s1->priv_data;

ff_oss_audio_close(s);
return 0;
}

static const AVClass oss_muxer_class = {
.class_name = "OSS outdev",
.item_name = av_default_item_name,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT,
};

AVOutputFormat ff_oss_muxer = {
.name = "oss",
.long_name = NULL_IF_CONFIG_SMALL("OSS (Open Sound System) playback"),
.priv_data_size = sizeof(OSSAudioData),



.audio_codec = AV_NE(AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_S16LE),
.video_codec = AV_CODEC_ID_NONE,
.write_header = audio_write_header,
.write_packet = audio_write_packet,
.write_trailer = audio_write_trailer,
.flags = AVFMT_NOFILE,
.priv_class = &oss_muxer_class,
};
