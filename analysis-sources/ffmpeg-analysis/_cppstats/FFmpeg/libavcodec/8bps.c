#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"
static const enum AVPixelFormat pixfmt_rgb24[] = {
AV_PIX_FMT_BGR24, AV_PIX_FMT_0RGB32, AV_PIX_FMT_NONE };
typedef struct EightBpsContext {
AVCodecContext *avctx;
unsigned char planes;
unsigned char planemap[4];
uint32_t pal[256];
} EightBpsContext;
static int decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
EightBpsContext * const c = avctx->priv_data;
const unsigned char *encoded = buf;
unsigned char *pixptr, *pixptr_end;
unsigned int height = avctx->height; 
unsigned int dlen, p, row;
const unsigned char *lp, *dp, *ep;
unsigned char count;
unsigned int px_inc;
unsigned int planes = c->planes;
unsigned char *planemap = c->planemap;
int ret;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
ep = encoded + buf_size;
dp = encoded + planes * (height << 1);
px_inc = planes + (avctx->pix_fmt == AV_PIX_FMT_0RGB32);
for (p = 0; p < planes; p++) {
lp = encoded + p * (height << 1);
for (row = 0; row < height; row++) {
pixptr = frame->data[0] + row * frame->linesize[0] + planemap[p];
pixptr_end = pixptr + frame->linesize[0];
if (ep - lp < row * 2 + 2)
return AVERROR_INVALIDDATA;
dlen = av_be2ne16(*(const unsigned short *)(lp + row * 2));
while (dlen > 0) {
if (ep - dp <= 1)
return AVERROR_INVALIDDATA;
if ((count = *dp++) <= 127) {
count++;
dlen -= count + 1;
if (pixptr_end - pixptr < count * px_inc)
break;
if (ep - dp < count)
return AVERROR_INVALIDDATA;
while (count--) {
*pixptr = *dp++;
pixptr += px_inc;
}
} else {
count = 257 - count;
if (pixptr_end - pixptr < count * px_inc)
break;
while (count--) {
*pixptr = *dp;
pixptr += px_inc;
}
dp++;
dlen -= 2;
}
}
}
}
if (avctx->bits_per_coded_sample <= 8) {
int size;
const uint8_t *pal = av_packet_get_side_data(avpkt,
AV_PKT_DATA_PALETTE,
&size);
if (pal && size == AVPALETTE_SIZE) {
frame->palette_has_changed = 1;
memcpy(c->pal, pal, AVPALETTE_SIZE);
} else if (pal) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
}
memcpy (frame->data[1], c->pal, AVPALETTE_SIZE);
}
*got_frame = 1;
return buf_size;
}
static av_cold int decode_init(AVCodecContext *avctx)
{
EightBpsContext * const c = avctx->priv_data;
c->avctx = avctx;
switch (avctx->bits_per_coded_sample) {
case 8:
avctx->pix_fmt = AV_PIX_FMT_PAL8;
c->planes = 1;
c->planemap[0] = 0; 
break;
case 24:
avctx->pix_fmt = ff_get_format(avctx, pixfmt_rgb24);
c->planes = 3;
c->planemap[0] = 2; 
c->planemap[1] = 1; 
c->planemap[2] = 0; 
break;
case 32:
avctx->pix_fmt = AV_PIX_FMT_RGB32;
c->planes = 4;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Error: Unsupported color depth: %u.\n",
avctx->bits_per_coded_sample);
return AVERROR_INVALIDDATA;
}
if (avctx->pix_fmt == AV_PIX_FMT_RGB32) {
c->planemap[0] = HAVE_BIGENDIAN ? 1 : 2; 
c->planemap[1] = HAVE_BIGENDIAN ? 2 : 1; 
c->planemap[2] = HAVE_BIGENDIAN ? 3 : 0; 
c->planemap[3] = HAVE_BIGENDIAN ? 0 : 3; 
}
return 0;
}
AVCodec ff_eightbps_decoder = {
.name = "8bps",
.long_name = NULL_IF_CONFIG_SMALL("QuickTime 8BPS video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_8BPS,
.priv_data_size = sizeof(EightBpsContext),
.init = decode_init,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
