




















#include "libavutil/intreadwrite.h"
#include "libavcodec/internal.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"

static int epaf_probe(const AVProbeData *p)
{
if (((AV_RL32(p->buf) == MKTAG('f','a','p',' ') &&
AV_RL32(p->buf + 8) == 1) ||
(AV_RL32(p->buf) == MKTAG(' ','p','a','f') &&
AV_RN32(p->buf + 8) == 0)) &&
!AV_RN32(p->buf + 4) && AV_RN32(p->buf + 12) &&
AV_RN32(p->buf + 20))
return AVPROBE_SCORE_MAX / 4 * 3;
return 0;
}

static int epaf_read_header(AVFormatContext *s)
{
int le, sample_rate, codec, channels;
AVStream *st;

avio_skip(s->pb, 4);
if (avio_rl32(s->pb))
return AVERROR_INVALIDDATA;

le = avio_rl32(s->pb);
if (le && le != 1)
return AVERROR_INVALIDDATA;

if (le) {
sample_rate = avio_rl32(s->pb);
codec = avio_rl32(s->pb);
channels = avio_rl32(s->pb);
} else {
sample_rate = avio_rb32(s->pb);
codec = avio_rb32(s->pb);
channels = avio_rb32(s->pb);
}

if (channels <= 0 || channels > FF_SANE_NB_CHANNELS || sample_rate <= 0)
return AVERROR_INVALIDDATA;

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->channels = channels;
st->codecpar->sample_rate = sample_rate;
switch (codec) {
case 0:
st->codecpar->codec_id = le ? AV_CODEC_ID_PCM_S16LE : AV_CODEC_ID_PCM_S16BE;
break;
case 2:
st->codecpar->codec_id = AV_CODEC_ID_PCM_S8;
break;
case 1:
avpriv_request_sample(s, "24-bit Paris PCM format");
default:
return AVERROR_INVALIDDATA;
}

st->codecpar->bits_per_coded_sample = av_get_bits_per_sample(st->codecpar->codec_id);
st->codecpar->block_align = st->codecpar->bits_per_coded_sample * st->codecpar->channels / 8;

avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);

if (avio_skip(s->pb, 2024) < 0)
return AVERROR_INVALIDDATA;
return 0;
}

AVInputFormat ff_epaf_demuxer = {
.name = "epaf",
.long_name = NULL_IF_CONFIG_SMALL("Ensoniq Paris Audio File"),
.read_probe = epaf_probe,
.read_header = epaf_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
.extensions = "paf,fap",
.flags = AVFMT_GENERIC_INDEX,
};
