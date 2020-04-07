






























#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

static int fits_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pict, int *got_packet)
{
AVFrame * const p = (AVFrame *)pict;
uint8_t *bytestream, *bytestream_start, *ptr;
const uint16_t flip = (1 << 15);
uint64_t data_size = 0, padded_data_size = 0;
int ret, bitpix, naxis3 = 1, i, j, k, bytes_left;
int map[] = {2, 0, 1, 3}; 

switch (avctx->pix_fmt) {
case AV_PIX_FMT_GRAY8:
case AV_PIX_FMT_GRAY16BE:
map[0] = 0; 
if (avctx->pix_fmt == AV_PIX_FMT_GRAY8) {
bitpix = 8;
} else {
bitpix = 16;
}
break;
case AV_PIX_FMT_GBRP:
case AV_PIX_FMT_GBRAP:
bitpix = 8;
if (avctx->pix_fmt == AV_PIX_FMT_GBRP) {
naxis3 = 3;
} else {
naxis3 = 4;
}
break;
case AV_PIX_FMT_GBRP16BE:
case AV_PIX_FMT_GBRAP16BE:
bitpix = 16;
if (avctx->pix_fmt == AV_PIX_FMT_GBRP16BE) {
naxis3 = 3;
} else {
naxis3 = 4;
}
break;
default:
av_log(avctx, AV_LOG_ERROR, "unsupported pixel format\n");
return AVERROR(EINVAL);
}

data_size = (bitpix >> 3) * avctx->height * avctx->width * naxis3;
padded_data_size = ((data_size + 2879) / 2880 ) * 2880;

if ((ret = ff_alloc_packet2(avctx, pkt, padded_data_size, 0)) < 0)
return ret;

bytestream_start =
bytestream = pkt->data;

for (k = 0; k < naxis3; k++) {
for (i = 0; i < avctx->height; i++) {
ptr = p->data[map[k]] + (avctx->height - i - 1) * p->linesize[map[k]];
if (bitpix == 16) {
for (j = 0; j < avctx->width; j++) {

bytestream_put_be16(&bytestream, AV_RB16(ptr) ^ flip);
ptr += 2;
}
} else {
memcpy(bytestream, ptr, avctx->width);
bytestream += avctx->width;
}
}
}

bytes_left = padded_data_size - data_size;
memset(bytestream, 0, bytes_left);
bytestream += bytes_left;

pkt->size = bytestream - bytestream_start;
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;

return 0;
}

AVCodec ff_fits_encoder = {
.name = "fits",
.long_name = NULL_IF_CONFIG_SMALL("Flexible Image Transport System"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_FITS,
.encode2 = fits_encode_frame,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_GBRAP16BE,
AV_PIX_FMT_GBRP16BE,
AV_PIX_FMT_GBRP,
AV_PIX_FMT_GBRAP,
AV_PIX_FMT_GRAY16BE,
AV_PIX_FMT_GRAY8,
AV_PIX_FMT_NONE },
};
