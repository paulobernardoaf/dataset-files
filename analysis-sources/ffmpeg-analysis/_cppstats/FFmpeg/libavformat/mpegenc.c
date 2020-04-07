#include <stdint.h>
#include "libavutil/attributes.h"
#include "libavutil/fifo.h"
#include "libavutil/log.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavcodec/put_bits.h"
#include "avformat.h"
#include "internal.h"
#include "mpeg.h"
#define MAX_PAYLOAD_SIZE 4096
typedef struct PacketDesc {
int64_t pts;
int64_t dts;
int size;
int unwritten_size;
struct PacketDesc *next;
} PacketDesc;
typedef struct StreamInfo {
AVFifoBuffer *fifo;
uint8_t id;
int max_buffer_size; 
int buffer_index;
PacketDesc *predecode_packet;
PacketDesc *premux_packet;
PacketDesc **next_packet;
int packet_number;
uint8_t lpcm_header[3];
int lpcm_align;
int bytes_to_iframe;
int align_iframe;
int64_t vobu_start_pts;
} StreamInfo;
typedef struct MpegMuxContext {
const AVClass *class;
int packet_size; 
int packet_number;
int pack_header_freq; 
int system_header_freq;
int system_header_size;
int user_mux_rate; 
int mux_rate; 
int audio_bound;
int video_bound;
int is_mpeg2;
int is_vcd;
int is_svcd;
int is_dvd;
int64_t last_scr; 
int64_t vcd_padding_bitrate_num;
int64_t vcd_padding_bytes_written;
int preload;
} MpegMuxContext;
extern AVOutputFormat ff_mpeg1vcd_muxer;
extern AVOutputFormat ff_mpeg2dvd_muxer;
extern AVOutputFormat ff_mpeg2svcd_muxer;
extern AVOutputFormat ff_mpeg2vob_muxer;
static int put_pack_header(AVFormatContext *ctx, uint8_t *buf,
int64_t timestamp)
{
MpegMuxContext *s = ctx->priv_data;
PutBitContext pb;
init_put_bits(&pb, buf, 128);
put_bits32(&pb, PACK_START_CODE);
if (s->is_mpeg2)
put_bits(&pb, 2, 0x1);
else
put_bits(&pb, 4, 0x2);
put_bits(&pb, 3, (uint32_t)((timestamp >> 30) & 0x07));
put_bits(&pb, 1, 1);
put_bits(&pb, 15, (uint32_t)((timestamp >> 15) & 0x7fff));
put_bits(&pb, 1, 1);
put_bits(&pb, 15, (uint32_t)((timestamp) & 0x7fff));
put_bits(&pb, 1, 1);
if (s->is_mpeg2)
put_bits(&pb, 9, 0);
put_bits(&pb, 1, 1);
put_bits(&pb, 22, s->mux_rate);
put_bits(&pb, 1, 1);
if (s->is_mpeg2) {
put_bits(&pb, 1, 1);
put_bits(&pb, 5, 0x1f); 
put_bits(&pb, 3, 0); 
}
flush_put_bits(&pb);
return put_bits_ptr(&pb) - pb.buf;
}
static int put_system_header(AVFormatContext *ctx, uint8_t *buf,
int only_for_stream_id)
{
MpegMuxContext *s = ctx->priv_data;
int size, i, private_stream_coded, id;
PutBitContext pb;
init_put_bits(&pb, buf, 128);
put_bits32(&pb, SYSTEM_HEADER_START_CODE);
put_bits(&pb, 16, 0);
put_bits(&pb, 1, 1);
put_bits(&pb, 22, s->mux_rate);
put_bits(&pb, 1, 1); 
if (s->is_vcd && only_for_stream_id == VIDEO_ID) {
put_bits(&pb, 6, 0);
} else
put_bits(&pb, 6, s->audio_bound);
if (s->is_vcd) {
put_bits(&pb, 1, 0);
put_bits(&pb, 1, 1);
} else {
put_bits(&pb, 1, 0); 
put_bits(&pb, 1, 0); 
}
if (s->is_vcd || s->is_dvd) {
put_bits(&pb, 1, 1); 
put_bits(&pb, 1, 1); 
} else {
put_bits(&pb, 1, 0); 
put_bits(&pb, 1, 0); 
}
put_bits(&pb, 1, 1); 
if (s->is_vcd && (only_for_stream_id & 0xe0) == AUDIO_ID) {
put_bits(&pb, 5, 0);
} else
put_bits(&pb, 5, s->video_bound);
if (s->is_dvd) {
put_bits(&pb, 1, 0); 
put_bits(&pb, 7, 0x7f); 
} else
put_bits(&pb, 8, 0xff); 
if (s->is_dvd) {
int P_STD_max_video = 0;
int P_STD_max_mpeg_audio = 0;
int P_STD_max_mpeg_PS1 = 0;
for (i = 0; i < ctx->nb_streams; i++) {
StreamInfo *stream = ctx->streams[i]->priv_data;
id = stream->id;
if (id == 0xbd && stream->max_buffer_size > P_STD_max_mpeg_PS1) {
P_STD_max_mpeg_PS1 = stream->max_buffer_size;
} else if (id >= 0xc0 && id <= 0xc7 &&
stream->max_buffer_size > P_STD_max_mpeg_audio) {
P_STD_max_mpeg_audio = stream->max_buffer_size;
} else if (id == 0xe0 &&
stream->max_buffer_size > P_STD_max_video) {
P_STD_max_video = stream->max_buffer_size;
}
}
put_bits(&pb, 8, 0xb9); 
put_bits(&pb, 2, 3);
put_bits(&pb, 1, 1);
put_bits(&pb, 13, P_STD_max_video / 1024);
if (P_STD_max_mpeg_audio == 0)
P_STD_max_mpeg_audio = 4096;
put_bits(&pb, 8, 0xb8); 
put_bits(&pb, 2, 3);
put_bits(&pb, 1, 0);
put_bits(&pb, 13, P_STD_max_mpeg_audio / 128);
put_bits(&pb, 8, 0xbd); 
put_bits(&pb, 2, 3);
put_bits(&pb, 1, 0);
put_bits(&pb, 13, P_STD_max_mpeg_PS1 / 128);
put_bits(&pb, 8, 0xbf); 
put_bits(&pb, 2, 3);
put_bits(&pb, 1, 1);
put_bits(&pb, 13, 2);
} else {
private_stream_coded = 0;
for (i = 0; i < ctx->nb_streams; i++) {
StreamInfo *stream = ctx->streams[i]->priv_data;
if (!s->is_vcd || stream->id == only_for_stream_id ||
only_for_stream_id == 0) {
id = stream->id;
if (id < 0xc0) {
if (private_stream_coded)
continue;
private_stream_coded = 1;
id = 0xbd;
}
put_bits(&pb, 8, id); 
put_bits(&pb, 2, 3);
if (id < 0xe0) {
put_bits(&pb, 1, 0);
put_bits(&pb, 13, stream->max_buffer_size / 128);
} else {
put_bits(&pb, 1, 1);
put_bits(&pb, 13, stream->max_buffer_size / 1024);
}
}
}
}
flush_put_bits(&pb);
size = put_bits_ptr(&pb) - pb.buf;
AV_WB16(buf + 4, size - 6);
return size;
}
static int get_system_header_size(AVFormatContext *ctx)
{
int buf_index, i, private_stream_coded;
StreamInfo *stream;
MpegMuxContext *s = ctx->priv_data;
if (s->is_dvd)
return 18; 
buf_index = 12;
private_stream_coded = 0;
for (i = 0; i < ctx->nb_streams; i++) {
stream = ctx->streams[i]->priv_data;
if (stream->id < 0xc0) {
if (private_stream_coded)
continue;
private_stream_coded = 1;
}
buf_index += 3;
}
return buf_index;
}
static av_cold int mpeg_mux_init(AVFormatContext *ctx)
{
MpegMuxContext *s = ctx->priv_data;
int bitrate, i, mpa_id, mpv_id, h264_id, mps_id, ac3_id, dts_id, lpcm_id, j;
AVStream *st;
StreamInfo *stream;
int audio_bitrate;
int video_bitrate;
s->packet_number = 0;
s->is_vcd = (CONFIG_MPEG1VCD_MUXER && ctx->oformat == &ff_mpeg1vcd_muxer);
s->is_svcd = (CONFIG_MPEG2SVCD_MUXER && ctx->oformat == &ff_mpeg2svcd_muxer);
s->is_mpeg2 = ((CONFIG_MPEG2VOB_MUXER && ctx->oformat == &ff_mpeg2vob_muxer) ||
(CONFIG_MPEG2DVD_MUXER && ctx->oformat == &ff_mpeg2dvd_muxer) ||
(CONFIG_MPEG2SVCD_MUXER && ctx->oformat == &ff_mpeg2svcd_muxer));
s->is_dvd = (CONFIG_MPEG2DVD_MUXER && ctx->oformat == &ff_mpeg2dvd_muxer);
if (ctx->packet_size) {
if (ctx->packet_size < 20 || ctx->packet_size > (1 << 23) + 10) {
av_log(ctx, AV_LOG_ERROR, "Invalid packet size %d\n",
ctx->packet_size);
return AVERROR(EINVAL);
}
s->packet_size = ctx->packet_size;
} else
s->packet_size = 2048;
if (ctx->max_delay < 0) 
ctx->max_delay = AV_TIME_BASE*7/10;
s->vcd_padding_bytes_written = 0;
s->vcd_padding_bitrate_num = 0;
s->audio_bound = 0;
s->video_bound = 0;
mpa_id = AUDIO_ID;
ac3_id = AC3_ID;
dts_id = DTS_ID;
mpv_id = VIDEO_ID;
h264_id = H264_ID;
mps_id = SUB_ID;
lpcm_id = LPCM_ID;
for (i = 0; i < ctx->nb_streams; i++) {
AVCPBProperties *props;
st = ctx->streams[i];
stream = av_mallocz(sizeof(StreamInfo));
if (!stream)
return AVERROR(ENOMEM);
st->priv_data = stream;
avpriv_set_pts_info(st, 64, 1, 90000);
switch (st->codecpar->codec_type) {
case AVMEDIA_TYPE_AUDIO:
if (!s->is_mpeg2 &&
(st->codecpar->codec_id == AV_CODEC_ID_AC3 ||
st->codecpar->codec_id == AV_CODEC_ID_DTS ||
st->codecpar->codec_id == AV_CODEC_ID_PCM_S16BE ||
st->codecpar->codec_id == AV_CODEC_ID_PCM_DVD))
av_log(ctx, AV_LOG_WARNING,
"%s in MPEG-1 system streams is not widely supported, "
"consider using the vob or the dvd muxer "
"to force a MPEG-2 program stream.\n",
avcodec_get_name(st->codecpar->codec_id));
if (st->codecpar->codec_id == AV_CODEC_ID_AC3) {
stream->id = ac3_id++;
} else if (st->codecpar->codec_id == AV_CODEC_ID_DTS) {
stream->id = dts_id++;
} else if (st->codecpar->codec_id == AV_CODEC_ID_PCM_S16BE) {
stream->id = lpcm_id++;
for (j = 0; j < 4; j++) {
if (lpcm_freq_tab[j] == st->codecpar->sample_rate)
break;
}
if (j == 4) {
int sr;
av_log(ctx, AV_LOG_ERROR, "Invalid sampling rate for PCM stream.\n");
av_log(ctx, AV_LOG_INFO, "Allowed sampling rates:");
for (sr = 0; sr < 4; sr++)
av_log(ctx, AV_LOG_INFO, " %d", lpcm_freq_tab[sr]);
av_log(ctx, AV_LOG_INFO, "\n");
return AVERROR(EINVAL);
}
if (st->codecpar->channels > 8) {
av_log(ctx, AV_LOG_ERROR, "At most 8 channels allowed for LPCM streams.\n");
return AVERROR(EINVAL);
}
stream->lpcm_header[0] = 0x0c;
stream->lpcm_header[1] = (st->codecpar->channels - 1) | (j << 4);
stream->lpcm_header[2] = 0x80;
stream->lpcm_align = st->codecpar->channels * 2;
} else if (st->codecpar->codec_id == AV_CODEC_ID_PCM_DVD) {
int freq;
switch (st->codecpar->sample_rate) {
case 48000: freq = 0; break;
case 96000: freq = 1; break;
case 44100: freq = 2; break;
case 32000: freq = 3; break;
default:
av_log(ctx, AV_LOG_ERROR, "Unsupported sample rate.\n");
return AVERROR(EINVAL);
}
stream->lpcm_header[0] = 0x0c;
stream->lpcm_header[1] = (freq << 4) |
(((st->codecpar->bits_per_coded_sample - 16) / 4) << 6) |
st->codecpar->channels - 1;
stream->lpcm_header[2] = 0x80;
stream->id = lpcm_id++;
stream->lpcm_align = st->codecpar->channels * st->codecpar->bits_per_coded_sample / 8;
} else if (st->codecpar->codec_id == AV_CODEC_ID_MLP ||
st->codecpar->codec_id == AV_CODEC_ID_TRUEHD) {
av_log(ctx, AV_LOG_ERROR, "Support for muxing audio codec %s not implemented.\n",
avcodec_get_name(st->codecpar->codec_id));
return AVERROR_PATCHWELCOME;
} else if (st->codecpar->codec_id != AV_CODEC_ID_MP1 &&
st->codecpar->codec_id != AV_CODEC_ID_MP2 &&
st->codecpar->codec_id != AV_CODEC_ID_MP3) {
av_log(ctx, AV_LOG_ERROR, "Unsupported audio codec. Must be one of mp1, mp2, mp3, 16-bit pcm_dvd, pcm_s16be, ac3 or dts.\n");
return AVERROR(EINVAL);
} else {
stream->id = mpa_id++;
}
stream->max_buffer_size = 4 * 1024;
s->audio_bound++;
break;
case AVMEDIA_TYPE_VIDEO:
if (st->codecpar->codec_id == AV_CODEC_ID_H264)
stream->id = h264_id++;
else
stream->id = mpv_id++;
props = (AVCPBProperties*)av_stream_get_side_data(st, AV_PKT_DATA_CPB_PROPERTIES, NULL);
if (props && props->buffer_size)
stream->max_buffer_size = 6 * 1024 + props->buffer_size / 8;
else {
av_log(ctx, AV_LOG_WARNING,
"VBV buffer size not set, using default size of 230KB\n"
"If you want the mpeg file to be compliant to some specification\n"
"Like DVD, VCD or others, make sure you set the correct buffer size\n");
stream->max_buffer_size = 230 * 1024;
}
if (stream->max_buffer_size > 1024 * 8191) {
av_log(ctx, AV_LOG_WARNING, "buffer size %d, too large\n", stream->max_buffer_size);
stream->max_buffer_size = 1024 * 8191;
}
s->video_bound++;
break;
case AVMEDIA_TYPE_SUBTITLE:
stream->id = mps_id++;
stream->max_buffer_size = 16 * 1024;
break;
default:
av_log(ctx, AV_LOG_ERROR, "Invalid media type %s for output stream #%d\n",
av_get_media_type_string(st->codecpar->codec_type), i);
return AVERROR(EINVAL);
}
stream->fifo = av_fifo_alloc(16);
if (!stream->fifo)
return AVERROR(ENOMEM);
}
bitrate = 0;
audio_bitrate = 0;
video_bitrate = 0;
for (i = 0; i < ctx->nb_streams; i++) {
AVCPBProperties *props;
int codec_rate;
st = ctx->streams[i];
stream = (StreamInfo *)st->priv_data;
props = (AVCPBProperties*)av_stream_get_side_data(st, AV_PKT_DATA_CPB_PROPERTIES, NULL);
if (props)
codec_rate = props->max_bitrate;
else
codec_rate = st->codecpar->bit_rate;
if (!codec_rate)
codec_rate = (1 << 21) * 8 * 50 / ctx->nb_streams;
bitrate += codec_rate;
if ((stream->id & 0xe0) == AUDIO_ID)
audio_bitrate += codec_rate;
else if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
video_bitrate += codec_rate;
}
if (s->user_mux_rate) {
s->mux_rate = (s->user_mux_rate + (8 * 50) - 1) / (8 * 50);
} else {
bitrate += bitrate / 20;
bitrate += 10000;
s->mux_rate = (bitrate + (8 * 50) - 1) / (8 * 50);
if (s->mux_rate >= (1<<22)) {
av_log(ctx, AV_LOG_WARNING, "mux rate %d is too large\n", s->mux_rate);
s->mux_rate = (1<<22) - 1;
}
}
if (s->is_vcd) {
int64_t overhead_rate;
overhead_rate = audio_bitrate * 2294LL * (2324 - 2279);
overhead_rate += video_bitrate * 2279LL * (2324 - 2294);
s->vcd_padding_bitrate_num = (2324LL * 75 * 8 - bitrate) * 2279 * 2294 - overhead_rate;
#define VCD_PADDING_BITRATE_DEN (2279 * 2294)
}
if (s->is_vcd || s->is_mpeg2)
s->pack_header_freq = 1;
else
s->pack_header_freq = 2 * bitrate / s->packet_size / 8;
if (s->pack_header_freq == 0)
s->pack_header_freq = 1;
if (s->is_mpeg2)
s->system_header_freq = s->pack_header_freq * 40;
else if (s->is_vcd)
s->system_header_freq = 0x7fffffff;
else
s->system_header_freq = s->pack_header_freq * 5;
for (i = 0; i < ctx->nb_streams; i++) {
stream = ctx->streams[i]->priv_data;
stream->packet_number = 0;
}
s->system_header_size = get_system_header_size(ctx);
s->last_scr = AV_NOPTS_VALUE;
return 0;
}
static inline void put_timestamp(AVIOContext *pb, int id, int64_t timestamp)
{
avio_w8(pb, (id << 4) | (((timestamp >> 30) & 0x07) << 1) | 1);
avio_wb16(pb, (uint16_t)((((timestamp >> 15) & 0x7fff) << 1) | 1));
avio_wb16(pb, (uint16_t)((((timestamp) & 0x7fff) << 1) | 1));
}
static int get_vcd_padding_size(AVFormatContext *ctx, int64_t pts)
{
MpegMuxContext *s = ctx->priv_data;
int pad_bytes = 0;
if (s->vcd_padding_bitrate_num > 0 && pts != AV_NOPTS_VALUE) {
int64_t full_pad_bytes;
full_pad_bytes =
av_rescale(s->vcd_padding_bitrate_num, pts, 90000LL * 8 * VCD_PADDING_BITRATE_DEN);
pad_bytes = (int)(full_pad_bytes - s->vcd_padding_bytes_written);
if (pad_bytes < 0)
pad_bytes = 0;
}
return pad_bytes;
}
static void put_padding_packet(AVFormatContext *ctx, AVIOContext *pb,
int packet_bytes)
{
MpegMuxContext *s = ctx->priv_data;
int i;
avio_wb32(pb, PADDING_STREAM);
avio_wb16(pb, packet_bytes - 6);
if (!s->is_mpeg2) {
avio_w8(pb, 0x0f);
packet_bytes -= 7;
} else
packet_bytes -= 6;
for (i = 0; i < packet_bytes; i++)
avio_w8(pb, 0xff);
}
static int get_nb_frames(AVFormatContext *ctx, StreamInfo *stream, int len)
{
int nb_frames = 0;
PacketDesc *pkt_desc = stream->premux_packet;
while (len > 0) {
if (pkt_desc->size == pkt_desc->unwritten_size)
nb_frames++;
len -= pkt_desc->unwritten_size;
pkt_desc = pkt_desc->next;
}
return nb_frames;
}
static int flush_packet(AVFormatContext *ctx, int stream_index,
int64_t pts, int64_t dts, int64_t scr, int trailer_size)
{
MpegMuxContext *s = ctx->priv_data;
StreamInfo *stream = ctx->streams[stream_index]->priv_data;
uint8_t *buf_ptr;
int size, payload_size, startcode, id, stuffing_size, i, header_len;
int packet_size;
uint8_t buffer[128];
int zero_trail_bytes = 0;
int pad_packet_bytes = 0;
int pes_flags;
int general_pack = 0;
int nb_frames;
id = stream->id;
av_log(ctx, AV_LOG_TRACE, "packet ID=%2x PTS=%0.3f\n", id, pts / 90000.0);
buf_ptr = buffer;
if ((s->packet_number % s->pack_header_freq) == 0 || s->last_scr != scr) {
size = put_pack_header(ctx, buf_ptr, scr);
buf_ptr += size;
s->last_scr = scr;
if (s->is_vcd) {
if (stream->packet_number == 0) {
size = put_system_header(ctx, buf_ptr, id);
buf_ptr += size;
}
} else if (s->is_dvd) {
if (stream->align_iframe || s->packet_number == 0) {
int PES_bytes_to_fill = s->packet_size - size - 10;
if (pts != AV_NOPTS_VALUE) {
if (dts != pts)
PES_bytes_to_fill -= 5 + 5;
else
PES_bytes_to_fill -= 5;
}
if (stream->bytes_to_iframe == 0 || s->packet_number == 0) {
size = put_system_header(ctx, buf_ptr, 0);
buf_ptr += size;
size = buf_ptr - buffer;
avio_write(ctx->pb, buffer, size);
avio_wb32(ctx->pb, PRIVATE_STREAM_2);
avio_wb16(ctx->pb, 0x03d4); 
avio_w8(ctx->pb, 0x00); 
for (i = 0; i < 979; i++)
avio_w8(ctx->pb, 0x00);
avio_wb32(ctx->pb, PRIVATE_STREAM_2);
avio_wb16(ctx->pb, 0x03fa); 
avio_w8(ctx->pb, 0x01); 
for (i = 0; i < 1017; i++)
avio_w8(ctx->pb, 0x00);
memset(buffer, 0, 128);
buf_ptr = buffer;
s->packet_number++;
stream->align_iframe = 0;
scr += s->packet_size * 90000LL /
(s->mux_rate * 50LL);
size = put_pack_header(ctx, buf_ptr, scr);
s->last_scr = scr;
buf_ptr += size;
} else if (stream->bytes_to_iframe < PES_bytes_to_fill) {
pad_packet_bytes = PES_bytes_to_fill -
stream->bytes_to_iframe;
}
}
} else {
if ((s->packet_number % s->system_header_freq) == 0) {
size = put_system_header(ctx, buf_ptr, 0);
buf_ptr += size;
}
}
}
size = buf_ptr - buffer;
avio_write(ctx->pb, buffer, size);
packet_size = s->packet_size - size;
if (s->is_vcd && (id & 0xe0) == AUDIO_ID)
zero_trail_bytes += 20;
if ((s->is_vcd && stream->packet_number == 0) ||
(s->is_svcd && s->packet_number == 0)) {
if (s->is_svcd)
general_pack = 1;
pad_packet_bytes = packet_size - zero_trail_bytes;
}
packet_size -= pad_packet_bytes + zero_trail_bytes;
if (packet_size > 0) {
packet_size -= 6;
if (s->is_mpeg2) {
header_len = 3;
if (stream->packet_number == 0)
header_len += 3; 
header_len += 1; 
} else {
header_len = 0;
}
if (pts != AV_NOPTS_VALUE) {
if (dts != pts)
header_len += 5 + 5;
else
header_len += 5;
} else {
if (!s->is_mpeg2)
header_len++;
}
payload_size = packet_size - header_len;
if (id < 0xc0) {
startcode = PRIVATE_STREAM_1;
payload_size -= 1;
if (id >= 0x40) {
payload_size -= 3;
if (id >= 0xa0)
payload_size -= 3;
}
} else {
startcode = 0x100 + id;
}
stuffing_size = payload_size - av_fifo_size(stream->fifo);
if (payload_size <= trailer_size && pts != AV_NOPTS_VALUE) {
int timestamp_len = 0;
if (dts != pts)
timestamp_len += 5;
if (pts != AV_NOPTS_VALUE)
timestamp_len += s->is_mpeg2 ? 5 : 4;
pts =
dts = AV_NOPTS_VALUE;
header_len -= timestamp_len;
if (s->is_dvd && stream->align_iframe) {
pad_packet_bytes += timestamp_len;
packet_size -= timestamp_len;
} else {
payload_size += timestamp_len;
}
stuffing_size += timestamp_len;
if (payload_size > trailer_size)
stuffing_size += payload_size - trailer_size;
}
if (pad_packet_bytes > 0 && pad_packet_bytes <= 7) {
packet_size += pad_packet_bytes;
payload_size += pad_packet_bytes; 
if (stuffing_size < 0)
stuffing_size = pad_packet_bytes;
else
stuffing_size += pad_packet_bytes;
pad_packet_bytes = 0;
}
if (stuffing_size < 0)
stuffing_size = 0;
if (startcode == PRIVATE_STREAM_1 && id >= 0xa0) {
if (payload_size < av_fifo_size(stream->fifo))
stuffing_size += payload_size % stream->lpcm_align;
}
if (stuffing_size > 16) { 
pad_packet_bytes += stuffing_size;
packet_size -= stuffing_size;
payload_size -= stuffing_size;
stuffing_size = 0;
}
nb_frames = get_nb_frames(ctx, stream, payload_size - stuffing_size);
avio_wb32(ctx->pb, startcode);
avio_wb16(ctx->pb, packet_size);
if (!s->is_mpeg2)
for (i = 0; i < stuffing_size; i++)
avio_w8(ctx->pb, 0xff);
if (s->is_mpeg2) {
avio_w8(ctx->pb, 0x80); 
pes_flags = 0;
if (pts != AV_NOPTS_VALUE) {
pes_flags |= 0x80;
if (dts != pts)
pes_flags |= 0x40;
}
if (stream->packet_number == 0)
pes_flags |= 0x01;
avio_w8(ctx->pb, pes_flags); 
avio_w8(ctx->pb, header_len - 3 + stuffing_size);
if (pes_flags & 0x80) 
put_timestamp(ctx->pb, (pes_flags & 0x40) ? 0x03 : 0x02, pts);
if (pes_flags & 0x40) 
put_timestamp(ctx->pb, 0x01, dts);
if (pes_flags & 0x01) { 
avio_w8(ctx->pb, 0x10); 
if ((id & 0xe0) == AUDIO_ID)
avio_wb16(ctx->pb, 0x4000 | stream->max_buffer_size / 128);
else
avio_wb16(ctx->pb, 0x6000 | stream->max_buffer_size / 1024);
}
} else {
if (pts != AV_NOPTS_VALUE) {
if (dts != pts) {
put_timestamp(ctx->pb, 0x03, pts);
put_timestamp(ctx->pb, 0x01, dts);
} else {
put_timestamp(ctx->pb, 0x02, pts);
}
} else {
avio_w8(ctx->pb, 0x0f);
}
}
if (s->is_mpeg2) {
avio_w8(ctx->pb, 0xff);
for (i = 0; i < stuffing_size; i++)
avio_w8(ctx->pb, 0xff);
}
if (startcode == PRIVATE_STREAM_1) {
avio_w8(ctx->pb, id);
if (id >= 0xa0) {
avio_w8(ctx->pb, 7);
avio_wb16(ctx->pb, 4); 
avio_w8(ctx->pb, stream->lpcm_header[0]);
avio_w8(ctx->pb, stream->lpcm_header[1]);
avio_w8(ctx->pb, stream->lpcm_header[2]);
} else if (id >= 0x40) {
avio_w8(ctx->pb, nb_frames);
avio_wb16(ctx->pb, trailer_size + 1);
}
}
av_assert0(payload_size - stuffing_size <= av_fifo_size(stream->fifo));
av_fifo_generic_read(stream->fifo, ctx->pb,
payload_size - stuffing_size,
(void (*)(void*, void*, int))avio_write);
stream->bytes_to_iframe -= payload_size - stuffing_size;
} else {
payload_size =
stuffing_size = 0;
}
if (pad_packet_bytes > 0)
put_padding_packet(ctx, ctx->pb, pad_packet_bytes);
for (i = 0; i < zero_trail_bytes; i++)
avio_w8(ctx->pb, 0x00);
avio_write_marker(ctx->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_FLUSH_POINT);
s->packet_number++;
if (!general_pack)
stream->packet_number++;
return payload_size - stuffing_size;
}
static void put_vcd_padding_sector(AVFormatContext *ctx)
{
MpegMuxContext *s = ctx->priv_data;
int i;
for (i = 0; i < s->packet_size; i++)
avio_w8(ctx->pb, 0);
s->vcd_padding_bytes_written += s->packet_size;
avio_write_marker(ctx->pb, AV_NOPTS_VALUE, AVIO_DATA_MARKER_FLUSH_POINT);
s->packet_number++;
}
static int remove_decoded_packets(AVFormatContext *ctx, int64_t scr)
{
int i;
for (i = 0; i < ctx->nb_streams; i++) {
AVStream *st = ctx->streams[i];
StreamInfo *stream = st->priv_data;
PacketDesc *pkt_desc;
while ((pkt_desc = stream->predecode_packet) &&
scr > pkt_desc->dts) { 
if (stream->buffer_index < pkt_desc->size ||
stream->predecode_packet == stream->premux_packet) {
av_log(ctx, AV_LOG_ERROR,
"buffer underflow st=%d bufi=%d size=%d\n",
i, stream->buffer_index, pkt_desc->size);
break;
}
stream->buffer_index -= pkt_desc->size;
stream->predecode_packet = pkt_desc->next;
av_freep(&pkt_desc);
}
}
return 0;
}
static int output_packet(AVFormatContext *ctx, int flush)
{
MpegMuxContext *s = ctx->priv_data;
AVStream *st;
StreamInfo *stream;
int i, avail_space = 0, es_size, trailer_size;
int best_i = -1;
int best_score = INT_MIN;
int ignore_constraints = 0;
int ignore_delay = 0;
int64_t scr = s->last_scr;
PacketDesc *timestamp_packet;
const int64_t max_delay = av_rescale(ctx->max_delay, 90000, AV_TIME_BASE);
retry:
for (i = 0; i < ctx->nb_streams; i++) {
AVStream *st = ctx->streams[i];
StreamInfo *stream = st->priv_data;
const int avail_data = av_fifo_size(stream->fifo);
const int space = stream->max_buffer_size - stream->buffer_index;
int rel_space = 1024LL * space / stream->max_buffer_size;
PacketDesc *next_pkt = stream->premux_packet;
if (s->packet_size > avail_data && !flush
&& st->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE)
return 0;
if (avail_data == 0)
continue;
av_assert0(avail_data > 0);
if (space < s->packet_size && !ignore_constraints)
continue;
if (next_pkt && next_pkt->dts - scr > max_delay && !ignore_delay)
continue;
if ( stream->predecode_packet
&& stream->predecode_packet->size > stream->buffer_index)
rel_space += 1<<28;
if (rel_space > best_score) {
best_score = rel_space;
best_i = i;
avail_space = space;
}
}
if (best_i < 0) {
int64_t best_dts = INT64_MAX;
int has_premux = 0;
for (i = 0; i < ctx->nb_streams; i++) {
AVStream *st = ctx->streams[i];
StreamInfo *stream = st->priv_data;
PacketDesc *pkt_desc = stream->predecode_packet;
if (pkt_desc && pkt_desc->dts < best_dts)
best_dts = pkt_desc->dts;
has_premux |= !!stream->premux_packet;
}
if (best_dts < INT64_MAX) {
av_log(ctx, AV_LOG_TRACE, "bumping scr, scr:%f, dts:%f\n",
scr / 90000.0, best_dts / 90000.0);
if (scr >= best_dts + 1 && !ignore_constraints) {
av_log(ctx, AV_LOG_ERROR,
"packet too large, ignoring buffer limits to mux it\n");
ignore_constraints = 1;
}
scr = FFMAX(best_dts + 1, scr);
if (remove_decoded_packets(ctx, scr) < 0)
return -1;
} else if (has_premux && flush) {
av_log(ctx, AV_LOG_ERROR,
"delay too large, ignoring ...\n");
ignore_delay = 1;
ignore_constraints = 1;
} else
return 0;
goto retry;
}
av_assert0(best_i >= 0);
st = ctx->streams[best_i];
stream = st->priv_data;
av_assert0(av_fifo_size(stream->fifo) > 0);
av_assert0(avail_space >= s->packet_size || ignore_constraints);
timestamp_packet = stream->premux_packet;
if (timestamp_packet->unwritten_size == timestamp_packet->size) {
trailer_size = 0;
} else {
trailer_size = timestamp_packet->unwritten_size;
timestamp_packet = timestamp_packet->next;
}
if (timestamp_packet) {
av_log(ctx, AV_LOG_TRACE, "dts:%f pts:%f scr:%f stream:%d\n",
timestamp_packet->dts / 90000.0,
timestamp_packet->pts / 90000.0,
scr / 90000.0, best_i);
es_size = flush_packet(ctx, best_i, timestamp_packet->pts,
timestamp_packet->dts, scr, trailer_size);
} else {
av_assert0(av_fifo_size(stream->fifo) == trailer_size);
es_size = flush_packet(ctx, best_i, AV_NOPTS_VALUE, AV_NOPTS_VALUE, scr,
trailer_size);
}
if (s->is_vcd) {
int vcd_pad_bytes;
while ((vcd_pad_bytes = get_vcd_padding_size(ctx, stream->premux_packet->pts)) >= s->packet_size) {
put_vcd_padding_sector(ctx);
s->last_scr += s->packet_size * 90000LL / (s->mux_rate * 50LL);
}
}
stream->buffer_index += es_size;
s->last_scr += s->packet_size * 90000LL / (s->mux_rate * 50LL);
while (stream->premux_packet &&
stream->premux_packet->unwritten_size <= es_size) {
es_size -= stream->premux_packet->unwritten_size;
stream->premux_packet = stream->premux_packet->next;
}
if (es_size) {
av_assert0(stream->premux_packet);
stream->premux_packet->unwritten_size -= es_size;
}
if (remove_decoded_packets(ctx, s->last_scr) < 0)
return -1;
return 1;
}
static int mpeg_mux_write_packet(AVFormatContext *ctx, AVPacket *pkt)
{
int stream_index = pkt->stream_index;
int size = pkt->size;
uint8_t *buf = pkt->data;
MpegMuxContext *s = ctx->priv_data;
AVStream *st = ctx->streams[stream_index];
StreamInfo *stream = st->priv_data;
int64_t pts, dts;
PacketDesc *pkt_desc;
int preload;
const int is_iframe = st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
(pkt->flags & AV_PKT_FLAG_KEY);
preload = av_rescale(s->preload, 90000, AV_TIME_BASE);
pts = pkt->pts;
dts = pkt->dts;
if (s->last_scr == AV_NOPTS_VALUE) {
if (dts == AV_NOPTS_VALUE || (dts < preload && ctx->avoid_negative_ts) || s->is_dvd) {
if (dts != AV_NOPTS_VALUE)
s->preload += av_rescale(-dts, AV_TIME_BASE, 90000);
s->last_scr = 0;
} else {
s->last_scr = dts - preload;
s->preload = 0;
}
preload = av_rescale(s->preload, 90000, AV_TIME_BASE);
av_log(ctx, AV_LOG_DEBUG, "First SCR: %"PRId64" First DTS: %"PRId64"\n", s->last_scr, dts + preload);
}
if (dts != AV_NOPTS_VALUE) dts += preload;
if (pts != AV_NOPTS_VALUE) pts += preload;
av_log(ctx, AV_LOG_TRACE, "dts:%f pts:%f flags:%d stream:%d nopts:%d\n",
dts / 90000.0, pts / 90000.0, pkt->flags,
pkt->stream_index, pts != AV_NOPTS_VALUE);
if (!stream->premux_packet)
stream->next_packet = &stream->premux_packet;
*stream->next_packet =
pkt_desc = av_mallocz(sizeof(PacketDesc));
if (!pkt_desc)
return AVERROR(ENOMEM);
pkt_desc->pts = pts;
pkt_desc->dts = dts;
if (st->codecpar->codec_id == AV_CODEC_ID_PCM_DVD) {
if (size < 3) {
av_log(ctx, AV_LOG_ERROR, "Invalid packet size %d\n", size);
return AVERROR(EINVAL);
}
buf += 3;
size -= 3;
}
pkt_desc->unwritten_size =
pkt_desc->size = size;
if (!stream->predecode_packet)
stream->predecode_packet = pkt_desc;
stream->next_packet = &pkt_desc->next;
if (av_fifo_realloc2(stream->fifo, av_fifo_size(stream->fifo) + size) < 0)
return -1;
if (s->is_dvd) {
if (is_iframe &&
(s->packet_number == 0 ||
(pts - stream->vobu_start_pts >= 36000))) {
stream->bytes_to_iframe = av_fifo_size(stream->fifo);
stream->align_iframe = 1;
stream->vobu_start_pts = pts;
}
}
av_fifo_generic_write(stream->fifo, buf, size, NULL);
for (;;) {
int ret = output_packet(ctx, 0);
if (ret <= 0)
return ret;
}
}
static int mpeg_mux_end(AVFormatContext *ctx)
{
StreamInfo *stream;
int i;
for (;;) {
int ret = output_packet(ctx, 1);
if (ret < 0)
return ret;
else if (ret == 0)
break;
}
for (i = 0; i < ctx->nb_streams; i++) {
stream = ctx->streams[i]->priv_data;
av_assert0(av_fifo_size(stream->fifo) == 0);
}
return 0;
}
static void mpeg_mux_deinit(AVFormatContext *ctx)
{
for (int i = 0; i < ctx->nb_streams; i++) {
StreamInfo *stream = ctx->streams[i]->priv_data;
if (!stream)
continue;
av_fifo_freep(&stream->fifo);
}
}
#define OFFSET(x) offsetof(MpegMuxContext, x)
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "muxrate", NULL, OFFSET(user_mux_rate), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, ((1<<22) - 1) * (8 * 50), E },
{ "preload", "Initial demux-decode delay in microseconds.", OFFSET(preload), AV_OPT_TYPE_INT, { .i64 = 500000 }, 0, INT_MAX, E },
{ NULL },
};
#define MPEGENC_CLASS(flavor) static const AVClass flavor ##_class = { .class_name = #flavor " muxer", .item_name = av_default_item_name, .version = LIBAVUTIL_VERSION_INT, .option = options, };
#if CONFIG_MPEG1SYSTEM_MUXER
MPEGENC_CLASS(mpeg)
AVOutputFormat ff_mpeg1system_muxer = {
.name = "mpeg",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-1 Systems / MPEG program stream"),
.mime_type = "video/mpeg",
.extensions = "mpg,mpeg",
.priv_data_size = sizeof(MpegMuxContext),
.audio_codec = AV_CODEC_ID_MP2,
.video_codec = AV_CODEC_ID_MPEG1VIDEO,
.write_header = mpeg_mux_init,
.write_packet = mpeg_mux_write_packet,
.write_trailer = mpeg_mux_end,
.deinit = mpeg_mux_deinit,
.priv_class = &mpeg_class,
};
#endif
#if CONFIG_MPEG1VCD_MUXER
MPEGENC_CLASS(vcd)
AVOutputFormat ff_mpeg1vcd_muxer = {
.name = "vcd",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-1 Systems / MPEG program stream (VCD)"),
.mime_type = "video/mpeg",
.priv_data_size = sizeof(MpegMuxContext),
.audio_codec = AV_CODEC_ID_MP2,
.video_codec = AV_CODEC_ID_MPEG1VIDEO,
.write_header = mpeg_mux_init,
.write_packet = mpeg_mux_write_packet,
.write_trailer = mpeg_mux_end,
.deinit = mpeg_mux_deinit,
.priv_class = &vcd_class,
};
#endif
#if CONFIG_MPEG2VOB_MUXER
MPEGENC_CLASS(vob)
AVOutputFormat ff_mpeg2vob_muxer = {
.name = "vob",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-2 PS (VOB)"),
.mime_type = "video/mpeg",
.extensions = "vob",
.priv_data_size = sizeof(MpegMuxContext),
.audio_codec = AV_CODEC_ID_MP2,
.video_codec = AV_CODEC_ID_MPEG2VIDEO,
.write_header = mpeg_mux_init,
.write_packet = mpeg_mux_write_packet,
.write_trailer = mpeg_mux_end,
.deinit = mpeg_mux_deinit,
.priv_class = &vob_class,
};
#endif
#if CONFIG_MPEG2SVCD_MUXER
MPEGENC_CLASS(svcd)
AVOutputFormat ff_mpeg2svcd_muxer = {
.name = "svcd",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-2 PS (SVCD)"),
.mime_type = "video/mpeg",
.extensions = "vob",
.priv_data_size = sizeof(MpegMuxContext),
.audio_codec = AV_CODEC_ID_MP2,
.video_codec = AV_CODEC_ID_MPEG2VIDEO,
.write_header = mpeg_mux_init,
.write_packet = mpeg_mux_write_packet,
.write_trailer = mpeg_mux_end,
.deinit = mpeg_mux_deinit,
.priv_class = &svcd_class,
};
#endif
#if CONFIG_MPEG2DVD_MUXER
MPEGENC_CLASS(dvd)
AVOutputFormat ff_mpeg2dvd_muxer = {
.name = "dvd",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-2 PS (DVD VOB)"),
.mime_type = "video/mpeg",
.extensions = "dvd",
.priv_data_size = sizeof(MpegMuxContext),
.audio_codec = AV_CODEC_ID_MP2,
.video_codec = AV_CODEC_ID_MPEG2VIDEO,
.write_header = mpeg_mux_init,
.write_packet = mpeg_mux_write_packet,
.write_trailer = mpeg_mux_end,
.deinit = mpeg_mux_deinit,
.priv_class = &dvd_class,
};
#endif
