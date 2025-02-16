






















#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/decode.h"
#include "libavcodec/internal.h"

#include "v4l2_context.h"
#include "v4l2_m2m.h"
#include "v4l2_fmt.h"

static int v4l2_try_start(AVCodecContext *avctx)
{
V4L2m2mContext *s = ((V4L2m2mPriv*)avctx->priv_data)->context;
V4L2Context *const capture = &s->capture;
V4L2Context *const output = &s->output;
struct v4l2_selection selection = { 0 };
int ret;


if (!output->streamon) {
ret = ff_v4l2_context_set_status(output, VIDIOC_STREAMON);
if (ret < 0) {
av_log(avctx, AV_LOG_DEBUG, "VIDIOC_STREAMON on output context\n");
return ret;
}
}

if (capture->streamon)
return 0;


capture->format.type = capture->type;
ret = ioctl(s->fd, VIDIOC_G_FMT, &capture->format);
if (ret) {
av_log(avctx, AV_LOG_WARNING, "VIDIOC_G_FMT ioctl\n");
return ret;
}


avctx->pix_fmt = ff_v4l2_format_v4l2_to_avfmt(capture->format.fmt.pix_mp.pixelformat, AV_CODEC_ID_RAWVIDEO);
capture->av_pix_fmt = avctx->pix_fmt;


selection.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
selection.r.height = avctx->coded_height;
selection.r.width = avctx->coded_width;
ret = ioctl(s->fd, VIDIOC_S_SELECTION, &selection);
if (!ret) {
ret = ioctl(s->fd, VIDIOC_G_SELECTION, &selection);
if (ret) {
av_log(avctx, AV_LOG_WARNING, "VIDIOC_G_SELECTION ioctl\n");
} else {
av_log(avctx, AV_LOG_DEBUG, "crop output %dx%d\n", selection.r.width, selection.r.height);

capture->height = selection.r.height;
capture->width = selection.r.width;
}
}


if (!capture->buffers) {
ret = ff_v4l2_context_init(capture);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "can't request capture buffers\n");
return AVERROR(ENOMEM);
}
}


ret = ff_v4l2_context_set_status(capture, VIDIOC_STREAMON);
if (ret) {
av_log(avctx, AV_LOG_DEBUG, "VIDIOC_STREAMON, on capture context\n");
return ret;
}

return 0;
}

