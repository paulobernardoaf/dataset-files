





















#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

#define NC_VIDEO_FLAG 0x1A5

static int nc_probe(const AVProbeData *probe_packet)
{
int size;

if (AV_RB32(probe_packet->buf) != NC_VIDEO_FLAG)
return 0;

size = AV_RL16(probe_packet->buf + 5);

if (size + 20 > probe_packet->buf_size)
return AVPROBE_SCORE_MAX/4;

if (AV_RB32(probe_packet->buf+16+size) == NC_VIDEO_FLAG)
return AVPROBE_SCORE_MAX;

return 0;
}

static int nc_read_header(AVFormatContext *s)
{
AVStream *st = avformat_new_stream(s, NULL);

if (!st)
return AVERROR(ENOMEM);

st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_MPEG4;
st->need_parsing = AVSTREAM_PARSE_FULL;

avpriv_set_pts_info(st, 64, 1, 100);

return 0;
}

static int nc_read_packet(AVFormatContext *s, AVPacket *pkt)
{
int size;
int ret;

uint32_t state=-1;
while (state != NC_VIDEO_FLAG) {
if (avio_feof(s->pb))
return AVERROR(EIO);
state = (state<<8) + avio_r8(s->pb);
}

avio_r8(s->pb);
size = avio_rl16(s->pb);
avio_skip(s->pb, 9);

if (size == 0) {
av_log(s, AV_LOG_DEBUG, "Next packet size is zero\n");
return AVERROR(EAGAIN);
}

ret = av_get_packet(s->pb, pkt, size);
if (ret != size) {
return AVERROR(EIO);
}

pkt->stream_index = 0;
return size;
}

AVInputFormat ff_nc_demuxer = {
.name = "nc",
.long_name = NULL_IF_CONFIG_SMALL("NC camera feed"),
.read_probe = nc_probe,
.read_header = nc_read_header,
.read_packet = nc_read_packet,
.extensions = "v",
};
