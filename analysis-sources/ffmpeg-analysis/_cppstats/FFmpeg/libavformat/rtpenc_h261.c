#include "avformat.h"
#include "rtpenc.h"
#define RTP_H261_HEADER_SIZE 4
static const uint8_t *find_resync_marker_reverse(const uint8_t *av_restrict start,
const uint8_t *av_restrict end)
{
const uint8_t *p = end - 1;
start += 1; 
for (; p > start; p--) {
if (p[0] == 0 && p[1] == 1)
return p;
}
return end;
}
void ff_rtp_send_h261(AVFormatContext *ctx, const uint8_t *frame_buf, int frame_size)
{
int cur_frame_size;
int last_packet_of_frame;
RTPMuxContext *rtp_ctx = ctx->priv_data;
rtp_ctx->timestamp = rtp_ctx->cur_timestamp;
while (frame_size > 0) {
rtp_ctx->buf[0] = 1; 
rtp_ctx->buf[1] = 0; 
rtp_ctx->buf[2] = 0; 
rtp_ctx->buf[3] = 0; 
if (frame_size < 2 || frame_buf[0] != 0 || frame_buf[1] != 1) {
av_log(ctx, AV_LOG_WARNING,
"RTP/H.261 packet not cut at a GOB boundary, not signaled correctly\n");
}
cur_frame_size = FFMIN(rtp_ctx->max_payload_size - RTP_H261_HEADER_SIZE, frame_size);
if (cur_frame_size < frame_size) {
const uint8_t *packet_end = find_resync_marker_reverse(frame_buf,
frame_buf + cur_frame_size);
cur_frame_size = packet_end - frame_buf;
}
last_packet_of_frame = cur_frame_size == frame_size;
memcpy(&rtp_ctx->buf[RTP_H261_HEADER_SIZE], frame_buf, cur_frame_size);
ff_rtp_send_data(ctx, rtp_ctx->buf, RTP_H261_HEADER_SIZE + cur_frame_size, last_packet_of_frame);
frame_buf += cur_frame_size;
frame_size -= cur_frame_size;
}
}
