#include "libavutil/avassert.h"
#include "avcodec.h"
#include "internal.h"
#include "libavutil/common.h"
typedef struct EightSvxContext {
uint8_t fib_acc[2];
const int8_t *table;
uint8_t *data[2];
int data_size;
int data_idx;
} EightSvxContext;
static const int8_t fibonacci[16] = { -34, -21, -13, -8, -5, -3, -2, -1, 0, 1, 2, 3, 5, 8, 13, 21 };
static const int8_t exponential[16] = { -128, -64, -32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 32, 64 };
#define MAX_FRAME_SIZE 2048
static void delta_decode(uint8_t *dst, const uint8_t *src, int src_size,
uint8_t *state, const int8_t *table)
{
uint8_t val = *state;
while (src_size--) {
uint8_t d = *src++;
val = av_clip_uint8(val + table[d & 0xF]);
*dst++ = val;
val = av_clip_uint8(val + table[d >> 4]);
*dst++ = val;
}
*state = val;
}
static int eightsvx_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
EightSvxContext *esc = avctx->priv_data;
AVFrame *frame = data;
int buf_size;
int ch, ret;
int hdr_size = 2;
if (!esc->data[0] && avpkt) {
int chan_size = avpkt->size / avctx->channels - hdr_size;
if (avpkt->size % avctx->channels) {
av_log(avctx, AV_LOG_WARNING, "Packet with odd size, ignoring last byte\n");
}
if (avpkt->size < (hdr_size + 1) * avctx->channels) {
av_log(avctx, AV_LOG_ERROR, "packet size is too small\n");
return AVERROR_INVALIDDATA;
}
esc->fib_acc[0] = avpkt->data[1] + 128;
if (avctx->channels == 2)
esc->fib_acc[1] = avpkt->data[2+chan_size+1] + 128;
esc->data_idx = 0;
esc->data_size = chan_size;
if (!(esc->data[0] = av_malloc(chan_size)))
return AVERROR(ENOMEM);
if (avctx->channels == 2) {
if (!(esc->data[1] = av_malloc(chan_size))) {
av_freep(&esc->data[0]);
return AVERROR(ENOMEM);
}
}
memcpy(esc->data[0], &avpkt->data[hdr_size], chan_size);
if (avctx->channels == 2)
memcpy(esc->data[1], &avpkt->data[2*hdr_size+chan_size], chan_size);
}
if (!esc->data[0]) {
av_log(avctx, AV_LOG_ERROR, "unexpected empty packet\n");
return AVERROR_INVALIDDATA;
}
buf_size = FFMIN(MAX_FRAME_SIZE, esc->data_size - esc->data_idx);
if (buf_size <= 0) {
*got_frame_ptr = 0;
return avpkt->size;
}
frame->nb_samples = buf_size * 2;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
for (ch = 0; ch < avctx->channels; ch++) {
delta_decode(frame->data[ch], &esc->data[ch][esc->data_idx],
buf_size, &esc->fib_acc[ch], esc->table);
}
esc->data_idx += buf_size;
*got_frame_ptr = 1;
return ((avctx->frame_number == 0)*hdr_size + buf_size)*avctx->channels;
}
static av_cold int eightsvx_decode_init(AVCodecContext *avctx)
{
EightSvxContext *esc = avctx->priv_data;
if (avctx->channels < 1 || avctx->channels > 2) {
av_log(avctx, AV_LOG_ERROR, "8SVX does not support more than 2 channels\n");
return AVERROR_INVALIDDATA;
}
switch (avctx->codec->id) {
case AV_CODEC_ID_8SVX_FIB: esc->table = fibonacci; break;
case AV_CODEC_ID_8SVX_EXP: esc->table = exponential; break;
default:
av_assert1(0);
}
avctx->sample_fmt = AV_SAMPLE_FMT_U8P;
return 0;
}
static av_cold int eightsvx_decode_close(AVCodecContext *avctx)
{
EightSvxContext *esc = avctx->priv_data;
av_freep(&esc->data[0]);
av_freep(&esc->data[1]);
esc->data_size = 0;
esc->data_idx = 0;
return 0;
}
#if CONFIG_EIGHTSVX_FIB_DECODER
AVCodec ff_eightsvx_fib_decoder = {
.name = "8svx_fib",
.long_name = NULL_IF_CONFIG_SMALL("8SVX fibonacci"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_8SVX_FIB,
.priv_data_size = sizeof (EightSvxContext),
.init = eightsvx_decode_init,
.decode = eightsvx_decode_frame,
.close = eightsvx_decode_close,
.capabilities = AV_CODEC_CAP_DR1,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_U8P,
AV_SAMPLE_FMT_NONE },
};
#endif
#if CONFIG_EIGHTSVX_EXP_DECODER
AVCodec ff_eightsvx_exp_decoder = {
.name = "8svx_exp",
.long_name = NULL_IF_CONFIG_SMALL("8SVX exponential"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_8SVX_EXP,
.priv_data_size = sizeof (EightSvxContext),
.init = eightsvx_decode_init,
.decode = eightsvx_decode_frame,
.close = eightsvx_decode_close,
.capabilities = AV_CODEC_CAP_DR1,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_U8P,
AV_SAMPLE_FMT_NONE },
};
#endif
