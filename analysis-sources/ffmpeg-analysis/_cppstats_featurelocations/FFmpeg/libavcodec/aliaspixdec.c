




















#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define ALIAS_HEADER_SIZE 10

static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
AVPacket *avpkt)
{
AVFrame *f = data;
GetByteContext gb;
int width, height, ret, bits_pixel, pixel;
uint8_t *out_buf;
uint8_t count;
int x, y;

bytestream2_init(&gb, avpkt->data, avpkt->size);

if (bytestream2_get_bytes_left(&gb) < ALIAS_HEADER_SIZE) {
av_log(avctx, AV_LOG_ERROR, "Header too small %d.\n", avpkt->size);
return AVERROR_INVALIDDATA;
}

width = bytestream2_get_be16u(&gb);
height = bytestream2_get_be16u(&gb);
bytestream2_skipu(&gb, 4); 
bits_pixel = bytestream2_get_be16u(&gb);

if (bits_pixel == 24)
avctx->pix_fmt = AV_PIX_FMT_BGR24;
else if (bits_pixel == 8)
avctx->pix_fmt = AV_PIX_FMT_GRAY8;
else {
av_log(avctx, AV_LOG_ERROR, "Invalid pixel format.\n");
return AVERROR_INVALIDDATA;
}

ret = ff_set_dimensions(avctx, width, height);
if (ret < 0)
return ret;

if (bytestream2_get_bytes_left(&gb) < width*height / 255)
return AVERROR_INVALIDDATA;

ret = ff_get_buffer(avctx, f, 0);
if (ret < 0)
return ret;

f->pict_type = AV_PICTURE_TYPE_I;
f->key_frame = 1;

x = 0;
y = 1;
out_buf = f->data[0];
while (bytestream2_get_bytes_left(&gb) > 0) {
int i;


if (x == avctx->width) {
x = 0;
out_buf = f->data[0] + f->linesize[0] * y++;
if (y > avctx->height) {
av_log(avctx, AV_LOG_ERROR,
"Ended frame decoding with %d bytes left.\n",
bytestream2_get_bytes_left(&gb));
return AVERROR_INVALIDDATA;
}
}


count = bytestream2_get_byteu(&gb);
if (!count || x + count > avctx->width) {
av_log(avctx, AV_LOG_ERROR, "Invalid run length %d.\n", count);
return AVERROR_INVALIDDATA;
}

if (avctx->pix_fmt == AV_PIX_FMT_BGR24) {
pixel = bytestream2_get_be24(&gb);
for (i = 0; i < count; i++) {
AV_WB24(out_buf, pixel);
out_buf += 3;
}
} else { 
pixel = bytestream2_get_byte(&gb);
for (i = 0; i < count; i++)
*out_buf++ = pixel;
}

x += i;
}

if (x != width || y != height) {
av_log(avctx, AV_LOG_ERROR, "Picture stopped at %d,%d.\n", x, y);
return AVERROR_INVALIDDATA;
}

*got_frame = 1;
return avpkt->size;
}

AVCodec ff_alias_pix_decoder = {
.name = "alias_pix",
.long_name = NULL_IF_CONFIG_SMALL("Alias/Wavefront PIX image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_ALIAS_PIX,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
