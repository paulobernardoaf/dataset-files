#include "avformat.h"
static int daud_write_header(struct AVFormatContext *s)
{
AVCodecParameters *par = s->streams[0]->codecpar;
if (par->channels!=6 || par->sample_rate!=96000)
return -1;
return 0;
}
static int daud_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
if (pkt->size > 65535) {
av_log(s, AV_LOG_ERROR,
"Packet size too large for s302m. (%d > 65535)\n", pkt->size);
return -1;
}
avio_wb16(s->pb, pkt->size);
avio_wb16(s->pb, 0x8010); 
avio_write(s->pb, pkt->data, pkt->size);
return 0;
}
AVOutputFormat ff_daud_muxer = {
.name = "daud",
.long_name = NULL_IF_CONFIG_SMALL("D-Cinema audio"),
.extensions = "302",
.audio_codec = AV_CODEC_ID_PCM_S24DAUD,
.video_codec = AV_CODEC_ID_NONE,
.write_header = daud_write_header,
.write_packet = daud_write_packet,
.flags = AVFMT_NOTIMESTAMPS,
};
