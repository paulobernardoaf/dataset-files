#include "avformat.h"
#include "internal.h"
typedef struct RCVContext {
int frames;
} RCVContext;
static int vc1test_write_header(AVFormatContext *s)
{
AVCodecParameters *par = s->streams[0]->codecpar;
AVIOContext *pb = s->pb;
if (par->codec_id != AV_CODEC_ID_WMV3) {
av_log(s, AV_LOG_ERROR, "Only WMV3 is accepted!\n");
return -1;
}
avio_wl24(pb, 0); 
avio_w8(pb, 0xC5);
avio_wl32(pb, 4);
avio_write(pb, par->extradata, 4);
avio_wl32(pb, par->height);
avio_wl32(pb, par->width);
avio_wl32(pb, 0xC);
avio_wl24(pb, 0); 
avio_w8(pb, 0x80); 
avio_wl32(pb, 0); 
if (s->streams[0]->avg_frame_rate.den && s->streams[0]->avg_frame_rate.num == 1)
avio_wl32(pb, s->streams[0]->avg_frame_rate.den);
else
avio_wl32(pb, 0xFFFFFFFF); 
avpriv_set_pts_info(s->streams[0], 32, 1, 1000);
return 0;
}
static int vc1test_write_packet(AVFormatContext *s, AVPacket *pkt)
{
RCVContext *ctx = s->priv_data;
AVIOContext *pb = s->pb;
if (!pkt->size)
return 0;
avio_wl32(pb, pkt->size | ((pkt->flags & AV_PKT_FLAG_KEY) ? 0x80000000 : 0));
avio_wl32(pb, pkt->pts);
avio_write(pb, pkt->data, pkt->size);
ctx->frames++;
return 0;
}
static int vc1test_write_trailer(AVFormatContext *s)
{
RCVContext *ctx = s->priv_data;
AVIOContext *pb = s->pb;
if (s->pb->seekable & AVIO_SEEKABLE_NORMAL) {
avio_seek(pb, 0, SEEK_SET);
avio_wl24(pb, ctx->frames);
}
return 0;
}
AVOutputFormat ff_vc1t_muxer = {
.name = "vc1test",
.long_name = NULL_IF_CONFIG_SMALL("VC-1 test bitstream"),
.extensions = "rcv",
.priv_data_size = sizeof(RCVContext),
.audio_codec = AV_CODEC_ID_NONE,
.video_codec = AV_CODEC_ID_WMV3,
.write_header = vc1test_write_header,
.write_packet = vc1test_write_packet,
.write_trailer = vc1test_write_trailer,
};
