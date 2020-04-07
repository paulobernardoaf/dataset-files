#include <stdint.h>
#include <sys/types.h>
#include <mfx/mfxvideo.h>
#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "qsv.h"
#include "qsv_internal.h"
#include "qsvenc.h"
typedef struct QSVMJPEGEncContext {
AVClass *class;
QSVEncContext qsv;
} QSVMJPEGEncContext;
static av_cold int qsv_enc_init(AVCodecContext *avctx)
{
QSVMJPEGEncContext *q = avctx->priv_data;
return ff_qsv_enc_init(avctx, &q->qsv);
}
static int qsv_enc_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
QSVMJPEGEncContext *q = avctx->priv_data;
return ff_qsv_encode(avctx, &q->qsv, pkt, frame, got_packet);
}
static av_cold int qsv_enc_close(AVCodecContext *avctx)
{
QSVMJPEGEncContext *q = avctx->priv_data;
return ff_qsv_enc_close(avctx, &q->qsv);
}
#define OFFSET(x) offsetof(QSVMJPEGEncContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "async_depth", "Maximum processing parallelism", OFFSET(qsv.async_depth), AV_OPT_TYPE_INT, { .i64 = ASYNC_DEPTH_DEFAULT }, 1, INT_MAX, VE },
{ NULL },
};
static const AVClass class = {
.class_name = "mjpeg_qsv encoder",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
static const AVCodecDefault qsv_enc_defaults[] = {
{ "global_quality", "80" },
{ NULL },
};
AVCodec ff_mjpeg_qsv_encoder = {
.name = "mjpeg_qsv",
.long_name = NULL_IF_CONFIG_SMALL("MJPEG (Intel Quick Sync Video acceleration)"),
.priv_data_size = sizeof(QSVMJPEGEncContext),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_MJPEG,
.init = qsv_enc_init,
.encode2 = qsv_enc_frame,
.close = qsv_enc_close,
.capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_HYBRID,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_NV12,
AV_PIX_FMT_QSV,
AV_PIX_FMT_NONE },
.priv_class = &class,
.defaults = qsv_enc_defaults,
.wrapper_name = "qsv",
};
