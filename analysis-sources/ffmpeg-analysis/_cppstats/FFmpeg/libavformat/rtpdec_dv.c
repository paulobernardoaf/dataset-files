#include "libavutil/avstring.h"
#include "libavcodec/bytestream.h"
#include "avio_internal.h"
#include "rtpdec_formats.h"
struct PayloadContext {
AVIOContext *buf;
uint32_t timestamp;
int bundled_audio;
};
static av_cold void dv_close_context(PayloadContext *data)
{
ffio_free_dyn_buf(&data->buf);
}
static av_cold int dv_sdp_parse_fmtp_config(AVFormatContext *s,
AVStream *stream,
PayloadContext *dv_data,
const char *attr, const char *value)
{
if (!strcmp(attr, "audio") && !strcmp(value, "bundled"))
dv_data->bundled_audio = 1;
if (!strcmp(attr, "encode")) {
}
return 0;
}
static av_cold int dv_parse_sdp_line(AVFormatContext *ctx, int st_index,
PayloadContext *dv_data, const char *line)
{
AVStream *current_stream;
const char *sdp_line_ptr = line;
if (st_index < 0)
return 0;
current_stream = ctx->streams[st_index];
if (av_strstart(sdp_line_ptr, "fmtp:", &sdp_line_ptr)) {
return ff_parse_fmtp(ctx, current_stream, dv_data, sdp_line_ptr,
dv_sdp_parse_fmtp_config);
}
return 0;
}
static int dv_handle_packet(AVFormatContext *ctx, PayloadContext *rtp_dv_ctx,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
int res = 0;
if (rtp_dv_ctx->buf && rtp_dv_ctx->timestamp != *timestamp) {
ffio_free_dyn_buf(&rtp_dv_ctx->buf);
}
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/DV packet, got %d bytes\n", len);
return AVERROR_INVALIDDATA;
}
if (!rtp_dv_ctx->buf) {
res = avio_open_dyn_buf(&rtp_dv_ctx->buf);
if (res < 0)
return res;
rtp_dv_ctx->timestamp = *timestamp;
}
avio_write(rtp_dv_ctx->buf, buf, len);
if (!(flags & RTP_FLAG_MARKER))
return AVERROR(EAGAIN);
res = ff_rtp_finalize_packet(pkt, &rtp_dv_ctx->buf, st->index);
if (res < 0)
return res;
return 0;
}
const RTPDynamicProtocolHandler ff_dv_dynamic_handler = {
.enc_name = "DV",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_DVVIDEO,
.need_parsing = AVSTREAM_PARSE_FULL,
.parse_sdp_a_line = dv_parse_sdp_line,
.priv_data_size = sizeof(PayloadContext),
.close = dv_close_context,
.parse_packet = dv_handle_packet,
};
