




















#include "libavutil/attributes.h"
#include "mpegts.h"
#include "rtpdec_formats.h"

struct PayloadContext {
struct MpegTSContext *ts;
int read_buf_index;
int read_buf_size;
uint8_t buf[RTP_MAX_PACKET_LENGTH];
};

static void mpegts_close_context(PayloadContext *data)
{
if (!data)
return;
if (data->ts)
avpriv_mpegts_parse_close(data->ts);
}

static av_cold int mpegts_init(AVFormatContext *ctx, int st_index,
PayloadContext *data)
{
data->ts = avpriv_mpegts_parse_open(ctx);
if (!data->ts)
return AVERROR(ENOMEM);
return 0;
}

static int mpegts_handle_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
int ret;





*timestamp = RTP_NOTS_VALUE;

if (!buf) {
if (data->read_buf_index >= data->read_buf_size)
return AVERROR(EAGAIN);
ret = avpriv_mpegts_parse_packet(data->ts, pkt, data->buf + data->read_buf_index,
data->read_buf_size - data->read_buf_index);
if (ret < 0)
return AVERROR(EAGAIN);
data->read_buf_index += ret;
if (data->read_buf_index < data->read_buf_size)
return 1;
else
return 0;
}

ret = avpriv_mpegts_parse_packet(data->ts, pkt, buf, len);



if (ret < 0)
return AVERROR(EAGAIN);
if (ret < len) {
data->read_buf_size = FFMIN(len - ret, sizeof(data->buf));
memcpy(data->buf, buf + ret, data->read_buf_size);
data->read_buf_index = 0;
return 1;
}
return 0;
}

const RTPDynamicProtocolHandler ff_mpegts_dynamic_handler = {
.codec_type = AVMEDIA_TYPE_DATA,
.priv_data_size = sizeof(PayloadContext),
.parse_packet = mpegts_handle_packet,
.init = mpegts_init,
.close = mpegts_close_context,
.static_payload_id = 33,
};
