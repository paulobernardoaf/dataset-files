



























#include "libavutil/channel_layout.h"
#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"

enum {
TMV_PADDING = 0x01,
TMV_STEREO = 0x02,
};

#define TMV_TAG MKTAG('T', 'M', 'A', 'V')

typedef struct TMVContext {
unsigned audio_chunk_size;
unsigned video_chunk_size;
unsigned padding;
unsigned stream_index;
} TMVContext;

#define TMV_HEADER_SIZE 12

#define PROBE_MIN_SAMPLE_RATE 5000
#define PROBE_MAX_FPS 120
#define PROBE_MIN_AUDIO_SIZE (PROBE_MIN_SAMPLE_RATE / PROBE_MAX_FPS)

static int tmv_probe(const AVProbeData *p)
{
if (AV_RL32(p->buf) == TMV_TAG &&
AV_RL16(p->buf+4) >= PROBE_MIN_SAMPLE_RATE &&
AV_RL16(p->buf+6) >= PROBE_MIN_AUDIO_SIZE &&
!p->buf[8] && 
p->buf[9] && 
p->buf[10]) 
return AVPROBE_SCORE_MAX /
((p->buf[9] == 40 && p->buf[10] == 25) ? 1 : 4);
return 0;
}

static int tmv_read_header(AVFormatContext *s)
{
TMVContext *tmv = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *vst, *ast;
AVRational fps;
unsigned comp_method, char_cols, char_rows, features;

if (avio_rl32(pb) != TMV_TAG)
return -1;

if (!(vst = avformat_new_stream(s, NULL)))
return AVERROR(ENOMEM);

if (!(ast = avformat_new_stream(s, NULL)))
return AVERROR(ENOMEM);

ast->codecpar->sample_rate = avio_rl16(pb);
if (!ast->codecpar->sample_rate) {
av_log(s, AV_LOG_ERROR, "invalid sample rate\n");
return -1;
}

tmv->audio_chunk_size = avio_rl16(pb);
if (!tmv->audio_chunk_size) {
av_log(s, AV_LOG_ERROR, "invalid audio chunk size\n");
return -1;
}

comp_method = avio_r8(pb);
if (comp_method) {
av_log(s, AV_LOG_ERROR, "unsupported compression method %d\n",
comp_method);
return -1;
}

char_cols = avio_r8(pb);
char_rows = avio_r8(pb);
tmv->video_chunk_size = char_cols * char_rows * 2;

features = avio_r8(pb);
if (features & ~(TMV_PADDING | TMV_STEREO)) {
av_log(s, AV_LOG_ERROR, "unsupported features 0x%02x\n",
features & ~(TMV_PADDING | TMV_STEREO));
return -1;
}

ast->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
ast->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
if (features & TMV_STEREO) {
ast->codecpar->channels = 2;
ast->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
} else {
ast->codecpar->channels = 1;
ast->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
}
ast->codecpar->bits_per_coded_sample = 8;
ast->codecpar->bit_rate = ast->codecpar->sample_rate *
ast->codecpar->bits_per_coded_sample;
avpriv_set_pts_info(ast, 32, 1, ast->codecpar->sample_rate);

fps.num = ast->codecpar->sample_rate * ast->codecpar->channels;
fps.den = tmv->audio_chunk_size;
av_reduce(&fps.num, &fps.den, fps.num, fps.den, 0xFFFFFFFFLL);

vst->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
vst->codecpar->codec_id = AV_CODEC_ID_TMV;
vst->codecpar->format = AV_PIX_FMT_PAL8;
vst->codecpar->width = char_cols * 8;
vst->codecpar->height = char_rows * 8;
avpriv_set_pts_info(vst, 32, fps.den, fps.num);

if (features & TMV_PADDING)
tmv->padding =
((tmv->video_chunk_size + tmv->audio_chunk_size + 511) & ~511) -
(tmv->video_chunk_size + tmv->audio_chunk_size);

vst->codecpar->bit_rate = ((tmv->video_chunk_size + tmv->padding) *
fps.num * 8) / fps.den;

return 0;
}

static int tmv_read_packet(AVFormatContext *s, AVPacket *pkt)
{
TMVContext *tmv = s->priv_data;
AVIOContext *pb = s->pb;
int ret, pkt_size = tmv->stream_index ?
tmv->audio_chunk_size : tmv->video_chunk_size;

if (avio_feof(pb))
return AVERROR_EOF;

ret = av_get_packet(pb, pkt, pkt_size);

if (tmv->stream_index)
avio_skip(pb, tmv->padding);

pkt->stream_index = tmv->stream_index;
tmv->stream_index ^= 1;
pkt->flags |= AV_PKT_FLAG_KEY;

return ret;
}

static int tmv_read_seek(AVFormatContext *s, int stream_index,
int64_t timestamp, int flags)
{
TMVContext *tmv = s->priv_data;
int64_t pos;

if (stream_index)
return -1;

pos = timestamp *
(tmv->audio_chunk_size + tmv->video_chunk_size + tmv->padding);

if (avio_seek(s->pb, pos + TMV_HEADER_SIZE, SEEK_SET) < 0)
return -1;
tmv->stream_index = 0;
return 0;
}

AVInputFormat ff_tmv_demuxer = {
.name = "tmv",
.long_name = NULL_IF_CONFIG_SMALL("8088flex TMV"),
.priv_data_size = sizeof(TMVContext),
.read_probe = tmv_probe,
.read_header = tmv_read_header,
.read_packet = tmv_read_packet,
.read_seek = tmv_read_seek,
.flags = AVFMT_GENERIC_INDEX,
};
