#include "libavutil/intreadwrite.h"
#include "libavcodec/internal.h"
#include "avformat.h"
#include "internal.h"
static int probe(const AVProbeData *p)
{
if (p->buf_size < 2096)
return 0;
if ( AV_RL32(p->buf ) != 1
|| AV_RL32(p->buf + 8) > 100000
|| AV_RL32(p->buf + 12) > 8
|| AV_RL32(p->buf + 16) != 2096
||!AV_RL32(p->buf + 21)
|| AV_RL16(p->buf + 25) != 2096
|| AV_RL32(p->buf + 48) % AV_RL32(p->buf + 21)
)
return 0;
return AVPROBE_SCORE_EXTENSION;
}
static int read_header(AVFormatContext *s)
{
AVStream *st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_MS;
avio_rl32(s->pb);
avio_rl32(s->pb);
st->codecpar->sample_rate = avio_rl32(s->pb);
st->codecpar->channels = avio_rl32(s->pb);
if (st->codecpar->channels > FF_SANE_NB_CHANNELS)
return AVERROR(ENOSYS);
s->internal->data_offset = avio_rl32(s->pb);
avio_r8(s->pb);
st->codecpar->block_align = avio_rl32(s->pb);
if (st->codecpar->block_align > INT_MAX / FF_SANE_NB_CHANNELS)
return AVERROR_INVALIDDATA;
st->codecpar->block_align *= st->codecpar->channels;
avio_seek(s->pb, s->internal->data_offset, SEEK_SET);
return 0;
}
static int read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVStream *st = s->streams[0];
return av_get_packet(s->pb, pkt, st->codecpar->block_align);
}
AVInputFormat ff_boa_demuxer = {
.name = "boa",
.long_name = NULL_IF_CONFIG_SMALL("Black Ops Audio"),
.read_probe = probe,
.read_header = read_header,
.read_packet = read_packet,
.flags = AVFMT_GENERIC_INDEX,
};
