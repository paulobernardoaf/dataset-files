#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "pcm.h"
static int sdx_probe(const AVProbeData *p)
{
if (AV_RB32(p->buf) == AV_RB32("SDX:"))
return AVPROBE_SCORE_EXTENSION;
return 0;
}
static int sdx_read_header(AVFormatContext *s)
{
AVStream *st;
int depth, length;
avio_skip(s->pb, 4);
while (!avio_feof(s->pb)) {
if (avio_r8(s->pb) == 0x1a)
break;
}
if (avio_r8(s->pb) != 1)
return AVERROR_INVALIDDATA;
length = avio_r8(s->pb);
avio_skip(s->pb, length);
avio_skip(s->pb, 4);
depth = avio_r8(s->pb);
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->channels = 1;
st->codecpar->sample_rate = avio_rl32(s->pb);
switch (depth) {
case 8:
st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
break;
case 16:
st->codecpar->codec_id = AV_CODEC_ID_PCM_U16LE;
break;
case 24:
st->codecpar->codec_id = AV_CODEC_ID_PCM_U24LE;
break;
case 32:
st->codecpar->codec_id = AV_CODEC_ID_PCM_U32LE;
break;
default:
return AVERROR_INVALIDDATA;
}
avio_skip(s->pb, 16);
st->codecpar->block_align = depth / 8;
return 0;
}
AVInputFormat ff_sdx_demuxer = {
.name = "sdx",
.long_name = NULL_IF_CONFIG_SMALL("Sample Dump eXchange"),
.read_probe = sdx_probe,
.read_header = sdx_read_header,
.read_packet = ff_pcm_read_packet,
.read_seek = ff_pcm_read_seek,
.extensions = "sdx",
.flags = AVFMT_GENERIC_INDEX,
};
