#include <memory.h>
#include "libavcodec/codec2utils.h"
#include "libavutil/intreadwrite.h"
#include "avio_internal.h"
#include "avformat.h"
#include "internal.h"
#include "rawdec.h"
#include "rawenc.h"
#include "pcm.h"
#define AVPRIV_CODEC2_HEADER_SIZE 7
#define AVPRIV_CODEC2_MAGIC 0xC0DEC2
#define EXPECTED_CODEC2_MAJOR_VERSION 0
#define EXPECTED_CODEC2_MINOR_VERSION 8
typedef struct {
const AVClass *class;
int mode;
int frames_per_packet;
} Codec2Context;
static int codec2_probe(const AVProbeData *p)
{
if (AV_RB24(p->buf) != AVPRIV_CODEC2_MAGIC) {
return 0;
}
if (p->buf[3] != EXPECTED_CODEC2_MAJOR_VERSION ||
p->buf[4] < EXPECTED_CODEC2_MINOR_VERSION) {
return 0;
}
return AVPROBE_SCORE_EXTENSION + 1;
}
static int codec2_read_header_common(AVFormatContext *s, AVStream *st)
{
int mode = avpriv_codec2_mode_from_extradata(st->codecpar->extradata);
st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
st->codecpar->codec_id = AV_CODEC_ID_CODEC2;
st->codecpar->sample_rate = 8000;
st->codecpar->channels = 1;
st->codecpar->format = AV_SAMPLE_FMT_S16;
st->codecpar->channel_layout = AV_CH_LAYOUT_MONO;
st->codecpar->bit_rate = avpriv_codec2_mode_bit_rate(s, mode);
st->codecpar->frame_size = avpriv_codec2_mode_frame_size(s, mode);
st->codecpar->block_align = avpriv_codec2_mode_block_align(s, mode);
if (st->codecpar->bit_rate <= 0 ||
st->codecpar->frame_size <= 0 ||
st->codecpar->block_align <= 0) {
return AVERROR_INVALIDDATA;
}
avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
return 0;
}
static int codec2_read_header(AVFormatContext *s)
{
AVStream *st = avformat_new_stream(s, NULL);
int ret, version;
if (!st) {
return AVERROR(ENOMEM);
}
if (avio_rb24(s->pb) != AVPRIV_CODEC2_MAGIC) {
av_log(s, AV_LOG_ERROR, "not a .c2 file\n");
return AVERROR_INVALIDDATA;
}
ret = ff_alloc_extradata(st->codecpar, AVPRIV_CODEC2_EXTRADATA_SIZE);
if (ret) {
return ret;
}
ret = ffio_read_size(s->pb, st->codecpar->extradata, AVPRIV_CODEC2_EXTRADATA_SIZE);
if (ret < 0) {
return ret;
}
version = avpriv_codec2_version_from_extradata(st->codecpar->extradata);
if ((version >> 8) != EXPECTED_CODEC2_MAJOR_VERSION) {
avpriv_report_missing_feature(s, "Major version %i", version >> 8);
return AVERROR_PATCHWELCOME;
}
s->internal->data_offset = AVPRIV_CODEC2_HEADER_SIZE;
return codec2_read_header_common(s, st);
}
static int codec2_read_packet(AVFormatContext *s, AVPacket *pkt)
{
Codec2Context *c2 = s->priv_data;
AVStream *st = s->streams[0];
int ret, size, n, block_align, frame_size;
block_align = st->codecpar->block_align;
frame_size = st->codecpar->frame_size;
if (block_align <= 0 || frame_size <= 0 || c2->frames_per_packet <= 0) {
return AVERROR(EINVAL);
}
size = c2->frames_per_packet * block_align;
ret = av_get_packet(s->pb, pkt, size);
if (ret < 0) {
return ret;
}
n = ret / block_align;
pkt->duration = n * frame_size;
return ret;
}
static int codec2_write_header(AVFormatContext *s)
{
AVStream *st;
if (s->nb_streams != 1 || s->streams[0]->codecpar->codec_id != AV_CODEC_ID_CODEC2) {
av_log(s, AV_LOG_ERROR, ".c2 files must have exactly one codec2 stream\n");
return AVERROR(EINVAL);
}
st = s->streams[0];
if (st->codecpar->extradata_size != AVPRIV_CODEC2_EXTRADATA_SIZE) {
av_log(s, AV_LOG_ERROR, ".c2 files require exactly %i bytes of extradata (got %i)\n",
AVPRIV_CODEC2_EXTRADATA_SIZE, st->codecpar->extradata_size);
return AVERROR(EINVAL);
}
avio_wb24(s->pb, AVPRIV_CODEC2_MAGIC);
avio_write(s->pb, st->codecpar->extradata, AVPRIV_CODEC2_EXTRADATA_SIZE);
return 0;
}
static int codec2raw_read_header(AVFormatContext *s)
{
Codec2Context *c2 = s->priv_data;
AVStream *st;
int ret;
if (c2->mode < 0) {
av_log(s, AV_LOG_ERROR, "-mode must be set in order to make sense of raw codec2 files\n");
return AVERROR(EINVAL);
}
st = avformat_new_stream(s, NULL);
if (!st) {
return AVERROR(ENOMEM);
}
ret = ff_alloc_extradata(st->codecpar, AVPRIV_CODEC2_EXTRADATA_SIZE);
if (ret) {
return ret;
}
s->internal->data_offset = 0;
avpriv_codec2_make_extradata(st->codecpar->extradata, c2->mode);
return codec2_read_header_common(s, st);
}
#define FRAMES_PER_PACKET { "frames_per_packet", "Number of frames to read at a time. Higher = faster decoding, lower granularity", offsetof(Codec2Context, frames_per_packet), AV_OPT_TYPE_INT, {.i64 = 1}, 1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM}
static const AVOption codec2_options[] = {
FRAMES_PER_PACKET,
{ NULL },
};
static const AVOption codec2raw_options[] = {
AVPRIV_CODEC2_AVOPTIONS("codec2 mode [mandatory]", Codec2Context, -1, -1, AV_OPT_FLAG_DECODING_PARAM),
FRAMES_PER_PACKET,
{ NULL },
};
static const AVClass codec2_mux_class = {
.class_name = "codec2 muxer",
.item_name = av_default_item_name,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEMUXER,
};
static const AVClass codec2_demux_class = {
.class_name = "codec2 demuxer",
.item_name = av_default_item_name,
.option = codec2_options,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEMUXER,
};
static const AVClass codec2raw_demux_class = {
.class_name = "codec2raw demuxer",
.item_name = av_default_item_name,
.option = codec2raw_options,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEMUXER,
};
#if CONFIG_CODEC2_DEMUXER
AVInputFormat ff_codec2_demuxer = {
.name = "codec2",
.long_name = NULL_IF_CONFIG_SMALL("codec2 .c2 demuxer"),
.priv_data_size = sizeof(Codec2Context),
.extensions = "c2",
.read_probe = codec2_probe,
.read_header = codec2_read_header,
.read_packet = codec2_read_packet,
.read_seek = ff_pcm_read_seek,
.flags = AVFMT_GENERIC_INDEX,
.raw_codec_id = AV_CODEC_ID_CODEC2,
.priv_class = &codec2_demux_class,
};
#endif
#if CONFIG_CODEC2_MUXER
AVOutputFormat ff_codec2_muxer = {
.name = "codec2",
.long_name = NULL_IF_CONFIG_SMALL("codec2 .c2 muxer"),
.priv_data_size = sizeof(Codec2Context),
.extensions = "c2",
.audio_codec = AV_CODEC_ID_CODEC2,
.video_codec = AV_CODEC_ID_NONE,
.write_header = codec2_write_header,
.write_packet = ff_raw_write_packet,
.flags = AVFMT_NOTIMESTAMPS,
.priv_class = &codec2_mux_class,
};
#endif
#if CONFIG_CODEC2RAW_DEMUXER
AVInputFormat ff_codec2raw_demuxer = {
.name = "codec2raw",
.long_name = NULL_IF_CONFIG_SMALL("raw codec2 demuxer"),
.priv_data_size = sizeof(Codec2Context),
.read_header = codec2raw_read_header,
.read_packet = codec2_read_packet,
.read_seek = ff_pcm_read_seek,
.flags = AVFMT_GENERIC_INDEX,
.raw_codec_id = AV_CODEC_ID_CODEC2,
.priv_class = &codec2raw_demux_class,
};
#endif
