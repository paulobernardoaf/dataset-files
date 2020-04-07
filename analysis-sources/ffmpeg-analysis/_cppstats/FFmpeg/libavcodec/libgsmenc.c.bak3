




























#include "config.h"
#if HAVE_GSM_H
#include <gsm.h>
#else
#include <gsm/gsm.h>
#endif

#include "libavutil/common.h"

#include "avcodec.h"
#include "internal.h"
#include "gsm.h"

static av_cold int libgsm_encode_close(AVCodecContext *avctx) {
gsm_destroy(avctx->priv_data);
avctx->priv_data = NULL;
return 0;
}

static av_cold int libgsm_encode_init(AVCodecContext *avctx) {
if (avctx->channels > 1) {
av_log(avctx, AV_LOG_ERROR, "Mono required for GSM, got %d channels\n",
avctx->channels);
return -1;
}

if (avctx->sample_rate != 8000) {
av_log(avctx, AV_LOG_ERROR, "Sample rate 8000Hz required for GSM, got %dHz\n",
avctx->sample_rate);
if (avctx->strict_std_compliance > FF_COMPLIANCE_UNOFFICIAL)
return -1;
}
if (avctx->bit_rate != 13000 &&
avctx->bit_rate != 13200 &&
avctx->bit_rate != 0 ) {
av_log(avctx, AV_LOG_ERROR, "Bitrate 13000bps required for GSM, got %"PRId64"bps\n",
avctx->bit_rate);
if (avctx->strict_std_compliance > FF_COMPLIANCE_UNOFFICIAL)
return -1;
}

avctx->priv_data = gsm_create();
if (!avctx->priv_data)
goto error;

switch(avctx->codec_id) {
case AV_CODEC_ID_GSM:
avctx->frame_size = GSM_FRAME_SIZE;
avctx->block_align = GSM_BLOCK_SIZE;
break;
case AV_CODEC_ID_GSM_MS: {
int one = 1;
gsm_option(avctx->priv_data, GSM_OPT_WAV49, &one);
avctx->frame_size = 2*GSM_FRAME_SIZE;
avctx->block_align = GSM_MS_BLOCK_SIZE;
}
}

return 0;
error:
libgsm_encode_close(avctx);
return -1;
}

static int libgsm_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr)
{
int ret;
gsm_signal *samples = (gsm_signal *)frame->data[0];
struct gsm_state *state = avctx->priv_data;

if ((ret = ff_alloc_packet2(avctx, avpkt, avctx->block_align, 0)) < 0)
return ret;

switch(avctx->codec_id) {
case AV_CODEC_ID_GSM:
gsm_encode(state, samples, avpkt->data);
break;
case AV_CODEC_ID_GSM_MS:
gsm_encode(state, samples, avpkt->data);
gsm_encode(state, samples + GSM_FRAME_SIZE, avpkt->data + 32);
}

*got_packet_ptr = 1;
return 0;
}

static const AVCodecDefault libgsm_defaults[] = {
{ "b", "13000" },
{ NULL },
};

#if CONFIG_LIBGSM_ENCODER
AVCodec ff_libgsm_encoder = {
.name = "libgsm",
.long_name = NULL_IF_CONFIG_SMALL("libgsm GSM"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_GSM,
.init = libgsm_encode_init,
.encode2 = libgsm_encode_frame,
.close = libgsm_encode_close,
.defaults = libgsm_defaults,
.channel_layouts= (const uint64_t[]) { AV_CH_LAYOUT_MONO, 0 },
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_NONE },
.wrapper_name = "libgsm",
};
#endif
#if CONFIG_LIBGSM_MS_ENCODER
AVCodec ff_libgsm_ms_encoder = {
.name = "libgsm_ms",
.long_name = NULL_IF_CONFIG_SMALL("libgsm GSM Microsoft variant"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_GSM_MS,
.init = libgsm_encode_init,
.encode2 = libgsm_encode_frame,
.close = libgsm_encode_close,
.defaults = libgsm_defaults,
.channel_layouts= (const uint64_t[]) { AV_CH_LAYOUT_MONO, 0 },
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_NONE },
.wrapper_name = "libgsm",
};
#endif
