





















#include "libavutil/intreadwrite.h"
#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "xwd.h"

#define WINDOW_NAME "lavcxwdenc"
#define WINDOW_NAME_SIZE 11

static int xwd_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *pict, int *got_packet)
{
enum AVPixelFormat pix_fmt = avctx->pix_fmt;
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
uint32_t pixdepth, bpp, bpad, ncolors = 0, lsize, vclass, be = 0;
uint32_t rgb[3] = { 0 }, bitorder = 0;
uint32_t header_size;
int i, out_size, ret;
uint8_t *ptr, *buf;
AVFrame * const p = (AVFrame *)pict;
uint32_t pal[256];

pixdepth = av_get_bits_per_pixel(desc);
if (desc->flags & AV_PIX_FMT_FLAG_BE)
be = 1;
switch (pix_fmt) {
case AV_PIX_FMT_ARGB:
case AV_PIX_FMT_BGRA:
case AV_PIX_FMT_RGBA:
case AV_PIX_FMT_ABGR:
if (pix_fmt == AV_PIX_FMT_ARGB ||
pix_fmt == AV_PIX_FMT_ABGR)
be = 1;
if (pix_fmt == AV_PIX_FMT_ABGR ||
pix_fmt == AV_PIX_FMT_RGBA) {
rgb[0] = 0xFF;
rgb[1] = 0xFF00;
rgb[2] = 0xFF0000;
} else {
rgb[0] = 0xFF0000;
rgb[1] = 0xFF00;
rgb[2] = 0xFF;
}
bpp = 32;
pixdepth = 24;
vclass = XWD_TRUE_COLOR;
bpad = 32;
break;
case AV_PIX_FMT_BGR24:
case AV_PIX_FMT_RGB24:
if (pix_fmt == AV_PIX_FMT_RGB24)
be = 1;
bpp = 24;
vclass = XWD_TRUE_COLOR;
bpad = 32;
rgb[0] = 0xFF0000;
rgb[1] = 0xFF00;
rgb[2] = 0xFF;
break;
case AV_PIX_FMT_RGB565LE:
case AV_PIX_FMT_RGB565BE:
case AV_PIX_FMT_BGR565LE:
case AV_PIX_FMT_BGR565BE:
if (pix_fmt == AV_PIX_FMT_BGR565LE ||
pix_fmt == AV_PIX_FMT_BGR565BE) {
rgb[0] = 0x1F;
rgb[1] = 0x7E0;
rgb[2] = 0xF800;
} else {
rgb[0] = 0xF800;
rgb[1] = 0x7E0;
rgb[2] = 0x1F;
}
bpp = 16;
vclass = XWD_TRUE_COLOR;
bpad = 16;
break;
case AV_PIX_FMT_RGB555LE:
case AV_PIX_FMT_RGB555BE:
case AV_PIX_FMT_BGR555LE:
case AV_PIX_FMT_BGR555BE:
if (pix_fmt == AV_PIX_FMT_BGR555LE ||
pix_fmt == AV_PIX_FMT_BGR555BE) {
rgb[0] = 0x1F;
rgb[1] = 0x3E0;
rgb[2] = 0x7C00;
} else {
rgb[0] = 0x7C00;
rgb[1] = 0x3E0;
rgb[2] = 0x1F;
}
bpp = 16;
vclass = XWD_TRUE_COLOR;
bpad = 16;
break;
case AV_PIX_FMT_RGB8:
case AV_PIX_FMT_BGR8:
case AV_PIX_FMT_RGB4_BYTE:
case AV_PIX_FMT_BGR4_BYTE:
case AV_PIX_FMT_PAL8:
bpp = 8;
vclass = XWD_PSEUDO_COLOR;
bpad = 8;
ncolors = 256;
break;
case AV_PIX_FMT_GRAY8:
bpp = 8;
bpad = 8;
vclass = XWD_STATIC_GRAY;
break;
case AV_PIX_FMT_MONOWHITE:
be = 1;
bitorder = 1;
bpp = 1;
bpad = 8;
vclass = XWD_STATIC_GRAY;
break;
default:
av_log(avctx, AV_LOG_ERROR, "unsupported pixel format\n");
return AVERROR(EINVAL);
}

lsize = FFALIGN(bpp * avctx->width, bpad) / 8;
header_size = XWD_HEADER_SIZE + WINDOW_NAME_SIZE;
out_size = header_size + ncolors * XWD_CMAP_SIZE + avctx->height * lsize;

if ((ret = ff_alloc_packet2(avctx, pkt, out_size, 0)) < 0)
return ret;
buf = pkt->data;

p->key_frame = 1;
p->pict_type = AV_PICTURE_TYPE_I;

bytestream_put_be32(&buf, header_size);
bytestream_put_be32(&buf, XWD_VERSION); 
bytestream_put_be32(&buf, XWD_Z_PIXMAP); 
bytestream_put_be32(&buf, pixdepth); 
bytestream_put_be32(&buf, avctx->width); 
bytestream_put_be32(&buf, avctx->height); 
bytestream_put_be32(&buf, 0); 
bytestream_put_be32(&buf, be); 
bytestream_put_be32(&buf, 32); 
bytestream_put_be32(&buf, bitorder); 
bytestream_put_be32(&buf, bpad); 
bytestream_put_be32(&buf, bpp); 
bytestream_put_be32(&buf, lsize); 
bytestream_put_be32(&buf, vclass); 
bytestream_put_be32(&buf, rgb[0]); 
bytestream_put_be32(&buf, rgb[1]); 
bytestream_put_be32(&buf, rgb[2]); 
bytestream_put_be32(&buf, 8); 
bytestream_put_be32(&buf, ncolors); 
bytestream_put_be32(&buf, ncolors); 
bytestream_put_be32(&buf, avctx->width); 
bytestream_put_be32(&buf, avctx->height); 
bytestream_put_be32(&buf, 0); 
bytestream_put_be32(&buf, 0); 
bytestream_put_be32(&buf, 0); 
bytestream_put_buffer(&buf, WINDOW_NAME, WINDOW_NAME_SIZE);

if (pix_fmt == AV_PIX_FMT_PAL8) {
memcpy(pal, p->data[1], sizeof(pal));
} else {
avpriv_set_systematic_pal2(pal, pix_fmt);
}

for (i = 0; i < ncolors; i++) {
uint32_t val;
uint8_t red, green, blue;

val = pal[i];
red = (val >> 16) & 0xFF;
green = (val >> 8) & 0xFF;
blue = val & 0xFF;

bytestream_put_be32(&buf, i); 
bytestream_put_be16(&buf, red << 8);
bytestream_put_be16(&buf, green << 8);
bytestream_put_be16(&buf, blue << 8);
bytestream_put_byte(&buf, 0x7); 
bytestream_put_byte(&buf, 0); 
}

ptr = p->data[0];
for (i = 0; i < avctx->height; i++) {
bytestream_put_buffer(&buf, ptr, lsize);
ptr += p->linesize[0];
}

pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}

AVCodec ff_xwd_encoder = {
.name = "xwd",
.long_name = NULL_IF_CONFIG_SMALL("XWD (X Window Dump) image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_XWD,
.encode2 = xwd_encode_frame,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_BGRA,
AV_PIX_FMT_RGBA,
AV_PIX_FMT_ARGB,
AV_PIX_FMT_ABGR,
AV_PIX_FMT_RGB24,
AV_PIX_FMT_BGR24,
AV_PIX_FMT_RGB565BE,
AV_PIX_FMT_RGB565LE,
AV_PIX_FMT_BGR565BE,
AV_PIX_FMT_BGR565LE,
AV_PIX_FMT_RGB555BE,
AV_PIX_FMT_RGB555LE,
AV_PIX_FMT_BGR555BE,
AV_PIX_FMT_BGR555LE,
AV_PIX_FMT_RGB8,
AV_PIX_FMT_BGR8,
AV_PIX_FMT_RGB4_BYTE,
AV_PIX_FMT_BGR4_BYTE,
AV_PIX_FMT_PAL8,
AV_PIX_FMT_GRAY8,
AV_PIX_FMT_MONOWHITE,
AV_PIX_FMT_NONE },
};
