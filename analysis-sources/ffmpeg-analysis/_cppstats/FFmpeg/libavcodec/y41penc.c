#include "avcodec.h"
#include "internal.h"
static av_cold int y41p_encode_init(AVCodecContext *avctx)
{
if (avctx->width & 7) {
av_log(avctx, AV_LOG_ERROR, "y41p requires width to be divisible by 8.\n");
return AVERROR_INVALIDDATA;
}
avctx->bits_per_coded_sample = 12;
avctx->bit_rate = ff_guess_coded_bitrate(avctx);
return 0;
}
static int y41p_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pic, int *got_packet)
{
uint8_t *dst;
uint8_t *y, *u, *v;
int i, j, ret;
if ((ret = ff_alloc_packet2(avctx, pkt, avctx->width * avctx->height * 1.5, 0)) < 0)
return ret;
dst = pkt->data;
for (i = avctx->height - 1; i >= 0; i--) {
y = &pic->data[0][i * pic->linesize[0]];
u = &pic->data[1][i * pic->linesize[1]];
v = &pic->data[2][i * pic->linesize[2]];
for (j = 0; j < avctx->width; j += 8) {
*(dst++) = *(u++);
*(dst++) = *(y++);
*(dst++) = *(v++);
*(dst++) = *(y++);
*(dst++) = *(u++);
*(dst++) = *(y++);
*(dst++) = *(v++);
*(dst++) = *(y++);
*(dst++) = *(y++);
*(dst++) = *(y++);
*(dst++) = *(y++);
*(dst++) = *(y++);
}
}
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
static av_cold int y41p_encode_close(AVCodecContext *avctx)
{
return 0;
}
AVCodec ff_y41p_encoder = {
.name = "y41p",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed YUV 4:1:1 12-bit"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_Y41P,
.init = y41p_encode_init,
.encode2 = y41p_encode_frame,
.close = y41p_encode_close,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_YUV411P,
AV_PIX_FMT_NONE },
.capabilities = AV_CODEC_CAP_INTRA_ONLY,
};
