#include "avcodec.h"
#include "internal.h"
#include "libavutil/bswap.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"
static av_cold int decode_init(AVCodecContext *avctx)
{
if (avctx->width & 1) {
av_log(avctx, AV_LOG_ERROR, "v210x needs even width\n");
return AVERROR(EINVAL);
}
avctx->pix_fmt = AV_PIX_FMT_YUV422P16;
avctx->bits_per_raw_sample = 10;
return 0;
}
static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
AVPacket *avpkt)
{
const uint32_t *src = (const uint32_t *)avpkt->data;
AVFrame *pic = data;
int width = avctx->width;
int y = 0;
uint16_t *ydst, *udst, *vdst, *yend;
int ret;
if (avpkt->size < avctx->width * avctx->height * 8 / 3) {
av_log(avctx, AV_LOG_ERROR, "Packet too small\n");
return AVERROR_INVALIDDATA;
}
if (avpkt->size > avctx->width * avctx->height * 8 / 3) {
avpriv_request_sample(avctx, "(Probably) padded data");
}
if ((ret = ff_get_buffer(avctx, pic, 0)) < 0)
return ret;
ydst = (uint16_t *)pic->data[0];
udst = (uint16_t *)pic->data[1];
vdst = (uint16_t *)pic->data[2];
yend = ydst + width;
pic->pict_type = AV_PICTURE_TYPE_I;
pic->key_frame = 1;
for (;;) {
uint32_t v = av_be2ne32(*src++);
*udst++ = (v >> 16) & 0xFFC0;
*ydst++ = (v >> 6 ) & 0xFFC0;
*vdst++ = (v << 4 ) & 0xFFC0;
v = av_be2ne32(*src++);
*ydst++ = (v >> 16) & 0xFFC0;
if (ydst >= yend) {
ydst += pic->linesize[0] / 2 - width;
udst += pic->linesize[1] / 2 - width / 2;
vdst += pic->linesize[2] / 2 - width / 2;
yend = ydst + width;
if (++y >= avctx->height)
break;
}
*udst++ = (v >> 6 ) & 0xFFC0;
*ydst++ = (v << 4 ) & 0xFFC0;
v = av_be2ne32(*src++);
*vdst++ = (v >> 16) & 0xFFC0;
*ydst++ = (v >> 6 ) & 0xFFC0;
if (ydst >= yend) {
ydst += pic->linesize[0] / 2 - width;
udst += pic->linesize[1] / 2 - width / 2;
vdst += pic->linesize[2] / 2 - width / 2;
yend = ydst + width;
if (++y >= avctx->height)
break;
}
*udst++ = (v << 4 ) & 0xFFC0;
v = av_be2ne32(*src++);
*ydst++ = (v >> 16) & 0xFFC0;
*vdst++ = (v >> 6 ) & 0xFFC0;
*ydst++ = (v << 4 ) & 0xFFC0;
if (ydst >= yend) {
ydst += pic->linesize[0] / 2 - width;
udst += pic->linesize[1] / 2 - width / 2;
vdst += pic->linesize[2] / 2 - width / 2;
yend = ydst + width;
if (++y >= avctx->height)
break;
}
}
*got_frame = 1;
return avpkt->size;
}
AVCodec ff_v210x_decoder = {
.name = "v210x",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_V210X,
.init = decode_init,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
