




















#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

static int mtaf_probe(const AVProbeData *p)
{
if (p->buf_size < 0x44)
return 0;

if (AV_RL32(p->buf) != MKTAG('M','T','A','F') ||
AV_RL32(p->buf + 0x40) != MKTAG('H','E','A','D'))
return 0;

return AVPROBE_SCORE_MAX;
}

static int mtaf_read_header(AVFormatContext *s)
{
int stream_count;
AVStream *st;

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);

avio_skip(s->pb, 0x5c);
st->duration = avio_rl32(s->pb);
avio_skip(s->pb, 1);
stream_count = avio_r8(s->pb);
if (!stream_count)
return AVERROR_INVALIDDATA;

st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_MTAF;
st->codecpar->channels = 2 * stream_count;
st->codecpar->sample_rate = 48000;
st->codecpar->block_align = 0x110 * st->codecpar->channels / 2;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);

avio_seek(s->pb, 0x800, SEEK_SET);

return 0;
}

static int mtaf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVCodecParameters *par = s->streams[0]->codecpar;

return av_get_packet(s->pb, pkt, par->block_align);
}

AVInputFormat ff_mtaf_demuxer = {
.name = "mtaf",
.long_name = NULL_IF_CONFIG_SMALL("Konami PS2 MTAF"),
.read_probe = mtaf_probe,
.read_header = mtaf_read_header,
.read_packet = mtaf_read_packet,
.extensions = "mtaf",
};
