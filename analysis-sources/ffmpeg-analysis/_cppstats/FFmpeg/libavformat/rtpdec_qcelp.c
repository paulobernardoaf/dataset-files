#include "rtpdec_formats.h"
static const uint8_t frame_sizes[] = {
1, 4, 8, 17, 35
};
typedef struct InterleavePacket {
int pos;
int size;
uint8_t data[35*9];
} InterleavePacket;
struct PayloadContext {
int interleave_size;
int interleave_index;
InterleavePacket group[6];
int group_finished;
uint8_t next_data[1 + 35*10];
int next_size;
uint32_t next_timestamp;
};
static int return_stored_frame(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len);
static int store_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len)
{
int interleave_size, interleave_index;
int frame_size, ret;
InterleavePacket* ip;
if (len < 2)
return AVERROR_INVALIDDATA;
interleave_size = buf[0] >> 3 & 7;
interleave_index = buf[0] & 7;
if (interleave_size > 5) {
av_log(ctx, AV_LOG_ERROR, "Invalid interleave size %d\n",
interleave_size);
return AVERROR_INVALIDDATA;
}
if (interleave_index > interleave_size) {
av_log(ctx, AV_LOG_ERROR, "Invalid interleave index %d/%d\n",
interleave_index, interleave_size);
return AVERROR_INVALIDDATA;
}
if (interleave_size != data->interleave_size) {
int i;
data->interleave_size = interleave_size;
data->interleave_index = 0;
for (i = 0; i < 6; i++)
data->group[i].size = 0;
}
if (interleave_index < data->interleave_index) {
if (data->group_finished) {
data->interleave_index = 0;
} else {
for (; data->interleave_index <= interleave_size;
data->interleave_index++)
data->group[data->interleave_index].size = 0;
if (len > sizeof(data->next_data))
return AVERROR_INVALIDDATA;
memcpy(data->next_data, buf, len);
data->next_size = len;
data->next_timestamp = *timestamp;
*timestamp = RTP_NOTS_VALUE;
data->interleave_index = 0;
return return_stored_frame(ctx, data, st, pkt, timestamp, buf, len);
}
}
if (interleave_index > data->interleave_index) {
for (; data->interleave_index < interleave_index;
data->interleave_index++)
data->group[data->interleave_index].size = 0;
}
data->interleave_index = interleave_index;
if (buf[1] >= FF_ARRAY_ELEMS(frame_sizes))
return AVERROR_INVALIDDATA;
frame_size = frame_sizes[buf[1]];
if (1 + frame_size > len)
return AVERROR_INVALIDDATA;
if (len - 1 - frame_size > sizeof(data->group[0].data))
return AVERROR_INVALIDDATA;
if ((ret = av_new_packet(pkt, frame_size)) < 0)
return ret;
memcpy(pkt->data, &buf[1], frame_size);
pkt->stream_index = st->index;
ip = &data->group[data->interleave_index];
ip->size = len - 1 - frame_size;
ip->pos = 0;
memcpy(ip->data, &buf[1 + frame_size], ip->size);
data->group_finished = ip->size == 0;
if (interleave_index == interleave_size) {
data->interleave_index = 0;
return !data->group_finished;
} else {
data->interleave_index++;
return 0;
}
}
static int return_stored_frame(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len)
{
InterleavePacket* ip = &data->group[data->interleave_index];
int frame_size, ret;
if (data->group_finished && data->interleave_index == 0) {
*timestamp = data->next_timestamp;
ret = store_packet(ctx, data, st, pkt, timestamp, data->next_data,
data->next_size);
data->next_size = 0;
return ret;
}
if (ip->size == 0) {
if ((ret = av_new_packet(pkt, 1)) < 0)
return ret;
pkt->data[0] = 0; 
} else {
if (ip->pos >= ip->size)
return AVERROR_INVALIDDATA;
if (ip->data[ip->pos] >= FF_ARRAY_ELEMS(frame_sizes))
return AVERROR_INVALIDDATA;
frame_size = frame_sizes[ip->data[ip->pos]];
if (ip->pos + frame_size > ip->size)
return AVERROR_INVALIDDATA;
if ((ret = av_new_packet(pkt, frame_size)) < 0)
return ret;
memcpy(pkt->data, &ip->data[ip->pos], frame_size);
ip->pos += frame_size;
data->group_finished = ip->pos >= ip->size;
}
pkt->stream_index = st->index;
if (data->interleave_index == data->interleave_size) {
data->interleave_index = 0;
if (!data->group_finished)
return 1;
else
return data->next_size > 0;
} else {
data->interleave_index++;
return 1;
}
}
static int qcelp_parse_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
if (buf)
return store_packet(ctx, data, st, pkt, timestamp, buf, len);
else
return return_stored_frame(ctx, data, st, pkt, timestamp, buf, len);
}
const RTPDynamicProtocolHandler ff_qcelp_dynamic_handler = {
.enc_name = "x-Purevoice",
.codec_type = AVMEDIA_TYPE_AUDIO,
.codec_id = AV_CODEC_ID_QCELP,
.priv_data_size = sizeof(PayloadContext),
.static_payload_id = 12,
.parse_packet = qcelp_parse_packet,
};
