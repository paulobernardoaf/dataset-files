#include "libavutil/imgutils.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#define MAX_RLE_BULK 127
#define MAX_RLE_REPEAT 128
#define MAX_RLE_SKIP 254
typedef struct QtrleEncContext {
AVCodecContext *avctx;
int pixel_size;
AVFrame *previous_frame;
unsigned int max_buf_size;
int logical_width;
signed char *rlecode_table;
int *length_table;
uint8_t* skip_table;
int key_frame;
} QtrleEncContext;
static av_cold int qtrle_encode_end(AVCodecContext *avctx)
{
QtrleEncContext *s = avctx->priv_data;
av_frame_free(&s->previous_frame);
av_free(s->rlecode_table);
av_free(s->length_table);
av_free(s->skip_table);
return 0;
}
static av_cold int qtrle_encode_init(AVCodecContext *avctx)
{
QtrleEncContext *s = avctx->priv_data;
if (av_image_check_size(avctx->width, avctx->height, 0, avctx) < 0) {
return AVERROR(EINVAL);
}
s->avctx=avctx;
s->logical_width=avctx->width;
switch (avctx->pix_fmt) {
case AV_PIX_FMT_GRAY8:
if (avctx->width % 4) {
av_log(avctx, AV_LOG_ERROR, "Width not being a multiple of 4 is not supported\n");
return AVERROR(EINVAL);
}
s->logical_width = avctx->width / 4;
s->pixel_size = 4;
break;
case AV_PIX_FMT_RGB555BE:
s->pixel_size = 2;
break;
case AV_PIX_FMT_RGB24:
s->pixel_size = 3;
break;
case AV_PIX_FMT_ARGB:
s->pixel_size = 4;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unsupported colorspace.\n");
break;
}
avctx->bits_per_coded_sample = avctx->pix_fmt == AV_PIX_FMT_GRAY8 ? 40 : s->pixel_size*8;
s->rlecode_table = av_mallocz(s->logical_width);
s->skip_table = av_mallocz(s->logical_width);
s->length_table = av_mallocz_array(s->logical_width + 1, sizeof(int));
if (!s->skip_table || !s->length_table || !s->rlecode_table) {
av_log(avctx, AV_LOG_ERROR, "Error allocating memory.\n");
return AVERROR(ENOMEM);
}
s->previous_frame = av_frame_alloc();
if (!s->previous_frame) {
av_log(avctx, AV_LOG_ERROR, "Error allocating picture\n");
return AVERROR(ENOMEM);
}
s->max_buf_size = s->logical_width*s->avctx->height*s->pixel_size*2 
+ 15 
+ s->avctx->height*2 
+ s->logical_width/MAX_RLE_BULK + 1 ;
return 0;
}
static void qtrle_encode_line(QtrleEncContext *s, const AVFrame *p, int line, uint8_t **buf)
{
int width=s->logical_width;
int i;
signed char rlecode;
unsigned int skipcount;
unsigned int av_uninit(repeatcount);
int total_skip_cost;
int total_repeat_cost;
int base_bulk_cost;
int lowest_bulk_cost;
int lowest_bulk_cost_index;
int sec_lowest_bulk_cost;
int sec_lowest_bulk_cost_index;
uint8_t *this_line = p-> data[0] + line*p-> linesize[0] +
(width - 1)*s->pixel_size;
uint8_t *prev_line = s->previous_frame->data[0] + line * s->previous_frame->linesize[0] +
(width - 1)*s->pixel_size;
s->length_table[width] = 0;
skipcount = 0;
lowest_bulk_cost = INT_MAX / 2;
lowest_bulk_cost_index = width;
sec_lowest_bulk_cost = INT_MAX / 2;
sec_lowest_bulk_cost_index = width;
base_bulk_cost = 1 + s->pixel_size;
for (i = width - 1; i >= 0; i--) {
int prev_bulk_cost;
if (FFMIN(width, i + MAX_RLE_BULK) < lowest_bulk_cost_index) {
lowest_bulk_cost = sec_lowest_bulk_cost;
lowest_bulk_cost_index = sec_lowest_bulk_cost_index;
sec_lowest_bulk_cost = INT_MAX / 2;
sec_lowest_bulk_cost_index = width;
}
if (!i) {
base_bulk_cost++;
lowest_bulk_cost++;
sec_lowest_bulk_cost++;
}
prev_bulk_cost = s->length_table[i + 1] + base_bulk_cost;
if (prev_bulk_cost <= sec_lowest_bulk_cost) {
if (prev_bulk_cost <= lowest_bulk_cost) {
sec_lowest_bulk_cost = INT_MAX / 2;
lowest_bulk_cost = prev_bulk_cost;
lowest_bulk_cost_index = i + 1;
} else {
sec_lowest_bulk_cost = prev_bulk_cost;
sec_lowest_bulk_cost_index = i + 1;
}
}
if (!s->key_frame && !memcmp(this_line, prev_line, s->pixel_size))
skipcount = FFMIN(skipcount + 1, MAX_RLE_SKIP);
else
skipcount = 0;
total_skip_cost = s->length_table[i + skipcount] + 2;
s->skip_table[i] = skipcount;
if (i < width - 1 && !memcmp(this_line, this_line + s->pixel_size, s->pixel_size))
repeatcount = FFMIN(repeatcount + 1, MAX_RLE_REPEAT);
else
repeatcount = 1;
total_repeat_cost = s->length_table[i + repeatcount] + 1 + s->pixel_size;
if (i == 0) {
total_skip_cost--;
total_repeat_cost++;
}
if (repeatcount > 1 && (skipcount == 0 || total_repeat_cost < total_skip_cost)) {
s->length_table[i] = total_repeat_cost;
s->rlecode_table[i] = -repeatcount;
}
else if (skipcount > 0) {
s->length_table[i] = total_skip_cost;
s->rlecode_table[i] = 0;
}
else {
s->length_table[i] = lowest_bulk_cost;
s->rlecode_table[i] = lowest_bulk_cost_index - i;
}
lowest_bulk_cost += s->pixel_size;
sec_lowest_bulk_cost += s->pixel_size;
if (this_line >= p->data[0] + s->pixel_size)
this_line -= s->pixel_size;
if (prev_line >= s->previous_frame->data[0] + s->pixel_size)
prev_line -= s->pixel_size;
}
i=0;
this_line = p-> data[0] + line*p->linesize[0];
if (s->rlecode_table[0] == 0) {
bytestream_put_byte(buf, s->skip_table[0] + 1);
i += s->skip_table[0];
}
else bytestream_put_byte(buf, 1);
while (i < width) {
rlecode = s->rlecode_table[i];
bytestream_put_byte(buf, rlecode);
if (rlecode == 0) {
bytestream_put_byte(buf, s->skip_table[i] + 1);
i += s->skip_table[i];
}
else if (rlecode > 0) {
if (s->avctx->pix_fmt == AV_PIX_FMT_GRAY8) {
int j;
for (j = 0; j < rlecode*s->pixel_size; ++j)
bytestream_put_byte(buf, *(this_line + i*s->pixel_size + j) ^ 0xff);
} else {
bytestream_put_buffer(buf, this_line + i*s->pixel_size, rlecode*s->pixel_size);
}
i += rlecode;
}
else {
if (s->avctx->pix_fmt == AV_PIX_FMT_GRAY8) {
int j;
for (j = 0; j < s->pixel_size; ++j)
bytestream_put_byte(buf, *(this_line + i*s->pixel_size + j) ^ 0xff);
} else {
bytestream_put_buffer(buf, this_line + i*s->pixel_size, s->pixel_size);
}
i -= rlecode;
}
}
bytestream_put_byte(buf, -1); 
}
static int encode_frame(QtrleEncContext *s, const AVFrame *p, uint8_t *buf)
{
int i;
int start_line = 0;
int end_line = s->avctx->height;
uint8_t *orig_buf = buf;
if (!s->key_frame) {
unsigned line_size = s->logical_width * s->pixel_size;
for (start_line = 0; start_line < s->avctx->height; start_line++)
if (memcmp(p->data[0] + start_line*p->linesize[0],
s->previous_frame->data[0] + start_line * s->previous_frame->linesize[0],
line_size))
break;
for (end_line=s->avctx->height; end_line > start_line; end_line--)
if (memcmp(p->data[0] + (end_line - 1)*p->linesize[0],
s->previous_frame->data[0] + (end_line - 1) * s->previous_frame->linesize[0],
line_size))
break;
}
bytestream_put_be32(&buf, 0); 
if ((start_line == 0 && end_line == s->avctx->height) || start_line == s->avctx->height)
bytestream_put_be16(&buf, 0); 
else {
bytestream_put_be16(&buf, 8); 
bytestream_put_be16(&buf, start_line); 
bytestream_put_be16(&buf, 0); 
bytestream_put_be16(&buf, end_line - start_line); 
bytestream_put_be16(&buf, 0); 
}
for (i = start_line; i < end_line; i++)
qtrle_encode_line(s, p, i, &buf);
bytestream_put_byte(&buf, 0); 
AV_WB32(orig_buf, buf - orig_buf); 
return buf - orig_buf;
}
static int qtrle_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pict, int *got_packet)
{
QtrleEncContext * const s = avctx->priv_data;
int ret;
if ((ret = ff_alloc_packet2(avctx, pkt, s->max_buf_size, 0)) < 0)
return ret;
if (avctx->gop_size == 0 || (s->avctx->frame_number % avctx->gop_size) == 0) {
s->key_frame = 1;
} else {
s->key_frame = 0;
}
pkt->size = encode_frame(s, pict, pkt->data);
av_frame_unref(s->previous_frame);
ret = av_frame_ref(s->previous_frame, pict);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "cannot add reference\n");
return ret;
}
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->key_frame = s->key_frame;
avctx->coded_frame->pict_type = s->key_frame ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_P;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
if (s->key_frame)
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
AVCodec ff_qtrle_encoder = {
.name = "qtrle",
.long_name = NULL_IF_CONFIG_SMALL("QuickTime Animation (RLE) video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_QTRLE,
.priv_data_size = sizeof(QtrleEncContext),
.init = qtrle_encode_init,
.encode2 = qtrle_encode_frame,
.close = qtrle_encode_end,
.pix_fmts = (const enum AVPixelFormat[]){
AV_PIX_FMT_RGB24, AV_PIX_FMT_RGB555BE, AV_PIX_FMT_ARGB, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE
},
};
