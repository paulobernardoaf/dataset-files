#include "avcodec.h"
#include "decode.h"
#include "internal.h"
#include "libavutil/internal.h"
#include "libavutil/frame.h"
#include "libavutil/buffer.h"
#include "libavutil/pixdesc.h"
static void wrapped_avframe_release_buffer(void *unused, uint8_t *data)
{
AVFrame *frame = (AVFrame *)data;
av_frame_free(&frame);
}
static int wrapped_avframe_encode(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
AVFrame *wrapped = av_frame_clone(frame);
uint8_t *data;
int size = sizeof(*wrapped) + AV_INPUT_BUFFER_PADDING_SIZE;
if (!wrapped)
return AVERROR(ENOMEM);
data = av_mallocz(size);
if (!data) {
av_frame_free(&wrapped);
return AVERROR(ENOMEM);
}
pkt->buf = av_buffer_create(data, size,
wrapped_avframe_release_buffer, NULL,
AV_BUFFER_FLAG_READONLY);
if (!pkt->buf) {
av_frame_free(&wrapped);
av_freep(&data);
return AVERROR(ENOMEM);
}
av_frame_move_ref((AVFrame*)data, wrapped);
av_frame_free(&wrapped);
pkt->data = data;
pkt->size = sizeof(*wrapped);
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
static int wrapped_avframe_decode(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *pkt)
{
AVFrame *in, *out;
int err;
if (!(pkt->flags & AV_PKT_FLAG_TRUSTED)) {
return AVERROR(EPERM);
}
if (pkt->size < sizeof(AVFrame))
return AVERROR(EINVAL);
in = (AVFrame*)pkt->data;
out = data;
err = ff_decode_frame_props(avctx, out);
if (err < 0)
return err;
av_frame_move_ref(out, in);
err = ff_attach_decode_data(out);
if (err < 0) {
av_frame_unref(out);
return err;
}
*got_frame = 1;
return 0;
}
AVCodec ff_wrapped_avframe_encoder = {
.name = "wrapped_avframe",
.long_name = NULL_IF_CONFIG_SMALL("AVFrame to AVPacket passthrough"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_WRAPPED_AVFRAME,
.encode2 = wrapped_avframe_encode,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
AVCodec ff_wrapped_avframe_decoder = {
.name = "wrapped_avframe",
.long_name = NULL_IF_CONFIG_SMALL("AVPacket to AVFrame passthrough"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_WRAPPED_AVFRAME,
.decode = wrapped_avframe_decode,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