static int v4l2_prepare_decoder(V4L2m2mContext *s)
{
struct v4l2_event_subscription sub;
V4L2Context *output = &s->output;
int ret;




memset(&sub, 0, sizeof(sub));
sub.type = V4L2_EVENT_SOURCE_CHANGE;
ret = ioctl(s->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
if ( ret < 0) {
if (output->height == 0 || output->width == 0) {
av_log(s->avctx, AV_LOG_ERROR,
"the v4l2 driver does not support VIDIOC_SUBSCRIBE_EVENT\n"
"you must provide codec_height and codec_width on input\n");
return ret;
}
}

return 0;
}

static int v4l2_receive_frame(AVCodecContext *avctx, AVFrame *frame)
{
V4L2m2mContext *s = ((V4L2m2mPriv*)avctx->priv_data)->context;
V4L2Context *const capture = &s->capture;
V4L2Context *const output = &s->output;
AVPacket avpkt = {0};
int ret;

if (s->buf_pkt.size) {
avpkt = s->buf_pkt;
memset(&s->buf_pkt, 0, sizeof(AVPacket));
} else {
ret = ff_decode_get_packet(avctx, &avpkt);
if (ret < 0 && ret != AVERROR_EOF)
return ret;
}

if (s->draining)
goto dequeue;

ret = ff_v4l2_context_enqueue_packet(output, &avpkt);
if (ret < 0) {
if (ret != AVERROR(EAGAIN))
return ret;

s->buf_pkt = avpkt;

}

if (avpkt.size) {
ret = v4l2_try_start(avctx);
if (ret) {
av_packet_unref(&avpkt);


if (ret == AVERROR(ENOMEM))
return ret;

return 0;
}
}

dequeue:
if (!s->buf_pkt.size)
av_packet_unref(&avpkt);
return ff_v4l2_context_dequeue_frame(capture, frame, -1);
}

static av_cold int v4l2_decode_init(AVCodecContext *avctx)
{
V4L2Context *capture, *output;
V4L2m2mContext *s;
V4L2m2mPriv *priv = avctx->priv_data;
int ret;

ret = ff_v4l2_m2m_create_context(priv, &s);
if (ret < 0)
return ret;

capture = &s->capture;
output = &s->output;





output->height = capture->height = avctx->coded_height;
output->width = capture->width = avctx->coded_width;

output->av_codec_id = avctx->codec_id;
output->av_pix_fmt = AV_PIX_FMT_NONE;

capture->av_codec_id = AV_CODEC_ID_RAWVIDEO;
capture->av_pix_fmt = avctx->pix_fmt;

s->avctx = avctx;
ret = ff_v4l2_m2m_codec_init(priv);
if (ret) {
av_log(avctx, AV_LOG_ERROR, "can't configure decoder\n");
s->self_ref = NULL;
av_buffer_unref(&priv->context_ref);

return ret;
}

return v4l2_prepare_decoder(s);
}

static av_cold int v4l2_decode_close(AVCodecContext *avctx)
{
V4L2m2mPriv *priv = avctx->priv_data;
V4L2m2mContext *s = priv->context;
av_packet_unref(&s->buf_pkt);
return ff_v4l2_m2m_codec_end(priv);
}

#define OFFSET(x) offsetof(V4L2m2mPriv, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
V4L_M2M_DEFAULT_OPTS,
{ "num_capture_buffers", "Number of buffers in the capture context",
OFFSET(num_capture_buffers), AV_OPT_TYPE_INT, {.i64 = 20}, 20, INT_MAX, FLAGS },
{ NULL},
};

#define M2MDEC_CLASS(NAME) static const AVClass v4l2_m2m_ ##NAME ##_dec_class = { .class_name = #NAME "_v4l2m2m_decoder", .item_name = av_default_item_name, .option = options, .version = LIBAVUTIL_VERSION_INT, };







#define M2MDEC(NAME, LONGNAME, CODEC, bsf_name) M2MDEC_CLASS(NAME) AVCodec ff_ ##NAME ##_v4l2m2m_decoder = { .name = #NAME "_v4l2m2m" , .long_name = NULL_IF_CONFIG_SMALL("V4L2 mem2mem " LONGNAME " decoder wrapper"), .type = AVMEDIA_TYPE_VIDEO, .id = CODEC , .priv_data_size = sizeof(V4L2m2mPriv), .priv_class = &v4l2_m2m_ ##NAME ##_dec_class, .init = v4l2_decode_init, .receive_frame = v4l2_receive_frame, .close = v4l2_decode_close, .bsfs = bsf_name, .capabilities = AV_CODEC_CAP_HARDWARE | AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AVOID_PROBING, .caps_internal = FF_CODEC_CAP_SETS_PKT_DTS, .wrapper_name = "v4l2m2m", }

















M2MDEC(h264, "H.264", AV_CODEC_ID_H264, "h264_mp4toannexb");
M2MDEC(hevc, "HEVC", AV_CODEC_ID_HEVC, "hevc_mp4toannexb");
M2MDEC(mpeg1, "MPEG1", AV_CODEC_ID_MPEG1VIDEO, NULL);
M2MDEC(mpeg2, "MPEG2", AV_CODEC_ID_MPEG2VIDEO, NULL);
M2MDEC(mpeg4, "MPEG4", AV_CODEC_ID_MPEG4, NULL);
M2MDEC(h263, "H.263", AV_CODEC_ID_H263, NULL);
M2MDEC(vc1 , "VC1", AV_CODEC_ID_VC1, NULL);
M2MDEC(vp8, "VP8", AV_CODEC_ID_VP8, NULL);
M2MDEC(vp9, "VP9", AV_CODEC_ID_VP9, NULL);
