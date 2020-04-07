#include "rtpenc.h"
#define RTP_VP9_DESC_REQUIRED_SIZE 1
void ff_rtp_send_vp9(AVFormatContext *ctx, const uint8_t *buf, int size)
{
RTPMuxContext *rtp_ctx = ctx->priv_data;
int len;
rtp_ctx->timestamp = rtp_ctx->cur_timestamp;
rtp_ctx->buf_ptr = rtp_ctx->buf;
*rtp_ctx->buf_ptr++ = 0x08;
while (size > 0) {
len = FFMIN(size, rtp_ctx->max_payload_size - RTP_VP9_DESC_REQUIRED_SIZE);
if (len == size) {
rtp_ctx->buf[0] |= 0x04;
}
memcpy(rtp_ctx->buf_ptr, buf, len);
ff_rtp_send_data(ctx, rtp_ctx->buf, len + RTP_VP9_DESC_REQUIRED_SIZE, size == len);
size -= len;
buf += len;
rtp_ctx->buf[0] &= ~0x08;
}
}
