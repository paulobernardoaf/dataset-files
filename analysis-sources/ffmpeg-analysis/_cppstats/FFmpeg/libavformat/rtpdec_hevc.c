#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/base64.h"
#include "libavcodec/get_bits.h"
#include "avformat.h"
#include "internal.h"
#include "rtpdec.h"
#include "rtpdec_formats.h"
#define RTP_HEVC_PAYLOAD_HEADER_SIZE 2
#define RTP_HEVC_FU_HEADER_SIZE 1
#define RTP_HEVC_DONL_FIELD_SIZE 2
#define RTP_HEVC_DOND_FIELD_SIZE 1
#define RTP_HEVC_AP_NALU_LENGTH_FIELD_SIZE 2
#define HEVC_SPECIFIED_NAL_UNIT_TYPES 48
struct PayloadContext {
int using_donl_field;
int profile_id;
uint8_t *sps, *pps, *vps, *sei;
int sps_size, pps_size, vps_size, sei_size;
};
static const uint8_t start_sequence[] = { 0x00, 0x00, 0x00, 0x01 };
static av_cold int hevc_sdp_parse_fmtp_config(AVFormatContext *s,
AVStream *stream,
PayloadContext *hevc_data,
const char *attr, const char *value)
{
if (!strcmp(attr, "profile-id")) {
hevc_data->profile_id = atoi(value);
av_log(s, AV_LOG_TRACE, "SDP: found profile-id: %d\n", hevc_data->profile_id);
}
if (!strcmp(attr, "sprop-vps") || !strcmp(attr, "sprop-sps") ||
!strcmp(attr, "sprop-pps") || !strcmp(attr, "sprop-sei")) {
uint8_t **data_ptr = NULL;
int *size_ptr = NULL;
if (!strcmp(attr, "sprop-vps")) {
data_ptr = &hevc_data->vps;
size_ptr = &hevc_data->vps_size;
} else if (!strcmp(attr, "sprop-sps")) {
data_ptr = &hevc_data->sps;
size_ptr = &hevc_data->sps_size;
} else if (!strcmp(attr, "sprop-pps")) {
data_ptr = &hevc_data->pps;
size_ptr = &hevc_data->pps_size;
} else if (!strcmp(attr, "sprop-sei")) {
data_ptr = &hevc_data->sei;
size_ptr = &hevc_data->sei_size;
} else
av_assert0(0);
ff_h264_parse_sprop_parameter_sets(s, data_ptr,
size_ptr, value);
}
if (!strcmp(attr, "sprop-max-don-diff")) {
if (atoi(value) > 0)
hevc_data->using_donl_field = 1;
av_log(s, AV_LOG_TRACE, "Found sprop-max-don-diff in SDP, DON field usage is: %d\n",
hevc_data->using_donl_field);
}
if (!strcmp(attr, "sprop-depack-buf-nalus")) {
if (atoi(value) > 0)
hevc_data->using_donl_field = 1;
av_log(s, AV_LOG_TRACE, "Found sprop-depack-buf-nalus in SDP, DON field usage is: %d\n",
hevc_data->using_donl_field);
}
return 0;
}
static av_cold int hevc_parse_sdp_line(AVFormatContext *ctx, int st_index,
PayloadContext *hevc_data, const char *line)
{
AVStream *current_stream;
AVCodecParameters *par;
const char *sdp_line_ptr = line;
if (st_index < 0)
return 0;
current_stream = ctx->streams[st_index];
par = current_stream->codecpar;
if (av_strstart(sdp_line_ptr, "framesize:", &sdp_line_ptr)) {
ff_h264_parse_framesize(par, sdp_line_ptr);
} else if (av_strstart(sdp_line_ptr, "fmtp:", &sdp_line_ptr)) {
int ret = ff_parse_fmtp(ctx, current_stream, hevc_data, sdp_line_ptr,
hevc_sdp_parse_fmtp_config);
if (hevc_data->vps_size || hevc_data->sps_size ||
hevc_data->pps_size || hevc_data->sei_size) {
par->extradata_size = hevc_data->vps_size + hevc_data->sps_size +
hevc_data->pps_size + hevc_data->sei_size;
if ((ret = ff_alloc_extradata(par, par->extradata_size)) >= 0) {
int pos = 0;
memcpy(par->extradata + pos, hevc_data->vps, hevc_data->vps_size);
pos += hevc_data->vps_size;
memcpy(par->extradata + pos, hevc_data->sps, hevc_data->sps_size);
pos += hevc_data->sps_size;
memcpy(par->extradata + pos, hevc_data->pps, hevc_data->pps_size);
pos += hevc_data->pps_size;
memcpy(par->extradata + pos, hevc_data->sei, hevc_data->sei_size);
}
av_freep(&hevc_data->vps);
av_freep(&hevc_data->sps);
av_freep(&hevc_data->pps);
av_freep(&hevc_data->sei);
hevc_data->vps_size = 0;
hevc_data->sps_size = 0;
hevc_data->pps_size = 0;
hevc_data->sei_size = 0;
}
return ret;
}
return 0;
}
static int hevc_handle_packet(AVFormatContext *ctx, PayloadContext *rtp_hevc_ctx,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
const uint8_t *rtp_pl = buf;
int tid, lid, nal_type;
int first_fragment, last_fragment, fu_type;
uint8_t new_nal_header[2];
int res = 0;
if (len < RTP_HEVC_PAYLOAD_HEADER_SIZE + 1) {
av_log(ctx, AV_LOG_ERROR, "Too short RTP/HEVC packet, got %d bytes\n", len);
return AVERROR_INVALIDDATA;
}
nal_type = (buf[0] >> 1) & 0x3f;
lid = ((buf[0] << 5) & 0x20) | ((buf[1] >> 3) & 0x1f);
tid = buf[1] & 0x07;
if (lid) {
avpriv_report_missing_feature(ctx, "Multi-layer HEVC coding");
return AVERROR_PATCHWELCOME;
}
if (!tid) {
av_log(ctx, AV_LOG_ERROR, "Illegal temporal ID in RTP/HEVC packet\n");
return AVERROR_INVALIDDATA;
}
if (nal_type > 50) {
av_log(ctx, AV_LOG_ERROR, "Unsupported (HEVC) NAL type (%d)\n", nal_type);
return AVERROR_INVALIDDATA;
}
switch (nal_type) {
case 32:
case 33:
case 34:
case 39:
default:
if ((res = av_new_packet(pkt, sizeof(start_sequence) + len)) < 0)
return res;
memcpy(pkt->data, start_sequence, sizeof(start_sequence));
memcpy(pkt->data + sizeof(start_sequence), buf, len);
break;
case 48:
buf += RTP_HEVC_PAYLOAD_HEADER_SIZE;
len -= RTP_HEVC_PAYLOAD_HEADER_SIZE;
if (rtp_hevc_ctx->using_donl_field) {
buf += RTP_HEVC_DONL_FIELD_SIZE;
len -= RTP_HEVC_DONL_FIELD_SIZE;
}
res = ff_h264_handle_aggregated_packet(ctx, rtp_hevc_ctx, pkt, buf, len,
rtp_hevc_ctx->using_donl_field ?
RTP_HEVC_DOND_FIELD_SIZE : 0,
NULL, 0);
if (res < 0)
return res;
break;
case 49:
buf += RTP_HEVC_PAYLOAD_HEADER_SIZE;
len -= RTP_HEVC_PAYLOAD_HEADER_SIZE;
first_fragment = buf[0] & 0x80;
last_fragment = buf[0] & 0x40;
fu_type = buf[0] & 0x3f;
buf += RTP_HEVC_FU_HEADER_SIZE;
len -= RTP_HEVC_FU_HEADER_SIZE;
if (rtp_hevc_ctx->using_donl_field) {
buf += RTP_HEVC_DONL_FIELD_SIZE;
len -= RTP_HEVC_DONL_FIELD_SIZE;
}
av_log(ctx, AV_LOG_TRACE, " FU type %d with %d bytes\n", fu_type, len);
if (len <= 0) {
if (len < 0) {
av_log(ctx, AV_LOG_ERROR,
"Too short RTP/HEVC packet, got %d bytes of NAL unit type %d\n",
len, nal_type);
return AVERROR_INVALIDDATA;
} else {
return AVERROR(EAGAIN);
}
}
if (first_fragment && last_fragment) {
av_log(ctx, AV_LOG_ERROR, "Illegal combination of S and E bit in RTP/HEVC packet\n");
return AVERROR_INVALIDDATA;
}
new_nal_header[0] = (rtp_pl[0] & 0x81) | (fu_type << 1);
new_nal_header[1] = rtp_pl[1];
res = ff_h264_handle_frag_packet(pkt, buf, len, first_fragment,
new_nal_header, sizeof(new_nal_header));
break;
case 50:
avpriv_report_missing_feature(ctx, "PACI packets for RTP/HEVC");
res = AVERROR_PATCHWELCOME;
break;
}
pkt->stream_index = st->index;
return res;
}
const RTPDynamicProtocolHandler ff_hevc_dynamic_handler = {
.enc_name = "H265",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_HEVC,
.need_parsing = AVSTREAM_PARSE_FULL,
.priv_data_size = sizeof(PayloadContext),
.parse_sdp_a_line = hevc_parse_sdp_line,
.parse_packet = hevc_handle_packet,
};
