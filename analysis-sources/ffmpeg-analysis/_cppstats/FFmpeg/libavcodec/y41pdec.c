#include "avcodec.h"
#include "internal.h"
static av_cold int y41p_decode_init(AVCodecContext *avctx)
{
avctx->pix_fmt = AV_PIX_FMT_YUV411P;
avctx->bits_per_raw_sample = 12;
if (avctx->width & 7) {
av_log(avctx, AV_LOG_WARNING, "y41p requires width to be divisible by 8.\n");
}
return 0;
}
static int y41p_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
AVFrame *pic = data;
uint8_t *src = avpkt->data;
uint8_t *y, *u, *v;
int i, j, ret;
if (avpkt->size < 3LL * avctx->height * FFALIGN(avctx->width, 8) / 2) {
av_log(avctx, AV_LOG_ERROR, "Insufficient input data.\n");
return AVERROR(EINVAL);
}
if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
return ret;
pic->key_frame = 1;
pic->pict_type = AV_PICTURE_TYPE_I;
for (i = avctx->height - 1; i >= 0 ; i--) {
y = &pic->data[0][i * pic->linesize[0]];
u = &pic->data[1][i * pic->linesize[1]];
v = &pic->data[2][i * pic->linesize[2]];
for (j = 0; j < avctx->width; j += 8) {
*(u++) = *src++;
*(y++) = *src++;
*(v++) = *src++;
*(y++) = *src++;
*(u++) = *src++;
*(y++) = *src++;
*(v++) = *src++;
*(y++) = *src++;
*(y++) = *src++;
*(y++) = *src++;
*(y++) = *src++;
*(y++) = *src++;
}
}
*got_frame = 1;
return avpkt->size;
}
AVCodec ff_y41p_decoder = {
.name = "y41p",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed YUV 4:1:1 12-bit"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_Y41P,
.init = y41p_decode_init,
.decode = y41p_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
