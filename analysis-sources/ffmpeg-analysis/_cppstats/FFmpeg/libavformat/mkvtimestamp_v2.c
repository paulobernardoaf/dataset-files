#include "avformat.h"
#include "internal.h"
static int write_header(AVFormatContext *s)
{
static const char *header = "#timecode format v2\n";
avio_write(s->pb, header, strlen(header));
avpriv_set_pts_info(s->streams[0], 64, 1, 1000);
return 0;
}
static int write_packet(AVFormatContext *s, AVPacket *pkt)
{
char buf[256];
if (pkt->stream_index)
av_log(s, AV_LOG_WARNING, "More than one stream unsupported\n");
snprintf(buf, sizeof(buf), "%" PRId64 "\n", pkt->dts);
avio_write(s->pb, buf, strlen(buf));
return 0;
}
AVOutputFormat ff_mkvtimestamp_v2_muxer = {
.name = "mkvtimestamp_v2",
.long_name = NULL_IF_CONFIG_SMALL("extract pts as timecode v2 format, as defined by mkvtoolnix"),
.audio_codec = AV_CODEC_ID_NONE,
.video_codec = AV_CODEC_ID_RAWVIDEO,
.write_header = write_header,
.write_packet = write_packet,
};
