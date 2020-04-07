#include "libavutil/opt.h"
#include "avformat.h"
#define BOUNDARY_TAG "ffmpeg"
typedef struct MPJPEGContext {
AVClass *class;
char *boundary_tag;
} MPJPEGContext;
static int mpjpeg_write_header(AVFormatContext *s)
{
MPJPEGContext *mpj = s->priv_data;
avio_printf(s->pb, "--%s\r\n", mpj->boundary_tag);
return 0;
}
static int mpjpeg_write_packet(AVFormatContext *s, AVPacket *pkt)
{
MPJPEGContext *mpj = s->priv_data;
avio_printf(s->pb, "Content-type: image/jpeg\r\n");
avio_printf(s->pb, "Content-length: %d\r\n\r\n",
pkt->size);
avio_write(s->pb, pkt->data, pkt->size);
avio_printf(s->pb, "\r\n--%s\r\n", mpj->boundary_tag);
return 0;
}
static const AVOption options[] = {
{ "boundary_tag", "Boundary tag", offsetof(MPJPEGContext, boundary_tag), AV_OPT_TYPE_STRING, {.str = BOUNDARY_TAG}, .flags = AV_OPT_FLAG_ENCODING_PARAM },
{ NULL },
};
static const AVClass mpjpeg_muxer_class = {
.class_name = "mpjpeg_muxer",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
AVOutputFormat ff_mpjpeg_muxer = {
.name = "mpjpeg",
.long_name = NULL_IF_CONFIG_SMALL("MIME multipart JPEG"),
.mime_type = "multipart/x-mixed-replace;boundary=" BOUNDARY_TAG,
.extensions = "mjpg",
.priv_data_size = sizeof(MPJPEGContext),
.audio_codec = AV_CODEC_ID_NONE,
.video_codec = AV_CODEC_ID_MJPEG,
.write_header = mpjpeg_write_header,
.write_packet = mpjpeg_write_packet,
.flags = AVFMT_NOTIMESTAMPS,
.priv_class = &mpjpeg_muxer_class,
};
