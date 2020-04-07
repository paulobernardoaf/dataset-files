#include "avformat.h"
#include "rawdec.h"
#include "libavutil/intreadwrite.h"
static int ingenient_probe(const AVProbeData *p)
{
if ( AV_RN32(p->buf) != AV_RN32("MJPG")
|| p->buf_size < 50
|| AV_RB16(p->buf + 48) != 0xffd8)
return 0;
return AVPROBE_SCORE_MAX * 3 / 4;
}
static int ingenient_read_packet(AVFormatContext *s, AVPacket *pkt)
{
int ret, size, w, h, unk1, unk2;
if (avio_rl32(s->pb) != MKTAG('M', 'J', 'P', 'G'))
return AVERROR(EIO); 
size = avio_rl32(s->pb);
w = avio_rl16(s->pb);
h = avio_rl16(s->pb);
avio_skip(s->pb, 8); 
avio_skip(s->pb, 2);
unk1 = avio_rl16(s->pb);
unk2 = avio_rl16(s->pb);
avio_skip(s->pb, 22); 
av_log(s, AV_LOG_DEBUG, "Ingenient packet: size=%d, width=%d, height=%d, unk1=%d unk2=%d\n",
size, w, h, unk1, unk2);
ret = av_get_packet(s->pb, pkt, size);
if (ret < 0)
return ret;
pkt->stream_index = 0;
return ret;
}
FF_RAWVIDEO_DEMUXER_CLASS(ingenient)
AVInputFormat ff_ingenient_demuxer = {
.name = "ingenient",
.long_name = NULL_IF_CONFIG_SMALL("raw Ingenient MJPEG"),
.priv_data_size = sizeof(FFRawVideoDemuxerContext),
.read_probe = ingenient_probe,
.read_header = ff_raw_video_read_header,
.read_packet = ingenient_read_packet,
.flags = AVFMT_GENERIC_INDEX,
.extensions = "cgi", 
.raw_codec_id = AV_CODEC_ID_MJPEG,
.priv_class = &ingenient_demuxer_class,
};
