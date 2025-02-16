#include "libavutil/attributes.h"
#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "internal.h"
static const uint8_t frame_size[4] = { 24, 20, 4, 1 };
static av_cold int g723_1_init(AVFormatContext *s)
{
AVStream *st;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_G723_1;
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
st->codecpar->channels = 1;
st->codecpar->sample_rate = 8000;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
st->start_time = 0;
return 0;
}
static int g723_1_read_packet(AVFormatContext *s, AVPacket *pkt)
{
int size, byte, ret;
pkt->pos = avio_tell(s->pb);
byte = avio_r8(s->pb);
size = frame_size[byte & 3];
ret = av_new_packet(pkt, size);
if (ret < 0)
return ret;
pkt->data[0] = byte;
pkt->duration = 240;
pkt->stream_index = 0;
ret = avio_read(s->pb, pkt->data + 1, size - 1);
if (ret < size - 1) {
return ret < 0 ? ret : AVERROR_EOF;
}
return pkt->size;
}
AVInputFormat ff_g723_1_demuxer = {
.name = "g723_1",
.long_name = NULL_IF_CONFIG_SMALL("G.723.1"),
.read_header = g723_1_init,
.read_packet = g723_1_read_packet,
.extensions = "tco,rco,g723_1",
.flags = AVFMT_GENERIC_INDEX
};
