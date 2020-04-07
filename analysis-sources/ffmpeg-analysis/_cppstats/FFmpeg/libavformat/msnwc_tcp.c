#include "libavcodec/bytestream.h"
#include "avformat.h"
#include "internal.h"
#define HEADER_SIZE 24
static int msnwc_tcp_probe(const AVProbeData *p)
{
int i;
for (i = 0; i + HEADER_SIZE <= p->buf_size; i++) {
uint16_t width, height;
uint32_t fourcc;
const uint8_t *bytestream = p->buf + i;
if (bytestream_get_le16(&bytestream) != HEADER_SIZE)
continue;
width = bytestream_get_le16(&bytestream);
height = bytestream_get_le16(&bytestream);
if (!(width == 320 &&
height == 240) && !(width == 160 && height == 120))
continue;
bytestream += 2; 
bytestream += 4; 
fourcc = bytestream_get_le32(&bytestream);
if (fourcc != MKTAG('M', 'L', '2', '0'))
continue;
if (i) {
if (i < 14) 
return AVPROBE_SCORE_MAX / 2;
else 
return AVPROBE_SCORE_MAX / 3;
} else {
return AVPROBE_SCORE_MAX;
}
}
return 0;
}
static int msnwc_tcp_read_header(AVFormatContext *ctx)
{
AVIOContext *pb = ctx->pb;
AVCodecParameters *par;
AVStream *st;
st = avformat_new_stream(ctx, NULL);
if (!st)
return AVERROR(ENOMEM);
par = st->codecpar;
par->codec_type = AVMEDIA_TYPE_VIDEO;
par->codec_id = AV_CODEC_ID_MIMIC;
par->codec_tag = MKTAG('M', 'L', '2', '0');
avpriv_set_pts_info(st, 32, 1, 1000);
while(avio_r8(pb) != HEADER_SIZE && !avio_feof(pb)) ;
if(avio_feof(pb)) {
av_log(ctx, AV_LOG_ERROR, "Could not find valid start.\n");
return AVERROR_INVALIDDATA;
}
return 0;
}
static int msnwc_tcp_read_packet(AVFormatContext *ctx, AVPacket *pkt)
{
AVIOContext *pb = ctx->pb;
uint16_t keyframe;
uint32_t size, timestamp;
int ret;
avio_skip(pb, 1); 
avio_skip(pb, 2);
avio_skip(pb, 2);
keyframe = avio_rl16(pb);
size = avio_rl32(pb);
avio_skip(pb, 4);
avio_skip(pb, 4);
timestamp = avio_rl32(pb);
if (!size)
return AVERROR_INVALIDDATA;
if ((ret = av_get_packet(pb, pkt, size)) < 0)
return ret;
avio_skip(pb, 1); 
pkt->pts = timestamp;
pkt->dts = timestamp;
pkt->stream_index = 0;
if (keyframe & 1)
pkt->flags |= AV_PKT_FLAG_KEY;
return HEADER_SIZE + size;
}
AVInputFormat ff_msnwc_tcp_demuxer = {
.name = "msnwctcp",
.long_name = NULL_IF_CONFIG_SMALL("MSN TCP Webcam stream"),
.read_probe = msnwc_tcp_probe,
.read_header = msnwc_tcp_read_header,
.read_packet = msnwc_tcp_read_packet,
};
