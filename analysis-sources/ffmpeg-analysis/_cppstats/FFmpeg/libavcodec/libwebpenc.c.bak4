

























#include "libwebpenc_common.h"

typedef LibWebPContextCommon LibWebPContext;

static av_cold int libwebp_encode_init(AVCodecContext *avctx)
{
return ff_libwebp_encode_init_common(avctx);
}

static int libwebp_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
LibWebPContext *s = avctx->priv_data;
WebPPicture *pic = NULL;
AVFrame *alt_frame = NULL;
WebPMemoryWriter mw = { 0 };

int ret = ff_libwebp_get_frame(avctx, s, frame, &alt_frame, &pic);
if (ret < 0)
goto end;

WebPMemoryWriterInit(&mw);
pic->custom_ptr = &mw;
pic->writer = WebPMemoryWrite;

ret = WebPEncode(&s->config, pic);
if (!ret) {
av_log(avctx, AV_LOG_ERROR, "WebPEncode() failed with error: %d\n",
pic->error_code);
ret = ff_libwebp_error_to_averror(pic->error_code);
goto end;
}

ret = ff_alloc_packet2(avctx, pkt, mw.size, mw.size);
if (ret < 0)
goto end;
memcpy(pkt->data, mw.mem, mw.size);

pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;

end:
#if (WEBP_ENCODER_ABI_VERSION > 0x0203)
WebPMemoryWriterClear(&mw);
#else
free(mw.mem); 
#endif
WebPPictureFree(pic);
av_freep(&pic);
av_frame_free(&alt_frame);

return ret;
}

static int libwebp_encode_close(AVCodecContext *avctx)
{
LibWebPContextCommon *s = avctx->priv_data;
av_frame_free(&s->ref);

return 0;
}

static const AVClass class = {
.class_name = "libwebp",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_libwebp_encoder = {
.name = "libwebp",
.long_name = NULL_IF_CONFIG_SMALL("libwebp WebP image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_WEBP,
.priv_data_size = sizeof(LibWebPContext),
.init = libwebp_encode_init,
.encode2 = libwebp_encode_frame,
.close = libwebp_encode_close,
.pix_fmts = (const enum AVPixelFormat[]) {
AV_PIX_FMT_RGB32,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVA420P,
AV_PIX_FMT_NONE
},
.priv_class = &class,
.defaults = libwebp_defaults,
.wrapper_name = "libwebp",
};
