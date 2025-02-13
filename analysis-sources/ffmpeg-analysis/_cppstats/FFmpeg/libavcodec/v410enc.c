#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
static av_cold int v410_encode_init(AVCodecContext *avctx)
{
if (avctx->width & 1) {
av_log(avctx, AV_LOG_ERROR, "v410 requires width to be even.\n");
return AVERROR_INVALIDDATA;
}
avctx->bits_per_coded_sample = 32;
avctx->bit_rate = ff_guess_coded_bitrate(avctx);
return 0;
}
static int v410_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pic, int *got_packet)
{
uint8_t *dst;
uint16_t *y, *u, *v;
uint32_t val;
int i, j, ret;
if ((ret = ff_alloc_packet2(avctx, pkt, avctx->width * avctx->height * 4,
avctx->width * avctx->height * 4)) < 0)
return ret;
dst = pkt->data;
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->key_frame = 1;
avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
y = (uint16_t *)pic->data[0];
u = (uint16_t *)pic->data[1];
v = (uint16_t *)pic->data[2];
for (i = 0; i < avctx->height; i++) {
for (j = 0; j < avctx->width; j++) {
val = u[j] << 2;
val |= y[j] << 12;
val |= (uint32_t) v[j] << 22;
AV_WL32(dst, val);
dst += 4;
}
y += pic->linesize[0] >> 1;
u += pic->linesize[1] >> 1;
v += pic->linesize[2] >> 1;
}
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
AVCodec ff_v410_encoder = {
.name = "v410",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed 4:4:4 10-bit"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_V410,
.init = v410_encode_init,
.encode2 = v410_encode_frame,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV444P10, AV_PIX_FMT_NONE },
};
