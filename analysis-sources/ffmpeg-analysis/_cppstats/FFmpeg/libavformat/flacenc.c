#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavcodec/flac.h"
#include "avformat.h"
#include "avio_internal.h"
#include "flacenc.h"
#include "id3v2.h"
#include "internal.h"
#include "vorbiscomment.h"
#include "libavcodec/bytestream.h"
typedef struct FlacMuxerContext {
const AVClass *class;
int write_header;
int audio_stream_idx;
int waiting_pics;
AVPacketList *queue, *queue_end;
uint8_t *streaminfo;
unsigned attached_types;
} FlacMuxerContext;
static int flac_write_block_padding(AVIOContext *pb, unsigned int n_padding_bytes,
int last_block)
{
avio_w8(pb, last_block ? 0x81 : 0x01);
avio_wb24(pb, n_padding_bytes);
ffio_fill(pb, 0, n_padding_bytes);
return 0;
}
static int flac_write_block_comment(AVIOContext *pb, AVDictionary **m,
int last_block, int bitexact)
{
const char *vendor = bitexact ? "ffmpeg" : LIBAVFORMAT_IDENT;
int64_t len;
uint8_t *p, *p0;
ff_metadata_conv(m, ff_vorbiscomment_metadata_conv, NULL);
len = ff_vorbiscomment_length(*m, vendor, NULL, 0);
if (len >= ((1<<24) - 4))
return AVERROR(EINVAL);
p0 = av_malloc(len+4);
if (!p0)
return AVERROR(ENOMEM);
p = p0;
bytestream_put_byte(&p, last_block ? 0x84 : 0x04);
bytestream_put_be24(&p, len);
ff_vorbiscomment_write(&p, m, vendor, NULL, 0);
avio_write(pb, p0, len+4);
av_freep(&p0);
p = NULL;
return 0;
}
static int flac_write_picture(struct AVFormatContext *s, AVPacket *pkt)
{
FlacMuxerContext *c = s->priv_data;
AVIOContext *pb = s->pb;
const AVPixFmtDescriptor *pixdesc;
const CodecMime *mime = ff_id3v2_mime_tags;
AVDictionaryEntry *e;
const char *mimetype = NULL, *desc = "";
const AVStream *st = s->streams[pkt->stream_index];
int i, mimelen, desclen, type = 0, blocklen;
if (!pkt->data)
return 0;
while (mime->id != AV_CODEC_ID_NONE) {
if (mime->id == st->codecpar->codec_id) {
mimetype = mime->str;
break;
}
mime++;
}
if (!mimetype) {
av_log(s, AV_LOG_ERROR, "No mimetype is known for stream %d, cannot "
"write an attached picture.\n", st->index);
return AVERROR(EINVAL);
}
mimelen = strlen(mimetype);
e = av_dict_get(st->metadata, "comment", NULL, 0);
for (i = 0; e && i < FF_ARRAY_ELEMS(ff_id3v2_picture_types); i++) {
if (!av_strcasecmp(e->value, ff_id3v2_picture_types[i])) {
type = i;
break;
}
}
if ((c->attached_types & (1 << type)) & 0x6) {
av_log(s, AV_LOG_ERROR, "Duplicate attachment for type '%s'\n", ff_id3v2_picture_types[type]);
return AVERROR(EINVAL);
}
if (type == 1 && (st->codecpar->codec_id != AV_CODEC_ID_PNG ||
st->codecpar->width != 32 ||
st->codecpar->height != 32)) {
av_log(s, AV_LOG_ERROR, "File icon attachment must be a 32x32 PNG");
return AVERROR(EINVAL);
}
c->attached_types |= (1 << type);
if ((e = av_dict_get(st->metadata, "title", NULL, 0)))
desc = e->value;
desclen = strlen(desc);
blocklen = 4 + 4 + mimelen + 4 + desclen + 4 + 4 + 4 + 4 + 4 + pkt->size;
if (blocklen >= 1<<24) {
av_log(s, AV_LOG_ERROR, "Picture block too big %d >= %d\n", blocklen, 1<<24);
return AVERROR(EINVAL);
}
avio_w8(pb, 0x06);
avio_wb24(pb, blocklen);
avio_wb32(pb, type);
avio_wb32(pb, mimelen);
avio_write(pb, mimetype, mimelen);
avio_wb32(pb, desclen);
avio_write(pb, desc, desclen);
avio_wb32(pb, st->codecpar->width);
avio_wb32(pb, st->codecpar->height);
if ((pixdesc = av_pix_fmt_desc_get(st->codecpar->format)))
avio_wb32(pb, av_get_bits_per_pixel(pixdesc));
else
avio_wb32(pb, 0);
avio_wb32(pb, 0);
avio_wb32(pb, pkt->size);
avio_write(pb, pkt->data, pkt->size);
return 0;
}
static int flac_finish_header(struct AVFormatContext *s)
{
int i, ret, padding = s->metadata_header_padding;
if (padding < 0)
padding = 8192;
padding = av_clip_uintp2(padding, 24);
for (i = 0; i < s->nb_streams; i++) {
AVStream *st = s->streams[i];
AVPacket *pkt = st->priv_data;
if (!pkt)
continue;
ret = flac_write_picture(s, pkt);
av_packet_unref(pkt);
if (ret < 0 && (s->error_recognition & AV_EF_EXPLODE))
return ret;
}
ret = flac_write_block_comment(s->pb, &s->metadata, !padding,
s->flags & AVFMT_FLAG_BITEXACT);
if (ret)
return ret;
if (padding)
flac_write_block_padding(s->pb, padding, 1);
return 0;
}
static int flac_init(struct AVFormatContext *s)
{
AVCodecParameters *par;
FlacMuxerContext *c = s->priv_data;
int i;
c->audio_stream_idx = -1;
for (i = 0; i < s->nb_streams; i++) {
AVStream *st = s->streams[i];
if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
if (c->audio_stream_idx >= 0 || st->codecpar->codec_id != AV_CODEC_ID_FLAC) {
av_log(s, AV_LOG_ERROR, "Invalid audio stream. Exactly one FLAC "
"audio stream is required.\n");
return AVERROR(EINVAL);
}
par = s->streams[i]->codecpar;
c->audio_stream_idx = i;
} else if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
if (!(st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
av_log(s, AV_LOG_WARNING, "Video stream #%d is not an attached picture. Ignoring\n", i);
continue;
} else if (st->codecpar->codec_id == AV_CODEC_ID_GIF) {
av_log(s, AV_LOG_ERROR, "GIF image support is not implemented.\n");
return AVERROR_PATCHWELCOME;
} else if (!c->write_header) {
av_log(s, AV_LOG_ERROR, "Can't write attached pictures without a header.\n");
return AVERROR(EINVAL);
}
c->waiting_pics++;
} else {
av_log(s, AV_LOG_ERROR, "Only audio streams and pictures are allowed in FLAC.\n");
return AVERROR(EINVAL);
}
}
if (c->audio_stream_idx < 0) {
av_log(s, AV_LOG_ERROR, "No audio stream present.\n");
return AVERROR(EINVAL);
}
if (par->channel_layout &&
!(par->channel_layout & ~0x3ffffULL) &&
!ff_flac_is_native_layout(par->channel_layout)) {
AVDictionaryEntry *chmask = av_dict_get(s->metadata, "WAVEFORMATEXTENSIBLE_CHANNEL_MASK",
NULL, 0);
if (chmask) {
av_log(s, AV_LOG_WARNING, "A WAVEFORMATEXTENSIBLE_CHANNEL_MASK is "
"already present, this muxer will not overwrite it.\n");
} else {
uint8_t buf[32];
snprintf(buf, sizeof(buf), "0x%"PRIx64, par->channel_layout);
av_dict_set(&s->metadata, "WAVEFORMATEXTENSIBLE_CHANNEL_MASK", buf, 0);
}
}
return 0;
}
static int flac_write_header(struct AVFormatContext *s)
{
FlacMuxerContext *c = s->priv_data;
AVCodecParameters *par = s->streams[c->audio_stream_idx]->codecpar;
int ret;
if (!c->write_header)
return 0;
ret = ff_flac_write_header(s->pb, par->extradata,
par->extradata_size, 0);
if (ret < 0)
return ret;
if (!c->waiting_pics)
ret = flac_finish_header(s);
return ret;
}
static int flac_write_audio_packet(struct AVFormatContext *s, AVPacket *pkt)
{
FlacMuxerContext *c = s->priv_data;
uint8_t *streaminfo;
int streaminfo_size;
streaminfo = av_packet_get_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA,
&streaminfo_size);
if (streaminfo && streaminfo_size == FLAC_STREAMINFO_SIZE) {
av_freep(&c->streaminfo);
c->streaminfo = av_malloc(FLAC_STREAMINFO_SIZE);
if (!c->streaminfo)
return AVERROR(ENOMEM);
memcpy(c->streaminfo, streaminfo, FLAC_STREAMINFO_SIZE);
}
if (pkt->size)
avio_write(s->pb, pkt->data, pkt->size);
return 0;
}
static int flac_queue_flush(AVFormatContext *s)
{
FlacMuxerContext *c = s->priv_data;
AVPacket pkt;
int ret, write = 1;
ret = flac_finish_header(s);
if (ret < 0)
write = 0;
while (c->queue) {
ff_packet_list_get(&c->queue, &c->queue_end, &pkt);
if (write && (ret = flac_write_audio_packet(s, &pkt)) < 0)
write = 0;
av_packet_unref(&pkt);
}
return ret;
}
static int flac_write_trailer(struct AVFormatContext *s)
{
AVIOContext *pb = s->pb;
int64_t file_size;
FlacMuxerContext *c = s->priv_data;
uint8_t *streaminfo = c->streaminfo ? c->streaminfo :
s->streams[c->audio_stream_idx]->codecpar->extradata;
if (c->waiting_pics) {
av_log(s, AV_LOG_WARNING, "No packets were sent for some of the "
"attached pictures.\n");
flac_queue_flush(s);
}
if (!c->write_header || !streaminfo)
return 0;
if (pb->seekable & AVIO_SEEKABLE_NORMAL) {
file_size = avio_tell(pb);
avio_seek(pb, 8, SEEK_SET);
avio_write(pb, streaminfo, FLAC_STREAMINFO_SIZE);
avio_seek(pb, file_size, SEEK_SET);
} else {
av_log(s, AV_LOG_WARNING, "unable to rewrite FLAC header.\n");
}
return 0;
}
static void flac_deinit(struct AVFormatContext *s)
{
FlacMuxerContext *c = s->priv_data;
ff_packet_list_free(&c->queue, &c->queue_end);
av_freep(&c->streaminfo);
}
static int flac_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
FlacMuxerContext *c = s->priv_data;
int ret;
if (pkt->stream_index == c->audio_stream_idx) {
if (c->waiting_pics) {
ret = ff_packet_list_put(&c->queue, &c->queue_end, pkt, FF_PACKETLIST_FLAG_REF_PACKET);
if (ret < 0) {
av_log(s, AV_LOG_ERROR, "Out of memory in packet queue; skipping attached pictures\n");
c->waiting_pics = 0;
ret = flac_queue_flush(s);
if (ret < 0)
return ret;
return flac_write_audio_packet(s, pkt);
}
} else
return flac_write_audio_packet(s, pkt);
} else {
AVStream *st = s->streams[pkt->stream_index];
if (!c->waiting_pics ||
!(st->disposition & AV_DISPOSITION_ATTACHED_PIC))
return 0;
if (st->nb_frames == 1) {
av_log(s, AV_LOG_WARNING, "Got more than one picture in stream %d,"
" ignoring.\n", pkt->stream_index);
}
if (st->nb_frames >= 1)
return 0;
st->priv_data = av_packet_clone(pkt);
if (!st->priv_data)
av_log(s, AV_LOG_ERROR, "Out of memory queueing an attached picture; skipping\n");
c->waiting_pics--;
if (!c->waiting_pics &&
(ret = flac_queue_flush(s)) < 0)
return ret;
}
return 0;
}
static const AVOption flacenc_options[] = {
{ "write_header", "Write the file header", offsetof(FlacMuxerContext, write_header), AV_OPT_TYPE_BOOL, {.i64 = 1}, 0, 1, AV_OPT_FLAG_ENCODING_PARAM },
{ NULL },
};
static const AVClass flac_muxer_class = {
.class_name = "flac muxer",
.item_name = av_default_item_name,
.option = flacenc_options,
.version = LIBAVUTIL_VERSION_INT,
};
AVOutputFormat ff_flac_muxer = {
.name = "flac",
.long_name = NULL_IF_CONFIG_SMALL("raw FLAC"),
.priv_data_size = sizeof(FlacMuxerContext),
.mime_type = "audio/x-flac",
.extensions = "flac",
.audio_codec = AV_CODEC_ID_FLAC,
.video_codec = AV_CODEC_ID_PNG,
.init = flac_init,
.write_header = flac_write_header,
.write_packet = flac_write_packet,
.write_trailer = flac_write_trailer,
.deinit = flac_deinit,
.flags = AVFMT_NOTIMESTAMPS,
.priv_class = &flac_muxer_class,
};
