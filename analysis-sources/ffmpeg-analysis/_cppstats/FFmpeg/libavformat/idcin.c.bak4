





































































#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

#define HUFFMAN_TABLE_SIZE (64 * 1024)
#define IDCIN_FPS 14

typedef struct IdcinDemuxContext {
int video_stream_index;
int audio_stream_index;
int audio_chunk_size1;
int audio_chunk_size2;
int block_align;


int current_audio_chunk;
int next_chunk_is_video;
int audio_present;
int64_t first_pkt_pos;
} IdcinDemuxContext;

static int idcin_probe(const AVProbeData *p)
{
unsigned int number, sample_rate;
unsigned int w, h;
int i;














if (p->buf_size < 20 + HUFFMAN_TABLE_SIZE + 12)
return 0;


w = AV_RL32(&p->buf[0]);
if ((w == 0) || (w > 1024))
return 0;


h = AV_RL32(&p->buf[4]);
if ((h == 0) || (h > 1024))
return 0;


sample_rate = AV_RL32(&p->buf[8]);
if (sample_rate && (sample_rate < 8000 || sample_rate > 48000))
return 0;


number = AV_RL32(&p->buf[12]);
if (number > 2 || sample_rate && !number)
return 0;


number = AV_RL32(&p->buf[16]);
if (number > 2 || sample_rate && !number)
return 0;

i = 20 + HUFFMAN_TABLE_SIZE;
if (AV_RL32(&p->buf[i]) == 1)
i += 768;

if (i+12 > p->buf_size || AV_RL32(&p->buf[i+8]) != w*h)
return 1;


return AVPROBE_SCORE_EXTENSION;
}

static int idcin_read_header(AVFormatContext *s)
{
AVIOContext *pb = s->pb;
IdcinDemuxContext *idcin = s->priv_data;
AVStream *st;
unsigned int width, height;
unsigned int sample_rate, bytes_per_sample, channels;
int ret;


width = avio_rl32(pb);
height = avio_rl32(pb);
sample_rate = avio_rl32(pb);
bytes_per_sample = avio_rl32(pb);
channels = avio_rl32(pb);

if (s->pb->eof_reached) {
av_log(s, AV_LOG_ERROR, "incomplete header\n");
return s->pb->error ? s->pb->error : AVERROR_EOF;
}

if (av_image_check_size(width, height, 0, s) < 0)
return AVERROR_INVALIDDATA;
if (sample_rate > 0) {
if (sample_rate < 14 || sample_rate > INT_MAX) {
av_log(s, AV_LOG_ERROR, "invalid sample rate: %u\n", sample_rate);
return AVERROR_INVALIDDATA;
}
if (bytes_per_sample < 1 || bytes_per_sample > 2) {
av_log(s, AV_LOG_ERROR, "invalid bytes per sample: %u\n",
bytes_per_sample);
return AVERROR_INVALIDDATA;
}
if (channels < 1 || channels > 2) {
av_log(s, AV_LOG_ERROR, "invalid channels: %u\n", channels);
return AVERROR_INVALIDDATA;
}
idcin->audio_present = 1;
} else {

idcin->audio_present = 0;
}

st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avpriv_set_pts_info(st, 33, 1, IDCIN_FPS);
st->start_time = 0;
idcin->video_stream_index = st->index;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_IDCIN;
st->codecpar->codec_tag = 0; 
st->codecpar->width = width;
st->codecpar->height = height;


if ((ret = ff_get_extradata(s, st->codecpar, pb, HUFFMAN_TABLE_SIZE)) < 0)
return ret;

if (idcin->audio_present) {
idcin->audio_present = 1;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
avpriv_set_pts_info(st, 63, 1, sample_rate);
st->start_time = 0;
idcin->audio_stream_index = st->index;
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_tag = 1;
st->codecpar->channels = channels;
st->codecpar->channel_layout = channels > 1 ? AV_CH_LAYOUT_STEREO :
AV_CH_LAYOUT_MONO;
st->codecpar->sample_rate = sample_rate;
st->codecpar->bits_per_coded_sample = bytes_per_sample * 8;
st->codecpar->bit_rate = sample_rate * bytes_per_sample * 8 * channels;
st->codecpar->block_align = idcin->block_align = bytes_per_sample * channels;
if (bytes_per_sample == 1)
st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
else
st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;

if (sample_rate % 14 != 0) {
idcin->audio_chunk_size1 = (sample_rate / 14) *
bytes_per_sample * channels;
idcin->audio_chunk_size2 = (sample_rate / 14 + 1) *
bytes_per_sample * channels;
} else {
idcin->audio_chunk_size1 = idcin->audio_chunk_size2 =
(sample_rate / 14) * bytes_per_sample * channels;
}
idcin->current_audio_chunk = 0;
}

idcin->next_chunk_is_video = 1;
idcin->first_pkt_pos = avio_tell(s->pb);

return 0;
}

