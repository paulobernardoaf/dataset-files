




















#include "avformat.h"

static int null_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
return 0;
}

AVOutputFormat ff_null_muxer = {
.name = "null",
.long_name = NULL_IF_CONFIG_SMALL("raw null video"),
.audio_codec = AV_NE(AV_CODEC_ID_PCM_S16BE, AV_CODEC_ID_PCM_S16LE),
.video_codec = AV_CODEC_ID_WRAPPED_AVFRAME,
.write_packet = null_write_packet,
.flags = AVFMT_VARIABLE_FPS | AVFMT_NOFILE | AVFMT_NOTIMESTAMPS,
};
