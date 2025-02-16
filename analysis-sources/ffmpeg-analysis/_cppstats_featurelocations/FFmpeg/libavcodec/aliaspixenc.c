




















#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define ALIAS_HEADER_SIZE 10

static int encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
int width, height, bits_pixel, i, j, length, ret;
uint8_t *in_buf, *buf;

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
avctx->coded_frame->key_frame = 1;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

width = avctx->width;
height = avctx->height;

if (width > 65535 || height > 65535 ||
width * height >= INT_MAX / 4 - ALIAS_HEADER_SIZE) {
av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n", width, height);
return AVERROR_INVALIDDATA;
}

switch (avctx->pix_fmt) {
case AV_PIX_FMT_GRAY8:
bits_pixel = 8;
break;
case AV_PIX_FMT_BGR24:
bits_pixel = 24;
break;
default:
return AVERROR(EINVAL);
}

length = ALIAS_HEADER_SIZE + 4 * width * height; 
if ((ret = ff_alloc_packet2(avctx, pkt, length, ALIAS_HEADER_SIZE + height*2)) < 0) {
av_log(avctx, AV_LOG_ERROR, "Error getting output packet of size %d.\n", length);
return ret;
}

buf = pkt->data;


bytestream_put_be16(&buf, width);
bytestream_put_be16(&buf, height);
bytestream_put_be32(&buf, 0); 
bytestream_put_be16(&buf, bits_pixel);

for (j = 0; j < height; j++) {
in_buf = frame->data[0] + frame->linesize[0] * j;
for (i = 0; i < width; ) {
int count = 0;
int pixel;

if (avctx->pix_fmt == AV_PIX_FMT_GRAY8) {
pixel = *in_buf;
while (count < 255 && count + i < width && pixel == *in_buf) {
count++;
in_buf++;
}
bytestream_put_byte(&buf, count);
bytestream_put_byte(&buf, pixel);
} else { 
pixel = AV_RB24(in_buf);
while (count < 255 && count + i < width &&
pixel == AV_RB24(in_buf)) {
count++;
in_buf += 3;
}
bytestream_put_byte(&buf, count);
bytestream_put_be24(&buf, pixel);
}
i += count;
}
}


av_shrink_packet(pkt, buf - pkt->data);
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;

return 0;
}

AVCodec ff_alias_pix_encoder = {
.name = "alias_pix",
.long_name = NULL_IF_CONFIG_SMALL("Alias/Wavefront PIX image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_ALIAS_PIX,
.encode2 = encode_frame,
.pix_fmts = (const enum AVPixelFormat[]) {
AV_PIX_FMT_BGR24, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE
},
};
