




















#include "libavutil/intreadwrite.h"
#include "libavcodec/bytestream.h"
#include "libavcodec/png.h"
#include "avformat.h"
#include "flac_picture.h"
#include "id3v2.h"
#include "internal.h"

int ff_flac_parse_picture(AVFormatContext *s, uint8_t *buf, int buf_size)
{
const CodecMime *mime = ff_id3v2_mime_tags;
enum AVCodecID id = AV_CODEC_ID_NONE;
AVBufferRef *data = NULL;
uint8_t mimetype[64], *desc = NULL;
GetByteContext g;
AVStream *st;
int width, height, ret = 0;
unsigned int len, type;

if (buf_size < 34) {
av_log(s, AV_LOG_ERROR, "Attached picture metadata block too short\n");
if (s->error_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
return 0;
}

bytestream2_init(&g, buf, buf_size);


type = bytestream2_get_be32u(&g);
if (type >= FF_ARRAY_ELEMS(ff_id3v2_picture_types)) {
av_log(s, AV_LOG_ERROR, "Invalid picture type: %d.\n", type);
if (s->error_recognition & AV_EF_EXPLODE) {
return AVERROR_INVALIDDATA;
}
type = 0;
}


len = bytestream2_get_be32u(&g);
if (len <= 0 || len >= sizeof(mimetype)) {
av_log(s, AV_LOG_ERROR, "Could not read mimetype from an attached "
"picture.\n");
if (s->error_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
return 0;
}
if (len + 24 > bytestream2_get_bytes_left(&g)) {
av_log(s, AV_LOG_ERROR, "Attached picture metadata block too short\n");
if (s->error_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
return 0;
}
bytestream2_get_bufferu(&g, mimetype, len);
mimetype[len] = 0;

while (mime->id != AV_CODEC_ID_NONE) {
if (!strncmp(mime->str, mimetype, sizeof(mimetype))) {
id = mime->id;
break;
}
mime++;
}
if (id == AV_CODEC_ID_NONE) {
av_log(s, AV_LOG_ERROR, "Unknown attached picture mimetype: %s.\n",
mimetype);
if (s->error_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
return 0;
}


len = bytestream2_get_be32u(&g);
if (len > bytestream2_get_bytes_left(&g) - 20) {
av_log(s, AV_LOG_ERROR, "Attached picture metadata block too short\n");
if (s->error_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
return 0;
}
if (len > 0) {
if (!(desc = av_malloc(len + 1))) {
return AVERROR(ENOMEM);
}

bytestream2_get_bufferu(&g, desc, len);
desc[len] = 0;
}


width = bytestream2_get_be32u(&g);
height = bytestream2_get_be32u(&g);
bytestream2_skipu(&g, 8);


len = bytestream2_get_be32u(&g);
if (len <= 0 || len > bytestream2_get_bytes_left(&g)) {
av_log(s, AV_LOG_ERROR, "Attached picture metadata block too short\n");
if (s->error_recognition & AV_EF_EXPLODE)
ret = AVERROR_INVALIDDATA;
goto fail;
}
if (!(data = av_buffer_alloc(len + AV_INPUT_BUFFER_PADDING_SIZE))) {
RETURN_ERROR(AVERROR(ENOMEM));
}
bytestream2_get_bufferu(&g, data->data, len);
memset(data->data + len, 0, AV_INPUT_BUFFER_PADDING_SIZE);

if (AV_RB64(data->data) == PNGSIG)
id = AV_CODEC_ID_PNG;

st = avformat_new_stream(s, NULL);
if (!st) {
RETURN_ERROR(AVERROR(ENOMEM));
}

av_init_packet(&st->attached_pic);
st->attached_pic.buf = data;
st->attached_pic.data = data->data;
st->attached_pic.size = len;
st->attached_pic.stream_index = st->index;
st->attached_pic.flags |= AV_PKT_FLAG_KEY;

st->disposition |= AV_DISPOSITION_ATTACHED_PIC;
st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
st->codecpar->codec_id = id;
st->codecpar->width = width;
st->codecpar->height = height;
av_dict_set(&st->metadata, "comment", ff_id3v2_picture_types[type], 0);
if (desc)
av_dict_set(&st->metadata, "title", desc, AV_DICT_DONT_STRDUP_VAL);

return 0;

fail:
av_buffer_unref(&data);
av_freep(&desc);

return ret;
}
