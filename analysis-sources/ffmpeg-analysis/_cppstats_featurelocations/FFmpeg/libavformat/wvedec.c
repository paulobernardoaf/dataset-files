



















#include "avformat.h"
#include "internal.h"
#include "pcm.h"

static int wve_probe(const AVProbeData *p)
{
if (memcmp(p->buf, "ALawSoundFile**\0\017\020", 18) ||
memcmp(p->buf + 22, "\0\0\0\1\0\0\0\0\0\0", 10))
return 0;
return AVPROBE_SCORE_MAX;
}

static int wve_read_header(AVFormatContext *s)
{
AVStream *st;

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

avio_skip(s->pb, 18);
st->duration = avio_rb32(s->pb);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_PCM_ALAW;
st->codecpar->sample_rate = 8000;
st->codecpar->channels = 1;
st->codecpar->bits_per_coded_sample = av_get_bits_per_sample(st->codecpar->codec_id);
st->codecpar->block_align = st->codecpar->bits_per_coded_sample * st->codecpar->channels / 8;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
avio_skip(s->pb, 10);

return 0;
}

AVInputFormat ff_wve_demuxer = {
.name = "wve",
.long_name = NULL_IF_CONFIG_SMALL("Psion 3 audio"),
.read_probe = wve_probe,
.read_header = wve_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
};
