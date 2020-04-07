



























#include <string.h>
#include "libavutil/intreadwrite.h"
#include "avio_internal.h"
#include "internal.h"
#include "rtp.h"
#include "rtpdec.h"
#include "rtpdec_formats.h"

struct PayloadContext {
AVIOContext *pktbuf;
int64_t timestamp;
};


static int svq3_parse_packet (AVFormatContext *s, PayloadContext *sv,
AVStream *st, AVPacket *pkt,
uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
int config_packet, start_packet, end_packet;

if (len < 2)
return AVERROR_INVALIDDATA;

config_packet = buf[0] & 0x40;
start_packet = buf[0] & 0x20;
end_packet = buf[0] & 0x10;
buf += 2; 
len -= 2;

if (config_packet) {
if (len < 2 || ff_alloc_extradata(st->codecpar, len + 8))
return AVERROR_INVALIDDATA;

memcpy(st->codecpar->extradata, "SEQH", 4);
AV_WB32(st->codecpar->extradata + 4, len);
memcpy(st->codecpar->extradata + 8, buf, len);






st->codecpar->codec_id = AV_CODEC_ID_SVQ3;

return AVERROR(EAGAIN);
}

if (start_packet) {
int res;

ffio_free_dyn_buf(&sv->pktbuf);
if ((res = avio_open_dyn_buf(&sv->pktbuf)) < 0)
return res;
sv->timestamp = *timestamp;
}

if (!sv->pktbuf)
return AVERROR_INVALIDDATA;

avio_write(sv->pktbuf, buf, len);

if (end_packet) {
int ret = ff_rtp_finalize_packet(pkt, &sv->pktbuf, st->index);
if (ret < 0)
return ret;

*timestamp = sv->timestamp;
return 0;
}

return AVERROR(EAGAIN);
}

static void svq3_close_context(PayloadContext *sv)
{
ffio_free_dyn_buf(&sv->pktbuf);
}

const RTPDynamicProtocolHandler ff_svq3_dynamic_handler = {
.enc_name = "X-SV3V-ES",
.codec_type = AVMEDIA_TYPE_VIDEO,
.codec_id = AV_CODEC_ID_NONE, 
.priv_data_size = sizeof(PayloadContext),
.close = svq3_close_context,
.parse_packet = svq3_parse_packet,
};
