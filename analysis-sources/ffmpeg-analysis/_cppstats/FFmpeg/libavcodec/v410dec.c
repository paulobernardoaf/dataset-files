#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
#include "thread.h"
typedef struct ThreadData {
AVFrame *frame;
uint8_t *buf;
int stride;
} ThreadData;
static av_cold int v410_decode_init(AVCodecContext *avctx)
{
avctx->pix_fmt = AV_PIX_FMT_YUV444P10;
avctx->bits_per_raw_sample = 10;
if (avctx->width & 1) {
if (avctx->err_recognition & AV_EF_EXPLODE) {
av_log(avctx, AV_LOG_ERROR, "v410 requires width to be even.\n");
return AVERROR_INVALIDDATA;
} else {
av_log(avctx, AV_LOG_WARNING, "v410 requires width to be even, continuing anyway.\n");
}
}
return 0;
}
static int v410_decode_slice(AVCodecContext *avctx, void *arg, int jobnr, int threadnr)
{
ThreadData *td = arg;
AVFrame *pic = td->frame;
int stride = td->stride;
int thread_count = av_clip(avctx->thread_count, 1, avctx->height/4);
int slice_start = (avctx->height * jobnr) / thread_count;
int slice_end = (avctx->height * (jobnr+1)) / thread_count;
const uint8_t *src = td->buf + stride * slice_start;
uint16_t *y, *u, *v;
uint32_t val;
int i, j;
y = (uint16_t*)pic->data[0] + slice_start * (pic->linesize[0] >> 1);
u = (uint16_t*)pic->data[1] + slice_start * (pic->linesize[1] >> 1);
v = (uint16_t*)pic->data[2] + slice_start * (pic->linesize[2] >> 1);
for (i = slice_start; i < slice_end; i++) {
for (j = 0; j < avctx->width; j++) {
val = AV_RL32(src);
u[j] = (val >> 2) & 0x3FF;
y[j] = (val >> 12) & 0x3FF;
v[j] = (val >> 22);
src += 4;
}
y += pic->linesize[0] >> 1;
u += pic->linesize[1] >> 1;
v += pic->linesize[2] >> 1;
}
return 0;
}
static int v410_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
ThreadData td;
ThreadFrame frame = { .f = data };
AVFrame *pic = data;
uint8_t *src = avpkt->data;
int ret;
int thread_count = av_clip(avctx->thread_count, 1, avctx->height/4);
td.stride = avctx->width * 4;
if (avpkt->size < 4 * avctx->height * avctx->width) {
av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
return AVERROR(EINVAL);
}
if ((ret = ff_thread_get_buffer(avctx, &frame, 0)) < 0)
return ret;
pic->key_frame = 1;
pic->pict_type = AV_PICTURE_TYPE_I;
td.buf = src;
td.frame = pic;
avctx->execute2(avctx, v410_decode_slice, &td, NULL, thread_count);
*got_frame = 1;
return avpkt->size;
}
AVCodec ff_v410_decoder = {
.name = "v410",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:4:4 10-bit"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_V410,
.init = v410_decode_init,
.decode = v410_decode_frame,
.capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS |
AV_CODEC_CAP_FRAME_THREADS
};
