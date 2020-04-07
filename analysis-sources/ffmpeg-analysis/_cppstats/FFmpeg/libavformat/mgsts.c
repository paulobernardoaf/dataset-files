#include "libavutil/intreadwrite.h"
#include "libavutil/intfloat.h"
#include "avformat.h"
#include "riff.h"
static int read_probe(const AVProbeData *p)
{
if (AV_RB32(p->buf ) != 0x000E ||
AV_RB32(p->buf + 4) != 0x0050 ||
AV_RB32(p->buf + 12) != 0x0034)
return 0;
return AVPROBE_SCORE_MAX;
}
static int read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
AVStream *st;
AVRational fps;
uint32_t chunk_size;
avio_skip(pb, 4);
chunk_size = avio_rb32(pb);
if (chunk_size != 80)
return AVERROR(EIO);
avio_skip(pb, 20);
st = avformat_new_stream(s, 0);
if (!st)
return AVERROR(ENOMEM);
st->need_parsing = AVSTREAM_PARSE_HEADERS;
st->start_time = 0;
st->nb_frames =
st->duration = avio_rb32(pb);
fps = av_d2q(av_int2float(avio_rb32(pb)), INT_MAX);
st->codecpar->width = avio_rb32(pb);
st->codecpar->height = avio_rb32(pb);
avio_skip(pb, 12);
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_tag = avio_rb32(pb);
st->codecpar->codec_id = ff_codec_get_id(ff_codec_bmp_tags,
st->codecpar->codec_tag);
avpriv_set_pts_info(st, 64, fps.den, fps.num);
avio_skip(pb, 20);
return 0;
}
static int read_packet(AVFormatContext *s, AVPacket *pkt)
{
AVIOContext *pb = s->pb;
uint32_t chunk_size, payload_size;
int ret;
if (avio_feof(pb))
return AVERROR_EOF;
avio_skip(pb, 4);
chunk_size = avio_rb32(pb);
avio_skip(pb, 4);
payload_size = avio_rb32(pb);
if (chunk_size < payload_size + 16)
return AVERROR(EIO);
ret = av_get_packet(pb, pkt, payload_size);
if (ret < 0)
return ret;
pkt->pos -= 16;
pkt->duration = 1;
avio_skip(pb, chunk_size - (ret + 16));
return ret;
}
AVInputFormat ff_mgsts_demuxer = {
.name = "mgsts",
.long_name = NULL_IF_CONFIG_SMALL("Metal Gear Solid: The Twin Snakes"),
.read_probe = read_probe,
.read_header = read_header,
.read_packet = read_packet,
.flags = AVFMT_GENERIC_INDEX,
};
