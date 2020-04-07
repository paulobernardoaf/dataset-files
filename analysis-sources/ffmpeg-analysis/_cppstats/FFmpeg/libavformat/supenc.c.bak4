




















#include "avformat.h"
#include "internal.h"
#include "libavutil/intreadwrite.h"

#define SUP_PGS_MAGIC 0x5047 

static int sup_write_packet(AVFormatContext *s, AVPacket *pkt)
{
uint8_t *data = pkt->data;
size_t size = pkt->size;
uint32_t pts = 0, dts = 0;

if (pkt->pts != AV_NOPTS_VALUE) {
pts = pkt->pts;
}
if (pkt->dts != AV_NOPTS_VALUE) {
dts = pkt->dts;
}





while (size > 2) {
size_t len = AV_RB16(data + 1) + 3;

if (len > size) {
av_log(s, AV_LOG_ERROR, "Not enough data, skipping %"SIZE_SPECIFIER" bytes\n",
size);
return AVERROR_INVALIDDATA;
}


avio_wb16(s->pb, SUP_PGS_MAGIC);
avio_wb32(s->pb, pts);
avio_wb32(s->pb, dts);

avio_write(s->pb, data, len);

data += len;
size -= len;
}

if (size > 0) {
av_log(s, AV_LOG_ERROR, "Skipping %"SIZE_SPECIFIER" bytes after last segment in frame\n",
size);
return AVERROR_INVALIDDATA;
}

return 0;
}

static int sup_write_header(AVFormatContext *s)
{
if (s->nb_streams != 1) {
av_log(s, AV_LOG_ERROR, "%s files have exactly one stream\n",
s->oformat->name);
return AVERROR(EINVAL);
}

avpriv_set_pts_info(s->streams[0], 32, 1, 90000);

return 0;
}

AVOutputFormat ff_sup_muxer = {
.name = "sup",
.long_name = NULL_IF_CONFIG_SMALL("raw HDMV Presentation Graphic Stream subtitles"),
.extensions = "sup",
.mime_type = "application/x-pgs",
.subtitle_codec = AV_CODEC_ID_HDMV_PGS_SUBTITLE,
.write_header = sup_write_header,
.write_packet = sup_write_packet,
.flags = AVFMT_VARIABLE_FPS | AVFMT_TS_NONSTRICT,
};
