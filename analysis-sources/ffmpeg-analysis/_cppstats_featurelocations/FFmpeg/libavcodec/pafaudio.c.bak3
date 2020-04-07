




















#include "libavutil/intreadwrite.h"

#include "avcodec.h"
#include "internal.h"
#include "mathops.h"
#include "paf.h"

static av_cold int paf_audio_init(AVCodecContext *avctx)
{
if (avctx->channels != 2) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels\n");
return AVERROR_INVALIDDATA;
}

avctx->channel_layout = AV_CH_LAYOUT_STEREO;
avctx->sample_fmt = AV_SAMPLE_FMT_S16;

return 0;
}

static int paf_audio_decode(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *pkt)
{
AVFrame *frame = data;
int16_t *output_samples;
const uint8_t *src = pkt->data;
int frames, ret, i, j;
int16_t cb[256];

frames = pkt->size / PAF_SOUND_FRAME_SIZE;
if (frames < 1)
return AVERROR_INVALIDDATA;

frame->nb_samples = PAF_SOUND_SAMPLES * frames;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

output_samples = (int16_t *)frame->data[0];

for (j = 0; j < frames; j++) {
for (i = 0; i < 256; i++)
cb[i] = sign_extend(AV_RL16(src + i * 2), 16);
src += 256 * 2;

for (i = 0; i < PAF_SOUND_SAMPLES * 2; i++)
*output_samples++ = cb[*src++];
}
*got_frame = 1;

return pkt->size;
}

AVCodec ff_paf_audio_decoder = {
.name = "paf_audio",
.long_name = NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Audio"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_PAF_AUDIO,
.init = paf_audio_init,
.decode = paf_audio_decode,
.capabilities = AV_CODEC_CAP_DR1,
};
