#include "avcodec.h"
#include "internal.h"
static av_cold int v308_decode_init(AVCodecContext *avctx)
{
avctx->pix_fmt = AV_PIX_FMT_YUV444P;
if (avctx->width & 1)
av_log(avctx, AV_LOG_WARNING, "v308 requires width to be even.\n");
return 0;
}
static int v308_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
AVFrame *pic = data;
const uint8_t *src = avpkt->data;
uint8_t *y, *u, *v;
int i, j, ret;
if (avpkt->size < 3 * avctx->height * avctx->width) {
av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
return AVERROR(EINVAL);
}
if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
return ret;
pic->key_frame = 1;
pic->pict_type = AV_PICTURE_TYPE_I;
y = pic->data[0];
u = pic->data[1];
v = pic->data[2];
for (i = 0; i < avctx->height; i++) {
for (j = 0; j < avctx->width; j++) {
v[j] = *src++;
y[j] = *src++;
u[j] = *src++;
}
y += pic->linesize[0];
u += pic->linesize[1];
v += pic->linesize[2];
}
*got_frame = 1;
return avpkt->size;
}
AVCodec ff_v308_decoder = {
.name = "v308",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed 4:4:4"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_V308,
.init = v308_decode_init,
.decode = v308_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
