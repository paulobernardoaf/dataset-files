#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "internal.h"
#define FORM_TAG MKBETAG('F', 'O', 'R', 'M')
#define WVQA_TAG MKBETAG('W', 'V', 'Q', 'A')
#define VQHD_TAG MKBETAG('V', 'Q', 'H', 'D')
#define FINF_TAG MKBETAG('F', 'I', 'N', 'F')
#define SND0_TAG MKBETAG('S', 'N', 'D', '0')
#define SND1_TAG MKBETAG('S', 'N', 'D', '1')
#define SND2_TAG MKBETAG('S', 'N', 'D', '2')
#define VQFR_TAG MKBETAG('V', 'Q', 'F', 'R')
#define CINF_TAG MKBETAG('C', 'I', 'N', 'F')
#define CINH_TAG MKBETAG('C', 'I', 'N', 'H')
#define CIND_TAG MKBETAG('C', 'I', 'N', 'D')
#define PINF_TAG MKBETAG('P', 'I', 'N', 'F')
#define PINH_TAG MKBETAG('P', 'I', 'N', 'H')
#define PIND_TAG MKBETAG('P', 'I', 'N', 'D')
#define CMDS_TAG MKBETAG('C', 'M', 'D', 'S')
#define VQA_HEADER_SIZE 0x2A
#define VQA_PREAMBLE_SIZE 8
typedef struct WsVqaDemuxContext {
int version;
int bps;
int channels;
int sample_rate;
int audio_stream_index;
int video_stream_index;
} WsVqaDemuxContext;
static int wsvqa_probe(const AVProbeData *p)
{
if (p->buf_size < 12)
return 0;
if ((AV_RB32(&p->buf[0]) != FORM_TAG) ||
(AV_RB32(&p->buf[8]) != WVQA_TAG))
return 0;
return AVPROBE_SCORE_MAX;
}
static int wsvqa_read_header(AVFormatContext *s)
{
WsVqaDemuxContext *wsvqa = s->priv_data;
AVIOContext *pb = s->pb;
AVStream *st;
uint8_t *header;
uint8_t scratch[VQA_PREAMBLE_SIZE];
uint32_t chunk_tag;
uint32_t chunk_size;
int fps, ret;
st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
st->start_time = 0;
wsvqa->video_stream_index = st->index;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = AV_CODEC_ID_WS_VQA;
st->codecpar->codec_tag = 0; 
avio_seek(pb, 20, SEEK_SET);
if ((ret = ff_get_extradata(s, st->codecpar, pb, VQA_HEADER_SIZE)) < 0)
return ret;
header = st->codecpar->extradata;
st->codecpar->width = AV_RL16(&header[6]);
st->codecpar->height = AV_RL16(&header[8]);
fps = header[12];
st->nb_frames =
st->duration = AV_RL16(&header[4]);
if (fps < 1 || fps > 30) {
av_log(s, AV_LOG_ERROR, "invalid fps: %d\n", fps);
return AVERROR_INVALIDDATA;
}
avpriv_set_pts_info(st, 64, 1, fps);
wsvqa->version = AV_RL16(&header[ 0]);
wsvqa->sample_rate = AV_RL16(&header[24]);
wsvqa->channels = header[26];
wsvqa->bps = header[27];
wsvqa->audio_stream_index = -1;
s->ctx_flags |= AVFMTCTX_NOHEADER;
do {
if (avio_read(pb, scratch, VQA_PREAMBLE_SIZE) != VQA_PREAMBLE_SIZE)
return AVERROR(EIO);
chunk_tag = AV_RB32(&scratch[0]);
chunk_size = AV_RB32(&scratch[4]);
switch (chunk_tag) {
case CINF_TAG:
case CINH_TAG:
case CIND_TAG:
case PINF_TAG:
case PINH_TAG:
case PIND_TAG:
case FINF_TAG:
case CMDS_TAG:
break;
default:
av_log(s, AV_LOG_ERROR, " note: unknown chunk seen (%s)\n",
av_fourcc2str(chunk_tag));
break;
}
avio_skip(pb, chunk_size);
} while (chunk_tag != FINF_TAG);
return 0;
}
static int wsvqa_read_packet(AVFormatContext *s,
AVPacket *pkt)
{
WsVqaDemuxContext *wsvqa = s->priv_data;
AVIOContext *pb = s->pb;
int ret = -1;
uint8_t preamble[VQA_PREAMBLE_SIZE];
uint32_t chunk_type;
uint32_t chunk_size;
int skip_byte;
while (avio_read(pb, preamble, VQA_PREAMBLE_SIZE) == VQA_PREAMBLE_SIZE) {
chunk_type = AV_RB32(&preamble[0]);
chunk_size = AV_RB32(&preamble[4]);
skip_byte = chunk_size & 0x01;
if ((chunk_type == SND0_TAG) || (chunk_type == SND1_TAG) ||
(chunk_type == SND2_TAG) || (chunk_type == VQFR_TAG)) {
ret= av_get_packet(pb, pkt, chunk_size);
if (ret<0)
return AVERROR(EIO);
switch (chunk_type) {
case SND0_TAG:
case SND1_TAG:
case SND2_TAG:
if (wsvqa->audio_stream_index == -1) {
AVStream *st = avformat_new_stream(s, NULL);
if (!st)
return AVERROR(ENOMEM);
wsvqa->audio_stream_index = st->index;
if (!wsvqa->sample_rate)
wsvqa->sample_rate = 22050;
if (!wsvqa->channels)
wsvqa->channels = 1;
if (!wsvqa->bps)
wsvqa->bps = 8;
st->codecpar->sample_rate = wsvqa->sample_rate;
st->codecpar->bits_per_coded_sample = wsvqa->bps;
st->codecpar->channels = wsvqa->channels;
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
switch (chunk_type) {
case SND0_TAG:
if (wsvqa->bps == 16)
st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
else
st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
break;
case SND1_TAG:
st->codecpar->codec_id = AV_CODEC_ID_WESTWOOD_SND1;
break;
case SND2_TAG:
st->codecpar->codec_id = AV_CODEC_ID_ADPCM_IMA_WS;
if ((ret = ff_alloc_extradata(st->codecpar, 2)) < 0)
return ret;
AV_WL16(st->codecpar->extradata, wsvqa->version);
break;
}
}
pkt->stream_index = wsvqa->audio_stream_index;
switch (chunk_type) {
case SND1_TAG:
if(pkt->data)
pkt->duration = AV_RL16(pkt->data) / wsvqa->channels;
break;
case SND2_TAG:
pkt->duration = (chunk_size * 2) / wsvqa->channels;
break;
}
break;
case VQFR_TAG:
pkt->stream_index = wsvqa->video_stream_index;
pkt->duration = 1;
break;
}
if (skip_byte)
avio_skip(pb, 1);
return ret;
} else {
switch(chunk_type){
case CMDS_TAG:
break;
default:
av_log(s, AV_LOG_INFO, "Skipping unknown chunk %s\n",
av_fourcc2str(av_bswap32(chunk_type)));
}
avio_skip(pb, chunk_size + skip_byte);
}
}
return ret;
}
AVInputFormat ff_wsvqa_demuxer = {
.name = "wsvqa",
.long_name = NULL_IF_CONFIG_SMALL("Westwood Studios VQA"),
.priv_data_size = sizeof(WsVqaDemuxContext),
.read_probe = wsvqa_probe,
.read_header = wsvqa_read_header,
.read_packet = wsvqa_read_packet,
};
