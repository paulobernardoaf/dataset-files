#include <string.h>
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavcodec/avcodec.h"
#include "internal.h"
#include "rtp.h"
#include "rtpdec.h"
#include "rtpdec_formats.h"
struct PayloadContext {
int block_type; 
int block_size; 
int subpkts_per_block; 
uint16_t len[0x80]; 
uint8_t buf[0x80][0x800]; 
unsigned int cache; 
unsigned int n_pkts; 
uint32_t timestamp; 
};
static int qdm2_parse_config(PayloadContext *qdm, AVStream *st,
const uint8_t *buf, const uint8_t *end)
{
const uint8_t *p = buf;
int ret;
while (end - p >= 2) {
unsigned int item_len = p[0], config_item = p[1];
if (item_len < 2 || end - p < item_len || config_item > 4)
return AVERROR_INVALIDDATA;
switch (config_item) {
case 0: 
return p - buf + item_len;
case 1: 
break;
case 2: 
if (item_len < 3)
return AVERROR_INVALIDDATA;
qdm->subpkts_per_block = p[2];
break;
case 3: 
if (item_len < 4)
return AVERROR_INVALIDDATA;
qdm->block_type = AV_RB16(p + 2);
break;
case 4: 
if (item_len < 30)
return AVERROR_INVALIDDATA;
ret = ff_alloc_extradata(st->codecpar, 26 + item_len);
if (ret < 0) {
return ret;
}
AV_WB32(st->codecpar->extradata, 12);
memcpy(st->codecpar->extradata + 4, "frma", 4);
memcpy(st->codecpar->extradata + 8, "QDM2", 4);
AV_WB32(st->codecpar->extradata + 12, 6 + item_len);
memcpy(st->codecpar->extradata + 16, "QDCA", 4);
memcpy(st->codecpar->extradata + 20, p + 2, item_len - 2);
AV_WB32(st->codecpar->extradata + 18 + item_len, 8);
AV_WB32(st->codecpar->extradata + 22 + item_len, 0);
qdm->block_size = AV_RB32(p + 26);
break;
}
p += item_len;
}
return AVERROR(EAGAIN); 
}
static int qdm2_parse_subpacket(PayloadContext *qdm, AVStream *st,
const uint8_t *buf, const uint8_t *end)
{
const uint8_t *p = buf;
unsigned int id, len, type, to_copy;
id = *p++;
type = *p++;
if (type & 0x80) {
len = AV_RB16(p);
p += 2;
type &= 0x7F;
} else
len = *p++;
if (end - p < len + (type == 0x7F) || id >= 0x80)
return AVERROR_INVALIDDATA;
if (type == 0x7F)
type |= *p++ << 8;
to_copy = FFMIN(len + (p - &buf[1]), 0x800 - qdm->len[id]);
memcpy(&qdm->buf[id][qdm->len[id]], buf + 1, to_copy);
qdm->len[id] += to_copy;
return p + len - buf;
}
static int qdm2_restore_block(PayloadContext *qdm, AVStream *st, AVPacket *pkt)
{
int to_copy, n, res, include_csum;
uint8_t *p, *csum_pos = NULL;
av_assert0(qdm->cache > 0);
for (n = 0; n < 0x80; n++)
if (qdm->len[n] > 0)
break;
av_assert0(n < 0x80);
if ((res = av_new_packet(pkt, qdm->block_size)) < 0)
return res;
memset(pkt->data, 0, pkt->size);
pkt->stream_index = st->index;
p = pkt->data;
if (qdm->len[n] > 0xff) {
*p++ = qdm->block_type | 0x80;
AV_WB16(p, qdm->len[n]);
p += 2;
} else {
*p++ = qdm->block_type;
*p++ = qdm->len[n];
}
if ((include_csum = (qdm->block_type == 2 || qdm->block_type == 4))) {
csum_pos = p;
p += 2;
}
to_copy = FFMIN(qdm->len[n], pkt->size - (p - pkt->data));
memcpy(p, qdm->buf[n], to_copy);
qdm->len[n] = 0;
if (include_csum) {
unsigned int total = 0;
uint8_t *q;
for (q = pkt->data; q < &pkt->data[qdm->block_size]; q++)
total += *q;
AV_WB16(csum_pos, (uint16_t) total);
}
return 0;
}
static int qdm2_parse_packet(AVFormatContext *s, PayloadContext *qdm,
AVStream *st, AVPacket *pkt,
uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
int res = AVERROR_INVALIDDATA, n;
const uint8_t *end = buf + len, *p = buf;
if (len > 0) {
if (len < 2)
return AVERROR_INVALIDDATA;
if (*p == 0xff) {
if (qdm->n_pkts > 0) {
av_log(s, AV_LOG_WARNING,
"Out of sequence config - dropping queue\n");
qdm->n_pkts = 0;
memset(qdm->len, 0, sizeof(qdm->len));
}
if ((res = qdm2_parse_config(qdm, st, ++p, end)) < 0)
return res;
p += res;
st->codecpar->codec_id = AV_CODEC_ID_QDM2;
}
if (st->codecpar->codec_id == AV_CODEC_ID_NONE)
return AVERROR(EAGAIN);
while (end - p >= 4) {
if ((res = qdm2_parse_subpacket(qdm, st, p, end)) < 0)
return res;
p += res;
}
qdm->timestamp = *timestamp;
if (++qdm->n_pkts < qdm->subpkts_per_block)
return AVERROR(EAGAIN);
qdm->cache = 0;
for (n = 0; n < 0x80; n++)
if (qdm->len[n] > 0)
qdm->cache++;
}
if (!qdm->cache || (res = qdm2_restore_block(qdm, st, pkt)) < 0)
return res;
if (--qdm->cache == 0)
qdm->n_pkts = 0;
*timestamp = qdm->timestamp;
qdm->timestamp = RTP_NOTS_VALUE;
return (qdm->cache > 0) ? 1 : 0;
}
const RTPDynamicProtocolHandler ff_qdm2_dynamic_handler = {
.enc_name = "X-QDM",
.codec_type = AVMEDIA_TYPE_AUDIO,
.codec_id = AV_CODEC_ID_NONE,
.priv_data_size = sizeof(PayloadContext),
.parse_packet = qdm2_parse_packet,
};
