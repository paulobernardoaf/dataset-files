


























#include "avformat.h"
#include "avio_internal.h"
#include "rtpdec_formats.h"
#include "libavutil/attributes.h"
#include "libavutil/intreadwrite.h"
#include "libavcodec/get_bits.h"

struct PayloadContext {
AVIOContext *buf;
uint8_t endbyte;
int endbyte_bits;
uint32_t timestamp;
int newformat;
};

static void h263_close_context(PayloadContext *data)
{
ffio_free_dyn_buf(&data->buf);
}

static int h263_handle_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{

int f, p, i, sbit, ebit, src, r;
int header_size, ret;

if (data->newformat)
return ff_h263_handle_packet(ctx, data, st, pkt, timestamp, buf, len,
seq, flags);

if (data->buf && data->timestamp != *timestamp) {

ffio_free_dyn_buf(&data->buf);
data->endbyte_bits = 0;
}

if (len < 4) {
av_log(ctx, AV_LOG_ERROR, "Too short H.263 RTP packet: %d\n", len);
return AVERROR_INVALIDDATA;
}

f = buf[0] & 0x80;
p = buf[0] & 0x40;
if (!f) {

header_size = 4;
i = buf[1] & 0x10;
r = ((buf[1] & 0x01) << 3) | ((buf[2] & 0xe0) >> 5);
} else if (!p) {

header_size = 8;
if (len < header_size) {
av_log(ctx, AV_LOG_ERROR,
"Too short H.263 RTP packet: %d bytes, %d header bytes\n",
len, header_size);
return AVERROR_INVALIDDATA;
}
r = buf[3] & 0x03;
i = buf[4] & 0x80;
} else {

header_size = 12;
if (len < header_size) {
av_log(ctx, AV_LOG_ERROR,
"Too short H.263 RTP packet: %d bytes, %d header bytes\n",
len, header_size);
return AVERROR_INVALIDDATA;
}
r = buf[3] & 0x03;
i = buf[4] & 0x80;
}
sbit = (buf[0] >> 3) & 0x7;
ebit = buf[0] & 0x7;
src = (buf[1] & 0xe0) >> 5;
if (!(buf[0] & 0xf8)) { 
if ((src == 0 || src >= 6) && r) {


av_log(ctx, AV_LOG_WARNING,
"Interpreting H.263 RTP data as RFC 2429/4629 even though "
"signalled with a static payload type.\n");
data->newformat = 1;
return ff_h263_handle_packet(ctx, data, st, pkt, timestamp, buf,
len, seq, flags);
}
}

buf += header_size;
len -= header_size;

if (!data->buf) {


if (len > 4 && AV_RB32(buf) >> 10 == 0x20) {
ret = avio_open_dyn_buf(&data->buf);
if (ret < 0)
return ret;
data->timestamp = *timestamp;
} else {

return AVERROR(EAGAIN);
}
}

if (data->endbyte_bits || sbit) {
if (data->endbyte_bits == sbit) {
data->endbyte |= buf[0] & (0xff >> sbit);
data->endbyte_bits = 0;
buf++;
len--;
avio_w8(data->buf, data->endbyte);
} else {

GetBitContext gb;
init_get_bits(&gb, buf, len*8 - ebit);
skip_bits(&gb, sbit);
if (data->endbyte_bits) {
data->endbyte |= get_bits(&gb, 8 - data->endbyte_bits);
avio_w8(data->buf, data->endbyte);
}
while (get_bits_left(&gb) >= 8)
avio_w8(data->buf, get_bits(&gb, 8));
data->endbyte_bits = get_bits_left(&gb);
if (data->endbyte_bits)
data->endbyte = get_bits(&gb, data->endbyte_bits) <<
(8 - data->endbyte_bits);
ebit = 0;
len = 0;
}
}
if (ebit) {
if (len > 0)
avio_write(data->buf, buf, len - 1);
data->endbyte_bits = 8 - ebit;
data->endbyte = buf[len - 1] & (0xff << ebit);
} else {
avio_write(data->buf, buf, len);
}

if (!(flags & RTP_FLAG_MARKER))
return AVERROR(EAGAIN);

if (data->endbyte_bits)
avio_w8(data->buf, data->endbyte);
data->endbyte_bits = 0;

ret = ff_rtp_finalize_packet(pkt, &data->buf, st->index);
if (ret < 0)
return ret;
if (!i)
pkt->flags |= AV_PKT_FLAG_KEY;

return 0;
}

const RTPDynamicProtocolHandler ff_h263_rfc2190_dynamic_handler = {
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_H263,
.need_parsing = AVSTREAM_PARSE_FULL,
.parse_packet = h263_handle_packet,
.priv_data_size = sizeof(PayloadContext),
.close = h263_close_context,
.static_payload_id = 34,
};
