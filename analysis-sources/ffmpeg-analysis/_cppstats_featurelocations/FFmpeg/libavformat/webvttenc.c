

























#include "avformat.h"
#include "internal.h"

static void webvtt_write_time(AVIOContext *pb, int64_t millisec)
{
int64_t sec, min, hour;
sec = millisec / 1000;
millisec -= 1000 * sec;
min = sec / 60;
sec -= 60 * min;
hour = min / 60;
min -= 60 * hour;

if (hour > 0)
avio_printf(pb, "%02"PRId64":", hour);

avio_printf(pb, "%02"PRId64":%02"PRId64".%03"PRId64"", min, sec, millisec);
}

static int webvtt_write_header(AVFormatContext *ctx)
{
AVStream *s = ctx->streams[0];
AVCodecParameters *par = ctx->streams[0]->codecpar;
AVIOContext *pb = ctx->pb;

if (ctx->nb_streams != 1 || par->codec_id != AV_CODEC_ID_WEBVTT) {
av_log(ctx, AV_LOG_ERROR, "Exactly one WebVTT stream is needed.\n");
return AVERROR(EINVAL);
}

avpriv_set_pts_info(s, 64, 1, 1000);

avio_printf(pb, "WEBVTT\n");

return 0;
}

static int webvtt_write_packet(AVFormatContext *ctx, AVPacket *pkt)
{
AVIOContext *pb = ctx->pb;
int id_size, settings_size;
uint8_t *id, *settings;

avio_printf(pb, "\n");

id = av_packet_get_side_data(pkt, AV_PKT_DATA_WEBVTT_IDENTIFIER,
&id_size);

if (id && id_size > 0)
avio_printf(pb, "%.*s\n", id_size, id);

webvtt_write_time(pb, pkt->pts);
avio_printf(pb, " --> ");
webvtt_write_time(pb, pkt->pts + pkt->duration);

settings = av_packet_get_side_data(pkt, AV_PKT_DATA_WEBVTT_SETTINGS,
&settings_size);

if (settings && settings_size > 0)
avio_printf(pb, " %.*s", settings_size, settings);

avio_printf(pb, "\n");

avio_write(pb, pkt->data, pkt->size);
avio_printf(pb, "\n");

return 0;
}

AVOutputFormat ff_webvtt_muxer = {
.name = "webvtt",
.long_name = NULL_IF_CONFIG_SMALL("WebVTT subtitle"),
.extensions = "vtt",
.mime_type = "text/vtt",
.flags = AVFMT_VARIABLE_FPS | AVFMT_TS_NONSTRICT,
.subtitle_codec = AV_CODEC_ID_WEBVTT,
.write_header = webvtt_write_header,
.write_packet = webvtt_write_packet,
};
