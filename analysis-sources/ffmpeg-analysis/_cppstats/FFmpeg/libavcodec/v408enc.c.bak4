





















#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

static av_cold int v408_encode_init(AVCodecContext *avctx)
{
avctx->bits_per_coded_sample = 32;
avctx->bit_rate = ff_guess_coded_bitrate(avctx);

return 0;
}

static int v408_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pic, int *got_packet)
{
uint8_t *dst;
uint8_t *y, *u, *v, *a;
int i, j, ret;

if ((ret = ff_alloc_packet2(avctx, pkt, avctx->width * avctx->height * 4, 0)) < 0)
return ret;
dst = pkt->data;

y = pic->data[0];
u = pic->data[1];
v = pic->data[2];
a = pic->data[3];

for (i = 0; i < avctx->height; i++) {
for (j = 0; j < avctx->width; j++) {
if (avctx->codec_id==AV_CODEC_ID_AYUV) {
*dst++ = v[j];
*dst++ = u[j];
*dst++ = y[j];
*dst++ = a[j];
} else {
*dst++ = u[j];
*dst++ = y[j];
*dst++ = v[j];
*dst++ = a[j];
}
}
y += pic->linesize[0];
u += pic->linesize[1];
v += pic->linesize[2];
a += pic->linesize[3];
}

pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}

static av_cold int v408_encode_close(AVCodecContext *avctx)
{
return 0;
}

#if CONFIG_AYUV_ENCODER
AVCodec ff_ayuv_encoder = {
.name = "ayuv",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed MS 4:4:4:4"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_AYUV,
.init = v408_encode_init,
.encode2 = v408_encode_frame,
.close = v408_encode_close,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUVA444P, AV_PIX_FMT_NONE },
.capabilities = AV_CODEC_CAP_INTRA_ONLY,
};
#endif
#if CONFIG_V408_ENCODER
AVCodec ff_v408_encoder = {
.name = "v408",
.long_name = NULL_IF_CONFIG_SMALL("Uncompressed packed QT 4:4:4:4"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_V408,
.init = v408_encode_init,
.encode2 = v408_encode_frame,
.close = v408_encode_close,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUVA444P, AV_PIX_FMT_NONE },
.capabilities = AV_CODEC_CAP_INTRA_ONLY,
};
#endif
