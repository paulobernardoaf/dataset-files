#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#define FLIC_FILE_MAGIC_1 0xAF11
#define FLIC_FILE_MAGIC_2 0xAF12
#define FLIC_FILE_MAGIC_3 0xAF44 
#define FLIC_CHUNK_MAGIC_1 0xF1FA
#define FLIC_CHUNK_MAGIC_2 0xF5FA
#define FLIC_MC_SPEED 5 
#define FLIC_DEFAULT_SPEED 5 
#define FLIC_TFTD_CHUNK_AUDIO 0xAAAA 
#define FLIC_TFTD_SAMPLE_RATE 22050
#define FLIC_HEADER_SIZE 128
#define FLIC_PREAMBLE_SIZE 6
typedef struct FlicDemuxContext {
int video_stream_index;
int audio_stream_index;
int frame_number;
} FlicDemuxContext;
static int flic_probe(const AVProbeData *p)
{
int magic_number;
if(p->buf_size < FLIC_HEADER_SIZE)
return 0;
magic_number = AV_RL16(&p->buf[4]);
if ((magic_number != FLIC_FILE_MAGIC_1) &&
(magic_number != FLIC_FILE_MAGIC_2) &&
(magic_number != FLIC_FILE_MAGIC_3))
return 0;
if(AV_RL16(&p->buf[0x10]) != FLIC_CHUNK_MAGIC_1){
if(AV_RL32(&p->buf[0x10]) > 2000)
return 0;
}
if( AV_RL16(&p->buf[0x08]) > 4096
|| AV_RL16(&p->buf[0x0A]) > 4096)
return 0;
return AVPROBE_SCORE_MAX - 1;
}
static int flic_read_header(AVFormatContext *s)
{
FlicDemuxContext *flic = s->priv_data;
AVIOContext *pb = s->pb;
unsigned char header[FLIC_HEADER_SIZE];
AVStream *st, *ast;
int speed, ret;
int magic_number;
unsigned char preamble[FLIC_PREAMBLE_SIZE];
flic->frame_number = 0;
if (avio_read(pb, header, FLIC_HEADER_SIZE) != FLIC_HEADER_SIZE)
return AVERROR(EIO);
magic_number = AV_RL16(&header[4]);
speed = AV_RL32(&header[0x10]);
if (speed == 0)
speed = FLIC_DEFAULT_SPEED;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
flic->video_stream_index = st->index;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_FLIC;
st->codecpar->codec_tag = 0; 
st->codecpar->width = AV_RL16(&header[0x08]);
st->codecpar->height = AV_RL16(&header[0x0A]);
if (!st->codecpar->width || !st->codecpar->height) {
av_log(s, AV_LOG_WARNING,
"File with no specified width/height. Trying 640x480.\n");
st->codecpar->width = 640;
st->codecpar->height = 480;
}
if ((ret = ff_alloc_extradata(st->codecpar, FLIC_HEADER_SIZE)) < 0)
return ret;
memcpy(st->codecpar->extradata, header, FLIC_HEADER_SIZE);
if (avio_read(pb, preamble, FLIC_PREAMBLE_SIZE) != FLIC_PREAMBLE_SIZE) {
av_log(s, AV_LOG_ERROR, "Failed to peek at preamble\n");
return AVERROR(EIO);
}
avio_seek(pb, -FLIC_PREAMBLE_SIZE, SEEK_CUR);
if (AV_RL16(&preamble[4]) == FLIC_TFTD_CHUNK_AUDIO) {
ast = avformat_new_stream(s, NULL);
if (!ast)
return AVERROR(ENOMEM);
flic->audio_stream_index = ast->index;
ast->codecpar->block_align = AV_RL32(&preamble[0]);
ast->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
ast->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
ast->codecpar->codec_tag = 0;
ast->codecpar->sample_rate = FLIC_TFTD_SAMPLE_RATE;
ast->codecpar->channels = 1;
ast->codecpar->bit_rate = st->codecpar->sample_rate * 8;
ast->codecpar->bits_per_coded_sample = 8;
ast->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
ast->codecpar->extradata_size = 0;
avpriv_set_pts_info(st, 64, ast->codecpar->block_align, FLIC_TFTD_SAMPLE_RATE);
avpriv_set_pts_info(ast, 64, 1, FLIC_TFTD_SAMPLE_RATE);
} else if (AV_RL16(&header[0x10]) == FLIC_CHUNK_MAGIC_1) {
avpriv_set_pts_info(st, 64, FLIC_MC_SPEED, 70);
avio_seek(pb, 12, SEEK_SET);
if ((ret = ff_alloc_extradata(st->codecpar, 12)) < 0)
return ret;
memcpy(st->codecpar->extradata, header, 12);
} else if (magic_number == FLIC_FILE_MAGIC_1) {
avpriv_set_pts_info(st, 64, speed, 70);
} else if ((magic_number == FLIC_FILE_MAGIC_2) ||
(magic_number == FLIC_FILE_MAGIC_3)) {
avpriv_set_pts_info(st, 64, speed, 1000);
} else {
av_log(s, AV_LOG_ERROR, "Invalid or unsupported magic chunk in file\n");
return AVERROR_INVALIDDATA;
}
return 0;
}
static int flic_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
FlicDemuxContext *flic = s->priv_data;
AVIOContext *pb = s->pb;
int packet_read = 0;
unsigned int size;
int magic;
int ret = 0;
unsigned char preamble[FLIC_PREAMBLE_SIZE];
while (!packet_read && !avio_feof(pb)) {
if ((ret = avio_read(pb, preamble, FLIC_PREAMBLE_SIZE)) !=
FLIC_PREAMBLE_SIZE) {
ret = AVERROR(EIO);
break;
}
size = AV_RL32(&preamble[0]);
magic = AV_RL16(&preamble[4]);
if (((magic == FLIC_CHUNK_MAGIC_1) || (magic == FLIC_CHUNK_MAGIC_2)) && size > FLIC_PREAMBLE_SIZE) {
if ((ret = av_new_packet(pkt, size)) < 0)
return ret;
pkt->stream_index = flic->video_stream_index;
pkt->pts = flic->frame_number++;
pkt->pos = avio_tell(pb);
memcpy(pkt->data, preamble, FLIC_PREAMBLE_SIZE);
ret = avio_read(pb, pkt->data + FLIC_PREAMBLE_SIZE,
size - FLIC_PREAMBLE_SIZE);
if (ret != size - FLIC_PREAMBLE_SIZE) {
ret = AVERROR(EIO);
}
packet_read = 1;
} else if (magic == FLIC_TFTD_CHUNK_AUDIO) {
if ((ret = av_new_packet(pkt, size)) < 0)
return ret;
avio_skip(pb, 10);
pkt->stream_index = flic->audio_stream_index;
pkt->pos = avio_tell(pb);
ret = avio_read(pb, pkt->data, size);
if (ret != size) {
ret = AVERROR(EIO);
break;
}
packet_read = 1;
} else {
avio_skip(pb, size - 6);
}
}
return avio_feof(pb) ? AVERROR_EOF : ret;
}
AVInputFormat ff_flic_demuxer = {
.name = "flic",
.long_name = NULL_IF_CONFIG_SMALL("FLI/FLC/FLX animation"),
.priv_data_size = sizeof(FlicDemuxContext),
.read_probe = flic_probe,
.read_header = flic_read_header,
.read_packet = flic_read_packet,
};
