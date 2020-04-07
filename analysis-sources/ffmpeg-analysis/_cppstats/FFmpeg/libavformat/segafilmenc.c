#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#include "avio_internal.h"
typedef struct FILMPacket {
int audio;
int keyframe;
int32_t pts;
int32_t duration;
int32_t size;
int32_t index;
struct FILMPacket *next;
} FILMPacket;
typedef struct FILMOutputContext {
int audio_index;
int video_index;
int64_t stab_pos;
FILMPacket *start;
FILMPacket *last;
int64_t packet_count;
} FILMOutputContext;
static int film_write_packet_to_header(AVFormatContext *format_context, FILMPacket *pkt)
{
AVIOContext *pb = format_context->pb;
int32_t info1 = 0;
int32_t info2 = 0;
if (pkt->audio) {
info1 = 0xFFFFFFFF;
info2 = 1;
} else {
info1 = pkt->pts;
info2 = pkt->duration;
if (!pkt->keyframe)
info1 |= 1U << 31;
}
avio_wb32(pb, pkt->index);
avio_wb32(pb, pkt->size);
avio_wb32(pb, info1);
avio_wb32(pb, info2);
return 0;
}
static int film_write_packet(AVFormatContext *format_context, AVPacket *pkt)
{
FILMPacket *metadata;
AVIOContext *pb = format_context->pb;
FILMOutputContext *film = format_context->priv_data;
int encoded_buf_size = 0;
enum AVCodecID codec_id;
metadata = av_mallocz(sizeof(FILMPacket));
if (!metadata)
return AVERROR(ENOMEM);
metadata->audio = pkt->stream_index == film->audio_index;
metadata->keyframe = pkt->flags & AV_PKT_FLAG_KEY;
metadata->pts = pkt->pts;
metadata->duration = pkt->duration;
metadata->size = pkt->size;
if (film->last == NULL) {
metadata->index = 0;
} else {
metadata->index = film->last->index + film->last->size;
film->last->next = metadata;
}
metadata->next = NULL;
if (film->start == NULL)
film->start = metadata;
film->packet_count++;
film->last = metadata;
codec_id = format_context->streams[pkt->stream_index]->codecpar->codec_id;
if (codec_id == AV_CODEC_ID_CINEPAK) {
encoded_buf_size = AV_RB24(&pkt->data[1]);
if (encoded_buf_size != pkt->size && (pkt->size % encoded_buf_size) != 0) {
avio_write(pb, pkt->data, pkt->size);
} else {
uint8_t padding[2] = {0, 0};
AV_WB24(&pkt->data[1], pkt->size - 8 + 2);
metadata->size += 2;
avio_write(pb, pkt->data, 10);
avio_write(pb, padding, 2);
avio_write(pb, &pkt->data[10], pkt->size - 10);
}
} else {
avio_write(pb, pkt->data, pkt->size);
}
return 0;
}
static int get_audio_codec_id(enum AVCodecID codec_id)
{
switch (codec_id) {
case AV_CODEC_ID_PCM_S8_PLANAR:
case AV_CODEC_ID_PCM_S16BE_PLANAR:
return 0;
case AV_CODEC_ID_ADPCM_ADX:
return 2;
default:
return -1;
}
}
static int film_init(AVFormatContext *format_context)
{
FILMOutputContext *film = format_context->priv_data;
film->audio_index = -1;
film->video_index = -1;
film->stab_pos = 0;
film->packet_count = 0;
film->start = NULL;
film->last = NULL;
for (int i = 0; i < format_context->nb_streams; i++) {
AVStream *st = format_context->streams[i];
if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
if (film->audio_index > -1) {
av_log(format_context, AV_LOG_ERROR, "Sega FILM allows a maximum of one audio stream.\n");
return AVERROR(EINVAL);
}
if (get_audio_codec_id(st->codecpar->codec_id) < 0) {
av_log(format_context, AV_LOG_ERROR,
"Incompatible audio stream format.\n");
return AVERROR(EINVAL);
}
film->audio_index = i;
}
if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
if (film->video_index > -1) {
av_log(format_context, AV_LOG_ERROR, "Sega FILM allows a maximum of one video stream.\n");
return AVERROR(EINVAL);
}
if (st->codecpar->codec_id != AV_CODEC_ID_CINEPAK &&
st->codecpar->codec_id != AV_CODEC_ID_RAWVIDEO) {
av_log(format_context, AV_LOG_ERROR,
"Incompatible video stream format.\n");
return AVERROR(EINVAL);
}
if (st->codecpar->format != AV_PIX_FMT_RGB24) {
av_log(format_context, AV_LOG_ERROR,
"Pixel format must be rgb24.\n");
return AVERROR(EINVAL);
}
film->video_index = i;
}
}
if (film->video_index == -1) {
av_log(format_context, AV_LOG_ERROR, "No video stream present.\n");
return AVERROR(EINVAL);
}
return 0;
}
static int shift_data(AVFormatContext *format_context, int64_t shift_size)
{
int ret = 0;
int64_t pos, pos_end;
uint8_t *buf, *read_buf[2];
int read_buf_id = 0;
int read_size[2];
AVIOContext *read_pb;
buf = av_malloc(shift_size * 2);
if (!buf)
return AVERROR(ENOMEM);
read_buf[0] = buf;
read_buf[1] = buf + shift_size;
avio_flush(format_context->pb);
ret = format_context->io_open(format_context, &read_pb, format_context->url, AVIO_FLAG_READ, NULL);
if (ret < 0) {
av_log(format_context, AV_LOG_ERROR, "Unable to re-open %s output file to "
"write the header\n", format_context->url);
av_free(buf);
return ret;
}
pos_end = avio_tell(format_context->pb);
avio_seek(format_context->pb, shift_size, SEEK_SET);
avio_seek(read_pb, 0, SEEK_SET);
pos = avio_tell(read_pb);
#define READ_BLOCK do { read_size[read_buf_id] = avio_read(read_pb, read_buf[read_buf_id], shift_size); read_buf_id ^= 1; } while (0)
READ_BLOCK;
do {
int n;
READ_BLOCK;
n = read_size[read_buf_id];
if (n <= 0)
break;
avio_write(format_context->pb, read_buf[read_buf_id], n);
pos += n;
} while (pos < pos_end);
ff_format_io_close(format_context, &read_pb);
av_free(buf);
return 0;
}
static int film_write_header(AVFormatContext *format_context)
{
int ret = 0;
int64_t sample_table_size, stabsize, headersize;
AVIOContext *pb = format_context->pb;
FILMOutputContext *film = format_context->priv_data;
FILMPacket *prev, *packet;
AVStream *video = NULL;
sample_table_size = film->packet_count * 16;
stabsize = 16 + sample_table_size;
headersize = 16 + 
32 + 
stabsize;
ret = shift_data(format_context, headersize);
if (ret < 0)
return ret;
avio_seek(pb, 0, SEEK_SET);
ffio_wfourcc(pb, "FILM");
avio_wb32(pb, 48 + stabsize);
ffio_wfourcc(pb, "1.09");
avio_wb32(pb, 0);
ffio_wfourcc(pb, "FDSC");
avio_wb32(pb, 0x20); 
video = format_context->streams[film->video_index];
switch (video->codecpar->codec_id) {
case AV_CODEC_ID_CINEPAK:
ffio_wfourcc(pb, "cvid");
break;
case AV_CODEC_ID_RAWVIDEO:
ffio_wfourcc(pb, "raw ");
break;
}
avio_wb32(pb, video->codecpar->height);
avio_wb32(pb, video->codecpar->width);
avio_w8(pb, 24); 
if (film->audio_index > -1) {
AVStream *audio = format_context->streams[film->audio_index];
int audio_codec = get_audio_codec_id(audio->codecpar->codec_id);
avio_w8(pb, audio->codecpar->channels); 
avio_w8(pb, audio->codecpar->bits_per_coded_sample); 
avio_w8(pb, audio_codec); 
avio_wb16(pb, audio->codecpar->sample_rate); 
} else {
avio_w8(pb, 0);
avio_w8(pb, 0);
avio_w8(pb, 0);
avio_wb16(pb, 0);
}
avio_wb32(pb, 0);
avio_wb16(pb, 0);
ffio_wfourcc(pb, "STAB");
avio_wb32(pb, 16 + (film->packet_count * 16));
avio_wb32(pb, av_q2d(av_inv_q(video->time_base)));
avio_wb32(pb, film->packet_count);
packet = film->start;
while (packet != NULL) {
film_write_packet_to_header(format_context, packet);
prev = packet;
packet = packet->next;
av_freep(&prev);
}
film->start = film->last = NULL;
return 0;
}
static void film_deinit(AVFormatContext *format_context)
{
FILMOutputContext *film = format_context->priv_data;
FILMPacket *packet = film->start;
while (packet != NULL) {
FILMPacket *next = packet->next;
av_free(packet);
packet = next;
}
film->start = film->last = NULL;
}
AVOutputFormat ff_segafilm_muxer = {
.name = "film_cpk",
.long_name = NULL_IF_CONFIG_SMALL("Sega FILM / CPK"),
.extensions = "cpk",
.priv_data_size = sizeof(FILMOutputContext),
.audio_codec = AV_CODEC_ID_PCM_S16BE_PLANAR,
.video_codec = AV_CODEC_ID_CINEPAK,
.init = film_init,
.write_trailer = film_write_header,
.write_packet = film_write_packet,
.deinit = film_deinit,
};
