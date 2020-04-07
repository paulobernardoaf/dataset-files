




















#include "libavutil/intreadwrite.h"

#include "avio_internal.h"
#include "rtpdec_formats.h"

#define RTP_VP9_DESC_REQUIRED_SIZE 1

struct PayloadContext {
AVIOContext *buf;
uint32_t timestamp;
};

static av_cold int vp9_init(AVFormatContext *ctx, int st_index,
PayloadContext *data)
{
av_log(ctx, AV_LOG_WARNING,
"RTP/VP9 support is still experimental\n");

return 0;
}

static int vp9_handle_packet(AVFormatContext *ctx, PayloadContext *rtp_vp9_ctx,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
int has_pic_id, has_layer_idc, has_ref_idc, has_ss_data;
av_unused int pic_id = 0, non_key_frame = 0, inter_picture_layer_frame;
av_unused int layer_temporal = -1, layer_spatial = -1, layer_quality = -1;
int ref_fields = 0, has_ref_field_ext_pic_id = 0;
int first_fragment, last_fragment;
int rtp_m;
int res = 0;


if (rtp_vp9_ctx->buf && rtp_vp9_ctx->timestamp != *timestamp)
ffio_free_dyn_buf(&rtp_vp9_ctx->buf);


if (len < RTP_VP9_DESC_REQUIRED_SIZE + 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet, got %d bytes\n", len);
return AVERROR_INVALIDDATA;
}

















has_pic_id = !!(buf[0] & 0x80);
inter_picture_layer_frame = !!(buf[0] & 0x40);
has_layer_idc = !!(buf[0] & 0x20);
has_ref_idc = !!(buf[0] & 0x10);
first_fragment = !!(buf[0] & 0x08);
last_fragment = !!(buf[0] & 0x04);
has_ss_data = !!(buf[0] & 0x02);

rtp_m = !!(flags & RTP_FLAG_MARKER);


if (last_fragment != rtp_m) {
av_log(ctx, AV_LOG_ERROR, "Invalid combination of B and M marker (%d != %d)\n", last_fragment, rtp_m);
return AVERROR_INVALIDDATA;
}


buf += RTP_VP9_DESC_REQUIRED_SIZE;
len -= RTP_VP9_DESC_REQUIRED_SIZE;














if (has_pic_id) {

if (buf[0] & 0x80) {
if (len < 2) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
pic_id = AV_RB16(buf) & 0x7fff;
buf += 2;
len -= 2;
} else {
pic_id = buf[0] & 0x7f;
buf++;
len--;
}
}













if (has_layer_idc) {
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
layer_temporal = buf[0] & 0xC0;
layer_spatial = buf[0] & 0x30;
layer_quality = buf[0] & 0x0C;
if (has_ref_idc) {
ref_fields = buf[0] & 0x03;
if (ref_fields)
non_key_frame = 1;
}
buf++;
len--;
}















if (has_ref_idc) {
while (ref_fields) {
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}

has_ref_field_ext_pic_id = buf[0] & 0x10;


if (has_ref_field_ext_pic_id) {
if (len < 2) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}



buf += 2;
len -= 2;
} else {



buf++;
len--;
}
ref_fields--;
}
}



















if (has_ss_data) {
int n_s, y, g, i;
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
n_s = buf[0] >> 5;
y = !!(buf[0] & 0x10);
g = !!(buf[0] & 0x08);
buf++;
len--;
if (n_s > 0) {
avpriv_report_missing_feature(ctx, "VP9 scalability structure with multiple layers");
return AVERROR_PATCHWELCOME;
}
if (y) {
if (len < 4 * (n_s + 1)) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
for (i = 0; i < n_s + 1; i++) {
av_unused int w, h;
w = AV_RB16(buf);
h = AV_RB16(buf + 2);
buf += 4;
len -= 4;
}
}
if (g) {
int n_g;
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
n_g = buf[0];
buf++;
len--;
for (i = 0; i < n_g; i++) {
av_unused int t, u, r, j;
if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
t = buf[0] >> 5;
u = !!(buf[0] & 0x10);
r = (buf[0] >> 2) & 0x03;
buf++;
len--;
if (len < r) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}
for (j = 0; j < r; j++) {
av_unused int p_diff = buf[0];
buf++;
len--;
}
}
}
}









if (len < 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/VP9 packet\n");
return AVERROR_INVALIDDATA;
}


if (!rtp_vp9_ctx->buf) {

if (first_fragment) {
res = avio_open_dyn_buf(&rtp_vp9_ctx->buf);
if (res < 0)
return res;

rtp_vp9_ctx->timestamp = *timestamp;
} else {

return AVERROR(EAGAIN);
}
}


avio_write(rtp_vp9_ctx->buf, buf, len);


if (!last_fragment)
return AVERROR(EAGAIN);


res = ff_rtp_finalize_packet(pkt, &rtp_vp9_ctx->buf, st->index);
if (res < 0)
return res;

return 0;
}

static void vp9_close_context(PayloadContext *vp9)
{
ffio_free_dyn_buf(&vp9->buf);
}

const RTPDynamicProtocolHandler ff_vp9_dynamic_handler = {
.enc_name = "VP9",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_VP9,
.priv_data_size = sizeof(PayloadContext),
.init = vp9_init,
.close = vp9_close_context,
.parse_packet = vp9_handle_packet
};
