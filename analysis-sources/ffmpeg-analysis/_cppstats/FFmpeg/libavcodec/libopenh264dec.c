#include <wels/codec_api.h>
#include <wels/codec_ver.h>
#include "libavutil/common.h"
#include "libavutil/fifo.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "libopenh264.h"
typedef struct SVCContext {
ISVCDecoder *decoder;
} SVCContext;
static av_cold int svc_decode_close(AVCodecContext *avctx)
{
SVCContext *s = avctx->priv_data;
if (s->decoder)
WelsDestroyDecoder(s->decoder);
return 0;
}
static av_cold int svc_decode_init(AVCodecContext *avctx)
{
SVCContext *s = avctx->priv_data;
SDecodingParam param = { 0 };
int err;
int log_level;
WelsTraceCallback callback_function;
if ((err = ff_libopenh264_check_version(avctx)) < 0)
return err;
if (WelsCreateDecoder(&s->decoder)) {
av_log(avctx, AV_LOG_ERROR, "Unable to create decoder\n");
return AVERROR_UNKNOWN;
}
log_level = WELS_LOG_DETAIL;
callback_function = ff_libopenh264_trace_callback;
(*s->decoder)->SetOption(s->decoder, DECODER_OPTION_TRACE_LEVEL, &log_level);
(*s->decoder)->SetOption(s->decoder, DECODER_OPTION_TRACE_CALLBACK, (void *)&callback_function);
(*s->decoder)->SetOption(s->decoder, DECODER_OPTION_TRACE_CALLBACK_CONTEXT, (void *)&avctx);
#if !OPENH264_VER_AT_LEAST(1, 6)
param.eOutputColorFormat = videoFormatI420;
#endif
param.eEcActiveIdc = ERROR_CON_DISABLE;
param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
if ((*s->decoder)->Initialize(s->decoder, &param) != cmResultSuccess) {
av_log(avctx, AV_LOG_ERROR, "Initialize failed\n");
return AVERROR_UNKNOWN;
}
avctx->pix_fmt = AV_PIX_FMT_YUV420P;
return 0;
}
static int svc_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
SVCContext *s = avctx->priv_data;
SBufferInfo info = { 0 };
uint8_t* ptrs[3];
int ret, linesize[3];
AVFrame *avframe = data;
DECODING_STATE state;
#if OPENH264_VER_AT_LEAST(1, 7)
int opt;
#endif
if (!avpkt->data) {
#if OPENH264_VER_AT_LEAST(1, 9)
int end_of_stream = 1;
(*s->decoder)->SetOption(s->decoder, DECODER_OPTION_END_OF_STREAM, &end_of_stream);
state = (*s->decoder)->FlushFrame(s->decoder, ptrs, &info);
#else
return 0;
#endif
} else {
info.uiInBsTimeStamp = avpkt->pts;
#if OPENH264_VER_AT_LEAST(1, 4)
state = (*s->decoder)->DecodeFrameNoDelay(s->decoder, avpkt->data, avpkt->size, ptrs, &info);
#else
state = (*s->decoder)->DecodeFrame2(s->decoder, avpkt->data, avpkt->size, ptrs, &info);
#endif
}
if (state != dsErrorFree) {
av_log(avctx, AV_LOG_ERROR, "DecodeFrame failed\n");
return AVERROR_UNKNOWN;
}
if (info.iBufferStatus != 1) {
av_log(avctx, AV_LOG_DEBUG, "No frame produced\n");
return avpkt->size;
}
ret = ff_set_dimensions(avctx, info.UsrData.sSystemBuffer.iWidth, info.UsrData.sSystemBuffer.iHeight);
if (ret < 0)
return ret;
if (ff_get_buffer(avctx, avframe, 0) < 0) {
av_log(avctx, AV_LOG_ERROR, "Unable to allocate buffer\n");
return AVERROR(ENOMEM);
}
linesize[0] = info.UsrData.sSystemBuffer.iStride[0];
linesize[1] = linesize[2] = info.UsrData.sSystemBuffer.iStride[1];
av_image_copy(avframe->data, avframe->linesize, (const uint8_t **) ptrs, linesize, avctx->pix_fmt, avctx->width, avctx->height);
avframe->pts = info.uiOutYuvTimeStamp;
avframe->pkt_dts = AV_NOPTS_VALUE;
#if FF_API_PKT_PTS
FF_DISABLE_DEPRECATION_WARNINGS
avframe->pkt_pts = avpkt->pts;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
#if OPENH264_VER_AT_LEAST(1, 7)
(*s->decoder)->GetOption(s->decoder, DECODER_OPTION_PROFILE, &opt);
avctx->profile = opt;
(*s->decoder)->GetOption(s->decoder, DECODER_OPTION_LEVEL, &opt);
avctx->level = opt;
#endif
*got_frame = 1;
return avpkt->size;
}
AVCodec ff_libopenh264_decoder = {
.name = "libopenh264",
.long_name = NULL_IF_CONFIG_SMALL("OpenH264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_H264,
.priv_data_size = sizeof(SVCContext),
.init = svc_decode_init,
.decode = svc_decode_frame,
.close = svc_decode_close,
.capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_SETS_PKT_DTS | FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
.bsfs = "h264_mp4toannexb",
.wrapper_name = "libopenh264",
};