static int idcin_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
int ret;
unsigned int command;
unsigned int chunk_size;
IdcinDemuxContext *idcin = s->priv_data;
AVIOContext *pb = s->pb;
int i;
int palette_scale;
unsigned char r, g, b;
unsigned char palette_buffer[768];
uint32_t palette[256];

if (avio_feof(s->pb))
return s->pb->error ? s->pb->error : AVERROR_EOF;

if (idcin->next_chunk_is_video) {
command = avio_rl32(pb);
if (command == 2) {
return AVERROR(EIO);
} else if (command == 1) {

ret = avio_read(pb, palette_buffer, 768);
if (ret < 0) {
return ret;
} else if (ret != 768) {
av_log(s, AV_LOG_ERROR, "incomplete packet\n");
return AVERROR(EIO);
}

palette_scale = 2;
for (i = 0; i < 768; i++)
if (palette_buffer[i] > 63) {
palette_scale = 0;
break;
}

for (i = 0; i < 256; i++) {
r = palette_buffer[i * 3 ] << palette_scale;
g = palette_buffer[i * 3 + 1] << palette_scale;
b = palette_buffer[i * 3 + 2] << palette_scale;
palette[i] = (0xFFU << 24) | (r << 16) | (g << 8) | (b);
if (palette_scale == 2)
palette[i] |= palette[i] >> 6 & 0x30303;
}
}

if (s->pb->eof_reached) {
av_log(s, AV_LOG_ERROR, "incomplete packet\n");
return s->pb->error ? s->pb->error : AVERROR_EOF;
}
chunk_size = avio_rl32(pb);
if (chunk_size < 4 || chunk_size > INT_MAX - 4) {
av_log(s, AV_LOG_ERROR, "invalid chunk size: %u\n", chunk_size);
return AVERROR_INVALIDDATA;
}

avio_skip(pb, 4);
chunk_size -= 4;
ret= av_get_packet(pb, pkt, chunk_size);
if (ret < 0)
return ret;
else if (ret != chunk_size) {
av_log(s, AV_LOG_ERROR, "incomplete packet\n");
return AVERROR(EIO);
}
if (command == 1) {
uint8_t *pal;

pal = av_packet_new_side_data(pkt, AV_PKT_DATA_PALETTE,
AVPALETTE_SIZE);
if (!pal) {
return AVERROR(ENOMEM);
}
memcpy(pal, palette, AVPALETTE_SIZE);
pkt->flags |= AV_PKT_FLAG_KEY;
}
pkt->stream_index = idcin->video_stream_index;
pkt->duration = 1;
} else {

if (idcin->current_audio_chunk)
chunk_size = idcin->audio_chunk_size2;
else
chunk_size = idcin->audio_chunk_size1;
ret= av_get_packet(pb, pkt, chunk_size);
if (ret < 0)
return ret;
pkt->stream_index = idcin->audio_stream_index;
pkt->duration = chunk_size / idcin->block_align;

idcin->current_audio_chunk ^= 1;
}

if (idcin->audio_present)
idcin->next_chunk_is_video ^= 1;

return 0;
}

static int idcin_read_seek(AVFormatContext *s, int stream_index,
int64_t timestamp, int flags)
{
IdcinDemuxContext *idcin = s->priv_data;

if (idcin->first_pkt_pos > 0) {
int64_t ret = avio_seek(s->pb, idcin->first_pkt_pos, SEEK_SET);
if (ret < 0)
return ret;
ff_update_cur_dts(s, s->streams[idcin->video_stream_index], 0);
idcin->next_chunk_is_video = 1;
idcin->current_audio_chunk = 0;
return 0;
}
return -1;
}

AVInputFormat ff_idcin_demuxer = {
.name = "idcin",
.long_name = NULL_IF_CONFIG_SMALL("id Cinematic"),
.priv_data_size = sizeof(IdcinDemuxContext),
.read_probe = idcin_probe,
.read_header = idcin_read_header,
.read_packet = idcin_read_packet,
.read_seek = idcin_read_seek,
.flags = AVFMT_NO_BYTE_SEEK,
};
