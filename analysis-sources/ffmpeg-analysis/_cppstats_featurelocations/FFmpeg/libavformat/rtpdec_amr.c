




















#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "rtpdec_formats.h"
#include "libavutil/avstring.h"

static const uint8_t frame_sizes_nb[16] = {
12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};
static const uint8_t frame_sizes_wb[16] = {
17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0
};

struct PayloadContext {
int octet_align;
int crc;
int interleaving;
int channels;
};

static av_cold int amr_init(AVFormatContext *s, int st_index, PayloadContext *data)
{
data->channels = 1;
return 0;
}

static int amr_handle_packet(AVFormatContext *ctx, PayloadContext *data,
AVStream *st, AVPacket *pkt, uint32_t *timestamp,
const uint8_t *buf, int len, uint16_t seq,
int flags)
{
const uint8_t *frame_sizes = NULL;
int frames;
int i, ret;
const uint8_t *speech_data;
uint8_t *ptr;

if (st->codecpar->codec_id == AV_CODEC_ID_AMR_NB) {
frame_sizes = frame_sizes_nb;
} else if (st->codecpar->codec_id == AV_CODEC_ID_AMR_WB) {
frame_sizes = frame_sizes_wb;
} else {
av_log(ctx, AV_LOG_ERROR, "Bad codec ID\n");
return AVERROR_INVALIDDATA;
}

if (st->codecpar->channels != 1) {
av_log(ctx, AV_LOG_ERROR, "Only mono AMR is supported\n");
return AVERROR_INVALIDDATA;
}
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;













for (frames = 1; frames < len && (buf[frames] & 0x80); frames++) ;

if (1 + frames >= len) {

av_log(ctx, AV_LOG_ERROR, "No speech data found\n");
return AVERROR_INVALIDDATA;
}

speech_data = buf + 1 + frames;


if ((ret = av_new_packet(pkt, len - 1)) < 0) {
av_log(ctx, AV_LOG_ERROR, "Out of memory\n");
return ret;
}
pkt->stream_index = st->index;
ptr = pkt->data;

for (i = 0; i < frames; i++) {
uint8_t toc = buf[1 + i];
int frame_size = frame_sizes[(toc >> 3) & 0x0f];

if (speech_data + frame_size > buf + len) {

av_log(ctx, AV_LOG_WARNING, "Too little speech data in the RTP packet\n");

memset(ptr, 0, pkt->data + pkt->size - ptr);
pkt->size = ptr - pkt->data;
return 0;
}


*ptr++ = toc & 0x7C;


memcpy(ptr, speech_data, frame_size);
speech_data += frame_size;
ptr += frame_size;
}

if (speech_data < buf + len) {
av_log(ctx, AV_LOG_WARNING, "Too much speech data in the RTP packet?\n");

memset(ptr, 0, pkt->data + pkt->size - ptr);
pkt->size = ptr - pkt->data;
}

return 0;
}

static int amr_parse_fmtp(AVFormatContext *s,
AVStream *stream, PayloadContext *data,
const char *attr, const char *value)
{




if (!strcmp(value, "")) {
av_log(s, AV_LOG_WARNING, "AMR fmtp attribute %s had "
"nonstandard empty value\n", attr);
value = "1";
}
if (!strcmp(attr, "octet-align"))
data->octet_align = atoi(value);
else if (!strcmp(attr, "crc"))
data->crc = atoi(value);
else if (!strcmp(attr, "interleaving"))
data->interleaving = atoi(value);
else if (!strcmp(attr, "channels"))
data->channels = atoi(value);
return 0;
}

static int amr_parse_sdp_line(AVFormatContext *s, int st_index,
PayloadContext *data, const char *line)
{
const char *p;
int ret;

if (st_index < 0)
return 0;






if (av_strstart(line, "fmtp:", &p)) {
ret = ff_parse_fmtp(s, s->streams[st_index], data, p, amr_parse_fmtp);
if (!data->octet_align || data->crc ||
data->interleaving || data->channels != 1) {
av_log(s, AV_LOG_ERROR, "Unsupported RTP/AMR configuration!\n");
return -1;
}
return ret;
}
return 0;
}

const RTPDynamicProtocolHandler ff_amr_nb_dynamic_handler = {
.enc_name = "AMR",
.codec_type = AVMEDIA_TYPE_AUDIO,
.codec_id = AV_CODEC_ID_AMR_NB,
.priv_data_size = sizeof(PayloadContext),
.init = amr_init,
.parse_sdp_a_line = amr_parse_sdp_line,
.parse_packet = amr_handle_packet,
};

const RTPDynamicProtocolHandler ff_amr_wb_dynamic_handler = {
.enc_name = "AMR-WB",
.codec_type = AVMEDIA_TYPE_AUDIO,
.codec_id = AV_CODEC_ID_AMR_WB,
.priv_data_size = sizeof(PayloadContext),
.init = amr_init,
.parse_sdp_a_line = amr_parse_sdp_line,
.parse_packet = amr_handle_packet,
};
