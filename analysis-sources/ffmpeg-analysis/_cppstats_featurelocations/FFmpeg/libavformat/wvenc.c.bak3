





















#include "libavutil/attributes.h"

#include "apetag.h"
#include "avformat.h"
#include "wv.h"

typedef struct WvMuxContext {
int64_t samples;
} WvMuxContext;

static av_cold int wv_write_header(AVFormatContext *ctx)
{
if (ctx->nb_streams > 1 ||
ctx->streams[0]->codecpar->codec_id != AV_CODEC_ID_WAVPACK) {
av_log(ctx, AV_LOG_ERROR, "This muxer only supports a single WavPack stream.\n");
return AVERROR(EINVAL);
}

return 0;
}

static int wv_write_packet(AVFormatContext *ctx, AVPacket *pkt)
{
WvMuxContext *s = ctx->priv_data;
WvHeader header;
int ret;

if (pkt->size < WV_HEADER_SIZE ||
(ret = ff_wv_parse_header(&header, pkt->data)) < 0) {
av_log(ctx, AV_LOG_ERROR, "Invalid WavPack packet.\n");
return AVERROR(EINVAL);
}
s->samples += header.samples;

avio_write(ctx->pb, pkt->data, pkt->size);

return 0;
}

static av_cold int wv_write_trailer(AVFormatContext *ctx)
{
WvMuxContext *s = ctx->priv_data;


if ((ctx->pb->seekable & AVIO_SEEKABLE_NORMAL) && s->samples &&
s->samples < UINT32_MAX) {
int64_t pos = avio_tell(ctx->pb);
avio_seek(ctx->pb, 12, SEEK_SET);
avio_wl32(ctx->pb, s->samples);
avio_seek(ctx->pb, pos, SEEK_SET);
}

ff_ape_write_tag(ctx);
return 0;
}

AVOutputFormat ff_wv_muxer = {
.name = "wv",
.long_name = NULL_IF_CONFIG_SMALL("raw WavPack"),
.mime_type = "audio/x-wavpack",
.extensions = "wv",
.priv_data_size = sizeof(WvMuxContext),
.audio_codec = AV_CODEC_ID_WAVPACK,
.video_codec = AV_CODEC_ID_NONE,
.write_header = wv_write_header,
.write_packet = wv_write_packet,
.write_trailer = wv_write_trailer,
.flags = AVFMT_NOTIMESTAMPS,
};
