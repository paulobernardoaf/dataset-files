





























#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "msrledec.h"
#include "libavutil/imgutils.h"

typedef struct MsrleContext {
AVCodecContext *avctx;
AVFrame *frame;

GetByteContext gb;

uint32_t pal[256];
} MsrleContext;

static av_cold int msrle_decode_init(AVCodecContext *avctx)
{
MsrleContext *s = avctx->priv_data;
int i;

s->avctx = avctx;

switch (avctx->bits_per_coded_sample) {
case 1:
avctx->pix_fmt = AV_PIX_FMT_MONOWHITE;
break;
case 4:
case 8:
avctx->pix_fmt = AV_PIX_FMT_PAL8;
break;
case 24:
avctx->pix_fmt = AV_PIX_FMT_BGR24;
break;
default:
av_log(avctx, AV_LOG_ERROR, "unsupported bits per sample\n");
return AVERROR_INVALIDDATA;
}

s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);

if (avctx->extradata_size >= 4)
for (i = 0; i < FFMIN(avctx->extradata_size, AVPALETTE_SIZE)/4; i++)
s->pal[i] = 0xFFU<<24 | AV_RL32(avctx->extradata+4*i);

return 0;
}

static int msrle_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
MsrleContext *s = avctx->priv_data;
int istride = FFALIGN(avctx->width*avctx->bits_per_coded_sample, 32) / 8;
int ret;

if (buf_size < 2) 
return AVERROR_INVALIDDATA;

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

if (avctx->bits_per_coded_sample > 1 && avctx->bits_per_coded_sample <= 8) {
int size;
const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, &size);

if (pal && size == AVPALETTE_SIZE) {
s->frame->palette_has_changed = 1;
memcpy(s->pal, pal, AVPALETTE_SIZE);
} else if (pal) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", size);
}

memcpy(s->frame->data[1], s->pal, AVPALETTE_SIZE);
}


if (avctx->height * istride == avpkt->size) { 
int linesize = av_image_get_linesize(avctx->pix_fmt, avctx->width, 0);
uint8_t *ptr = s->frame->data[0];
uint8_t *buf = avpkt->data + (avctx->height-1)*istride;
int i, j;

if (linesize < 0)
return linesize;

for (i = 0; i < avctx->height; i++) {
if (avctx->bits_per_coded_sample == 4) {
for (j = 0; j < avctx->width - 1; j += 2) {
ptr[j+0] = buf[j>>1] >> 4;
ptr[j+1] = buf[j>>1] & 0xF;
}
if (avctx->width & 1)
ptr[j+0] = buf[j>>1] >> 4;
} else {
memcpy(ptr, buf, linesize);
}
buf -= istride;
ptr += s->frame->linesize[0];
}
} else {
bytestream2_init(&s->gb, buf, buf_size);
ff_msrle_decode(avctx, s->frame, avctx->bits_per_coded_sample, &s->gb);
}

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;


return buf_size;
}

static void msrle_decode_flush(AVCodecContext *avctx)
{
MsrleContext *s = avctx->priv_data;

av_frame_unref(s->frame);
}

static av_cold int msrle_decode_end(AVCodecContext *avctx)
{
MsrleContext *s = avctx->priv_data;


av_frame_free(&s->frame);

return 0;
}

AVCodec ff_msrle_decoder = {
.name = "msrle",
.long_name = NULL_IF_CONFIG_SMALL("Microsoft RLE"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_MSRLE,
.priv_data_size = sizeof(MsrleContext),
.init = msrle_decode_init,
.close = msrle_decode_end,
.decode = msrle_decode_frame,
.flush = msrle_decode_flush,
.capabilities = AV_CODEC_CAP_DR1,
};
