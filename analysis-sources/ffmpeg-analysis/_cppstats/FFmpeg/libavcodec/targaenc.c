#include <string.h>
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avcodec.h"
#include "internal.h"
#include "rle.h"
#include "targa.h"
typedef struct TargaContext {
AVClass *class;
int rle;
} TargaContext;
static int targa_encode_rle(uint8_t *outbuf, int out_size, const AVFrame *pic,
int bpp, int w, int h)
{
int y,ret;
uint8_t *out;
out = outbuf;
for(y = 0; y < h; y ++) {
ret = ff_rle_encode(out, out_size, pic->data[0] + pic->linesize[0] * y, bpp, w, 0x7f, 0, -1, 0);
if(ret == -1){
return -1;
}
out+= ret;
out_size -= ret;
}
return out - outbuf;
}
static int targa_encode_normal(uint8_t *outbuf, const AVFrame *pic, int bpp, int w, int h)
{
int i, n = bpp * w;
uint8_t *out = outbuf;
uint8_t *ptr = pic->data[0];
for(i=0; i < h; i++) {
memcpy(out, ptr, n);
out += n;
ptr += pic->linesize[0];
}
return out - outbuf;
}
static int targa_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *p, int *got_packet)
{
TargaContext *s = avctx->priv_data;
int bpp, picsize, datasize = -1, ret, i;
uint8_t *out;
picsize = av_image_get_buffer_size(avctx->pix_fmt,
avctx->width, avctx->height, 1);
if ((ret = ff_alloc_packet2(avctx, pkt, picsize + 45, 0)) < 0)
return ret;
memset(pkt->data, 0, 12);
AV_WL16(pkt->data+12, avctx->width);
AV_WL16(pkt->data+14, avctx->height);
pkt->data[17] = 0x20 | (avctx->pix_fmt == AV_PIX_FMT_BGRA ? 8 : 0);
out = pkt->data + 18; 
avctx->bits_per_coded_sample = av_get_bits_per_pixel(av_pix_fmt_desc_get(avctx->pix_fmt));
switch(avctx->pix_fmt) {
case AV_PIX_FMT_PAL8: {
int pal_bpp = 24; 
for (i = 0; i < 256; i++)
if (AV_RN32(p->data[1] + 4 * i) >> 24 != 0xFF) {
pal_bpp = 32;
break;
}
pkt->data[1] = 1; 
pkt->data[2] = TGA_PAL; 
pkt->data[6] = 1; 
pkt->data[7] = pal_bpp; 
pkt->data[16] = 8; 
for (i = 0; i < 256; i++)
if (pal_bpp == 32) {
AV_WL32(pkt->data + 18 + 4 * i, *(uint32_t *)(p->data[1] + i * 4));
} else {
AV_WL24(pkt->data + 18 + 3 * i, *(uint32_t *)(p->data[1] + i * 4));
}
out += 32 * pal_bpp; 
break;
}
case AV_PIX_FMT_GRAY8:
pkt->data[2] = TGA_BW; 
avctx->bits_per_coded_sample = 0x28;
pkt->data[16] = 8; 
break;
case AV_PIX_FMT_RGB555LE:
pkt->data[2] = TGA_RGB; 
avctx->bits_per_coded_sample =
pkt->data[16] = 16; 
break;
case AV_PIX_FMT_BGR24:
pkt->data[2] = TGA_RGB; 
pkt->data[16] = 24; 
break;
case AV_PIX_FMT_BGRA:
pkt->data[2] = TGA_RGB; 
pkt->data[16] = 32; 
break;
default:
av_log(avctx, AV_LOG_ERROR, "Pixel format '%s' not supported.\n",
av_get_pix_fmt_name(avctx->pix_fmt));
return AVERROR(EINVAL);
}
bpp = pkt->data[16] >> 3;
#if FF_API_CODER_TYPE
FF_DISABLE_DEPRECATION_WARNINGS
if (avctx->coder_type == FF_CODER_TYPE_RAW)
s->rle = 0;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
if (s->rle)
datasize = targa_encode_rle(out, picsize, p, bpp, avctx->width, avctx->height);
if(datasize >= 0)
pkt->data[2] |= TGA_RLE;
else datasize = targa_encode_normal(out, p, bpp, avctx->width, avctx->height);
out += datasize;
memcpy(out, "\0\0\0\0\0\0\0\0TRUEVISION-XFILE.", 26);
pkt->size = out + 26 - pkt->data;
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
static av_cold int targa_encode_init(AVCodecContext *avctx)
{
if (avctx->width > 0xffff || avctx->height > 0xffff) {
av_log(avctx, AV_LOG_ERROR, "image dimensions too large\n");
return AVERROR(EINVAL);
}
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->key_frame = 1;
avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
return 0;
}
#define OFFSET(x) offsetof(TargaContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "rle", "Use run-length compression", OFFSET(rle), AV_OPT_TYPE_INT, { .i64 = 1 }, 0, 1, VE },
{ NULL },
};
static const AVClass targa_class = {
.class_name = "targa",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
AVCodec ff_targa_encoder = {
.name = "targa",
.long_name = NULL_IF_CONFIG_SMALL("Truevision Targa image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_TARGA,
.priv_data_size = sizeof(TargaContext),
.priv_class = &targa_class,
.init = targa_encode_init,
.encode2 = targa_encode_frame,
.pix_fmts = (const enum AVPixelFormat[]){
AV_PIX_FMT_BGR24, AV_PIX_FMT_BGRA, AV_PIX_FMT_RGB555LE, AV_PIX_FMT_GRAY8, AV_PIX_FMT_PAL8,
AV_PIX_FMT_NONE
},
};
