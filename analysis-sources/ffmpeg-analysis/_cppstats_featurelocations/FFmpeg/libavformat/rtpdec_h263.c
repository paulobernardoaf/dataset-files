




















#include "avformat.h"
#include "rtpdec_formats.h"
#include "libavutil/attributes.h"
#include "libavutil/intreadwrite.h"

int ff_h263_handle_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq, int flags)
{
uint8_t *ptr;
uint16_t header;
int startcode, vrc, picture_header, ret;

if (len < 2) {
av_log(ctx, AV_LOG_ERROR, "Too short H.263 RTP packet\n");
return AVERROR_INVALIDDATA;
}














header = AV_RB16(buf);
startcode = (header & 0x0400) >> 9;
vrc = header & 0x0200;
picture_header = (header & 0x01f8) >> 3;
buf += 2;
len -= 2;

if (vrc) {

buf += 1;
len -= 1;
}
if (picture_header) {

buf += picture_header;
len -= picture_header;
}

if (len < 0) {
av_log(ctx, AV_LOG_ERROR, "Too short H.263 RTP packet\n");
return AVERROR_INVALIDDATA;
}

if ((ret = av_new_packet(pkt, len + startcode)) < 0) {
av_log(ctx, AV_LOG_ERROR, "Out of memory\n");
return ret;
}
pkt->stream_index = st->index;
ptr = pkt->data;

if (startcode) {
*ptr++ = 0;
*ptr++ = 0;
}
memcpy(ptr, buf, len);

return 0;
}

const RTPDynamicProtocolHandler ff_h263_1998_dynamic_handler = {
.enc_name = "H263-1998",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_H263,
.need_parsing = AVSTREAM_PARSE_FULL,
.parse_packet = ff_h263_handle_packet,
};

const RTPDynamicProtocolHandler ff_h263_2000_dynamic_handler = {
.enc_name = "H263-2000",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_H263,
.need_parsing = AVSTREAM_PARSE_FULL,
.parse_packet = ff_h263_handle_packet,
};
