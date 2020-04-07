



























#include "libavcodec/internal.h"
#include "libavcodec/mathops.h"
#include "avcodec.h"
#include "dsd.h"

#define DSD_SILENCE 0x69






static av_cold int decode_init(AVCodecContext *avctx)
{
DSDContext * s;
int i;
uint8_t silence;

ff_init_dsd_data();

s = av_malloc_array(sizeof(DSDContext), avctx->channels);
if (!s)
return AVERROR(ENOMEM);

silence = avctx->codec_id == AV_CODEC_ID_DSD_LSBF || avctx->codec_id == AV_CODEC_ID_DSD_LSBF_PLANAR ? ff_reverse[DSD_SILENCE] : DSD_SILENCE;
for (i = 0; i < avctx->channels; i++) {
s[i].pos = 0;
memset(s[i].buf, silence, sizeof(s[i].buf));
}

avctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
avctx->priv_data = s;
return 0;
}

typedef struct ThreadData {
AVFrame *frame;
AVPacket *avpkt;
} ThreadData;

static int dsd_channel(AVCodecContext *avctx, void *tdata, int j, int threadnr)
{
int lsbf = avctx->codec_id == AV_CODEC_ID_DSD_LSBF || avctx->codec_id == AV_CODEC_ID_DSD_LSBF_PLANAR;
DSDContext *s = avctx->priv_data;
ThreadData *td = tdata;
AVFrame *frame = td->frame;
AVPacket *avpkt = td->avpkt;
int src_next, src_stride;
float *dst = ((float **)frame->extended_data)[j];

if (avctx->codec_id == AV_CODEC_ID_DSD_LSBF_PLANAR || avctx->codec_id == AV_CODEC_ID_DSD_MSBF_PLANAR) {
src_next = frame->nb_samples;
src_stride = 1;
} else {
src_next = 1;
src_stride = avctx->channels;
}

ff_dsd2pcm_translate(&s[j], frame->nb_samples, lsbf,
avpkt->data + j * src_next, src_stride,
dst, 1);

return 0;
}

static int decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
ThreadData td;
AVFrame *frame = data;
int ret;

frame->nb_samples = avpkt->size / avctx->channels;

if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

td.frame = frame;
td.avpkt = avpkt;
avctx->execute2(avctx, dsd_channel, &td, NULL, avctx->channels);

*got_frame_ptr = 1;
return frame->nb_samples * avctx->channels;
}

#define DSD_DECODER(id_, name_, long_name_) AVCodec ff_##name_##_decoder = { .name = #name_, .long_name = NULL_IF_CONFIG_SMALL(long_name_), .type = AVMEDIA_TYPE_AUDIO, .id = AV_CODEC_ID_##id_, .init = decode_init, .decode = decode_frame, .capabilities = AV_CODEC_CAP_DR1 | AV_CODEC_CAP_SLICE_THREADS, .sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE }, };












DSD_DECODER(DSD_LSBF, dsd_lsbf, "DSD (Direct Stream Digital), least significant bit first")
DSD_DECODER(DSD_MSBF, dsd_msbf, "DSD (Direct Stream Digital), most significant bit first")
DSD_DECODER(DSD_MSBF_PLANAR, dsd_msbf_planar, "DSD (Direct Stream Digital), most significant bit first, planar")
DSD_DECODER(DSD_LSBF_PLANAR, dsd_lsbf_planar, "DSD (Direct Stream Digital), least significant bit first, planar")
