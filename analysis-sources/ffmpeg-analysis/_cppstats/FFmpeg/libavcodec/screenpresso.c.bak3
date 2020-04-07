


































#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "internal.h"

typedef struct ScreenpressoContext {
AVFrame *current;


uint8_t *inflated_buf;
uLongf inflated_size;
} ScreenpressoContext;

static av_cold int screenpresso_close(AVCodecContext *avctx)
{
ScreenpressoContext *ctx = avctx->priv_data;

av_frame_free(&ctx->current);
av_freep(&ctx->inflated_buf);

return 0;
}

static av_cold int screenpresso_init(AVCodecContext *avctx)
{
ScreenpressoContext *ctx = avctx->priv_data;


int ret = av_image_check_size(avctx->width, avctx->height, 0, avctx);
if (ret < 0) {
av_log(avctx, AV_LOG_ERROR, "Invalid image size %dx%d.\n",
avctx->width, avctx->height);
return ret;
}


ctx->current = av_frame_alloc();
if (!ctx->current)
return AVERROR(ENOMEM);


ctx->inflated_size = avctx->width * avctx->height * 4;
ctx->inflated_buf = av_malloc(ctx->inflated_size);
if (!ctx->inflated_buf)
return AVERROR(ENOMEM);

return 0;
}

static void sum_delta_flipped(uint8_t *dst, int dst_linesize,
const uint8_t *src, int src_linesize,
int bytewidth, int height)
{
int i;
for (; height > 0; height--) {
const uint8_t *src1 = &src[(height - 1) * src_linesize];
for (i = 0; i < bytewidth; i++)
dst[i] += src1[i];
dst += dst_linesize;
}
}

static int screenpresso_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
ScreenpressoContext *ctx = avctx->priv_data;
AVFrame *frame = data;
uLongf length = ctx->inflated_size;
int keyframe, component_size, src_linesize;
int ret;


if (avpkt->size < 3) {
av_log(avctx, AV_LOG_ERROR, "Packet too small (%d)\n", avpkt->size);
return AVERROR_INVALIDDATA;
}


av_log(avctx, AV_LOG_DEBUG, "Compression level %d\n", avpkt->data[0] >> 4);
keyframe = avpkt->data[0] & 1;


component_size = ((avpkt->data[1] >> 2) & 0x03) + 1;
switch (component_size) {
case 2:
avctx->pix_fmt = AV_PIX_FMT_RGB555LE;
break;
case 3:
avctx->pix_fmt = AV_PIX_FMT_BGR24;
break;
case 4:
avctx->pix_fmt = AV_PIX_FMT_BGR0;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Invalid bits per pixel value (%d)\n",
component_size);
return AVERROR_INVALIDDATA;
}


ret = uncompress(ctx->inflated_buf, &length,
avpkt->data + 2, avpkt->size - 2);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "Deflate error %d.\n", ret);
return AVERROR_UNKNOWN;
}

ret = ff_reget_buffer(avctx, ctx->current, 0);
if (ret < 0)
return ret;


src_linesize = FFALIGN(avctx->width * component_size, 4);


if (keyframe)
av_image_copy_plane(ctx->current->data[0] +
ctx->current->linesize[0] * (avctx->height - 1),
-1 * ctx->current->linesize[0],
ctx->inflated_buf, src_linesize,
avctx->width * component_size, avctx->height);

else
sum_delta_flipped(ctx->current->data[0], ctx->current->linesize[0],
ctx->inflated_buf, src_linesize,
avctx->width * component_size, avctx->height);


ret = av_frame_ref(frame, ctx->current);
if (ret < 0)
return ret;


if (keyframe) {
frame->pict_type = AV_PICTURE_TYPE_I;
frame->key_frame = 1;
} else {
frame->pict_type = AV_PICTURE_TYPE_P;
}
*got_frame = 1;

return avpkt->size;
}

AVCodec ff_screenpresso_decoder = {
.name = "screenpresso",
.long_name = NULL_IF_CONFIG_SMALL("Screenpresso"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_SCREENPRESSO,
.init = screenpresso_init,
.decode = screenpresso_decode_frame,
.close = screenpresso_close,
.priv_data_size = sizeof(ScreenpressoContext),
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
};
