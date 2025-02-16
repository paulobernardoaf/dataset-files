#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "internal.h"
static int ads_probe(const AVProbeData *p)
{
if (memcmp(p->buf, "SShd", 4) ||
memcmp(p->buf+32, "SSbd", 4))
return 0;
return AVPROBE_SCORE_MAX / 3 * 2;
}
static int ads_read_header(AVFormatContext *s)
{
int align, codec, size;
AVStream *st;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avio_skip(s->pb, 8);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
codec = avio_rl32(s->pb);
st->codecpar->sample_rate = avio_rl32(s->pb);
if (st->codecpar->sample_rate <= 0)
return AVERROR_INVALIDDATA;
st->codecpar->channels = avio_rl32(s->pb);
if (st->codecpar->channels <= 0)
return AVERROR_INVALIDDATA;
align = avio_rl32(s->pb);
if (align <= 0 || align > INT_MAX / st->codecpar->channels)
return AVERROR_INVALIDDATA;
if (codec == 1)
st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE_PLANAR;
else
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_PSX;
st->codecpar->block_align = st->codecpar->channels * align;
avio_skip(s->pb, 12);
size = avio_rl32(s->pb);
if (st->codecpar->codec_id == AV_CODEC_ID_ADPCM_PSX)
st->duration = (size - 0x40) / 16 / st->codecpar->channels * 28;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}
static int ads_read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVCodecParameters *par = s->streams[0]->codecpar;
int ret;
ret = av_get_packet(s->pb, pkt, par->block_align);
pkt->stream_index = 0;
return ret;
}
AVInputFormat ff_ads_demuxer = {
.name = "ads",
.long_name = NULL_IF_CONFIG_SMALL("Sony PS2 ADS"),
.read_probe = ads_probe,
.read_header = ads_read_header,
.read_packet = ads_read_packet,
.extensions = "ads,ss2",
};
