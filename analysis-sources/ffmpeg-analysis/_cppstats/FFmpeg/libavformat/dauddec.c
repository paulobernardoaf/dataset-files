#include "libavutil/channel_layout.h"
#include "avformat.h"
static int daud_header(AVFormatContext *s) {
AVStream *st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_PCM_S24DAUD;
st->codecpar->codec_tag = MKTAG('d', 'a', 'u', 'd');
st->codecpar->channels = 6;
st->codecpar->channel_layout = AV_CH_LAYOUT_5POINT1;
st->codecpar->sample_rate = 96000;
st->codecpar->bit_rate = 3 * 6 * 96000 * 8;
st->codecpar->block_align = 3 * 6;
st->codecpar->bits_per_coded_sample = 24;
return 0;
}
static int daud_packet(AVFormatContext *s, AVPacket *pkt) {
AVIOContext *pb = s->pb;
int ret, size;
if (avio_feof(pb))
return AVERROR(EIO);
size = avio_rb16(pb);
avio_rb16(pb); 
ret = av_get_packet(pb, pkt, size);
pkt->stream_index = 0;
return ret;
}
AVInputFormat ff_daud_demuxer = {
.name = "daud",
.long_name = NULL_IF_CONFIG_SMALL("D-Cinema audio"),
.read_header = daud_header,
.read_packet = daud_packet,
.extensions = "302,daud",
};
