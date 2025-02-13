#include "libavutil/channel_layout.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
static int pcm_bluray_parse_header(AVCodecContext *avctx,
const uint8_t *header)
{
static const uint8_t bits_per_samples[4] = { 0, 16, 20, 24 };
static const uint32_t channel_layouts[16] = {
0, AV_CH_LAYOUT_MONO, 0, AV_CH_LAYOUT_STEREO, AV_CH_LAYOUT_SURROUND,
AV_CH_LAYOUT_2_1, AV_CH_LAYOUT_4POINT0, AV_CH_LAYOUT_2_2,
AV_CH_LAYOUT_5POINT0, AV_CH_LAYOUT_5POINT1, AV_CH_LAYOUT_7POINT0,
AV_CH_LAYOUT_7POINT1, 0, 0, 0, 0
};
static const uint8_t channels[16] = {
0, 1, 0, 2, 3, 3, 4, 4, 5, 6, 7, 8, 0, 0, 0, 0
};
uint8_t channel_layout = header[2] >> 4;
if (avctx->debug & FF_DEBUG_PICT_INFO)
ff_dlog(avctx, "pcm_bluray_parse_header: header = %02x%02x%02x%02x\n",
header[0], header[1], header[2], header[3]);
avctx->bits_per_coded_sample = bits_per_samples[header[3] >> 6];
if (!(avctx->bits_per_coded_sample == 16 || avctx->bits_per_coded_sample == 24)) {
av_log(avctx, AV_LOG_ERROR, "unsupported sample depth (%d)\n", avctx->bits_per_coded_sample);
return AVERROR_INVALIDDATA;
}
avctx->sample_fmt = avctx->bits_per_coded_sample == 16 ? AV_SAMPLE_FMT_S16
: AV_SAMPLE_FMT_S32;
if (avctx->sample_fmt == AV_SAMPLE_FMT_S32)
avctx->bits_per_raw_sample = avctx->bits_per_coded_sample;
switch (header[2] & 0x0f) {
case 1:
avctx->sample_rate = 48000;
break;
case 4:
avctx->sample_rate = 96000;
break;
case 5:
avctx->sample_rate = 192000;
break;
default:
avctx->sample_rate = 0;
av_log(avctx, AV_LOG_ERROR, "reserved sample rate (%d)\n",
header[2] & 0x0f);
return AVERROR_INVALIDDATA;
}
avctx->channel_layout = channel_layouts[channel_layout];
avctx->channels = channels[channel_layout];
if (!avctx->channels) {
av_log(avctx, AV_LOG_ERROR, "reserved channel configuration (%d)\n",
channel_layout);
return AVERROR_INVALIDDATA;
}
avctx->bit_rate = FFALIGN(avctx->channels, 2) * avctx->sample_rate *
avctx->bits_per_coded_sample;
if (avctx->debug & FF_DEBUG_PICT_INFO)
ff_dlog(avctx,
"pcm_bluray_parse_header: %d channels, %d bits per sample, %d Hz, %"PRId64" bit/s\n",
avctx->channels, avctx->bits_per_coded_sample,
avctx->sample_rate, avctx->bit_rate);
return 0;
}
static int pcm_bluray_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *src = avpkt->data;
int buf_size = avpkt->size;
GetByteContext gb;
int num_source_channels, channel, retval;
int sample_size, samples;
int16_t *dst16;
int32_t *dst32;
if (buf_size < 4) {
av_log(avctx, AV_LOG_ERROR, "PCM packet too small\n");
return AVERROR_INVALIDDATA;
}
if ((retval = pcm_bluray_parse_header(avctx, src)))
return retval;
src += 4;
buf_size -= 4;
bytestream2_init(&gb, src, buf_size);
num_source_channels = FFALIGN(avctx->channels, 2);
sample_size = (num_source_channels *
(avctx->sample_fmt == AV_SAMPLE_FMT_S16 ? 16 : 24)) >> 3;
samples = buf_size / sample_size;
frame->nb_samples = samples;
if ((retval = ff_get_buffer(avctx, frame, 0)) < 0)
return retval;
dst16 = (int16_t *)frame->data[0];
dst32 = (int32_t *)frame->data[0];
if (samples) {
switch (avctx->channel_layout) {
case AV_CH_LAYOUT_STEREO:
case AV_CH_LAYOUT_4POINT0:
case AV_CH_LAYOUT_2_2:
samples *= num_source_channels;
if (AV_SAMPLE_FMT_S16 == avctx->sample_fmt) {
#if HAVE_BIGENDIAN
bytestream2_get_buffer(&gb, dst16, buf_size);
#else
do {
*dst16++ = bytestream2_get_be16u(&gb);
} while (--samples);
#endif
} else {
do {
*dst32++ = bytestream2_get_be24u(&gb) << 8;
} while (--samples);
}
break;
case AV_CH_LAYOUT_MONO:
case AV_CH_LAYOUT_SURROUND:
case AV_CH_LAYOUT_2_1:
case AV_CH_LAYOUT_5POINT0:
if (AV_SAMPLE_FMT_S16 == avctx->sample_fmt) {
do {
#if HAVE_BIGENDIAN
bytestream2_get_buffer(&gb, dst16, avctx->channels * 2);
dst16 += avctx->channels;
#else
channel = avctx->channels;
do {
*dst16++ = bytestream2_get_be16u(&gb);
} while (--channel);
#endif
bytestream2_skip(&gb, 2);
} while (--samples);
} else {
do {
channel = avctx->channels;
do {
*dst32++ = bytestream2_get_be24u(&gb) << 8;
} while (--channel);
bytestream2_skip(&gb, 3);
} while (--samples);
}
break;
case AV_CH_LAYOUT_5POINT1:
if (AV_SAMPLE_FMT_S16 == avctx->sample_fmt) {
do {
dst16[0] = bytestream2_get_be16u(&gb);
dst16[1] = bytestream2_get_be16u(&gb);
dst16[2] = bytestream2_get_be16u(&gb);
dst16[4] = bytestream2_get_be16u(&gb);
dst16[5] = bytestream2_get_be16u(&gb);
dst16[3] = bytestream2_get_be16u(&gb);
dst16 += 6;
} while (--samples);
} else {
do {
dst32[0] = bytestream2_get_be24u(&gb) << 8;
dst32[1] = bytestream2_get_be24u(&gb) << 8;
dst32[2] = bytestream2_get_be24u(&gb) << 8;
dst32[4] = bytestream2_get_be24u(&gb) << 8;
dst32[5] = bytestream2_get_be24u(&gb) << 8;
dst32[3] = bytestream2_get_be24u(&gb) << 8;
dst32 += 6;
} while (--samples);
}
break;
case AV_CH_LAYOUT_7POINT0:
if (AV_SAMPLE_FMT_S16 == avctx->sample_fmt) {
do {
dst16[0] = bytestream2_get_be16u(&gb);
dst16[1] = bytestream2_get_be16u(&gb);
dst16[2] = bytestream2_get_be16u(&gb);
dst16[5] = bytestream2_get_be16u(&gb);
dst16[3] = bytestream2_get_be16u(&gb);
dst16[4] = bytestream2_get_be16u(&gb);
dst16[6] = bytestream2_get_be16u(&gb);
dst16 += 7;
bytestream2_skip(&gb, 2);
} while (--samples);
} else {
do {
dst32[0] = bytestream2_get_be24u(&gb) << 8;
dst32[1] = bytestream2_get_be24u(&gb) << 8;
dst32[2] = bytestream2_get_be24u(&gb) << 8;
dst32[5] = bytestream2_get_be24u(&gb) << 8;
dst32[3] = bytestream2_get_be24u(&gb) << 8;
dst32[4] = bytestream2_get_be24u(&gb) << 8;
dst32[6] = bytestream2_get_be24u(&gb) << 8;
dst32 += 7;
bytestream2_skip(&gb, 3);
} while (--samples);
}
break;
case AV_CH_LAYOUT_7POINT1:
if (AV_SAMPLE_FMT_S16 == avctx->sample_fmt) {
do {
dst16[0] = bytestream2_get_be16u(&gb);
dst16[1] = bytestream2_get_be16u(&gb);
dst16[2] = bytestream2_get_be16u(&gb);
dst16[6] = bytestream2_get_be16u(&gb);
dst16[4] = bytestream2_get_be16u(&gb);
dst16[5] = bytestream2_get_be16u(&gb);
dst16[7] = bytestream2_get_be16u(&gb);
dst16[3] = bytestream2_get_be16u(&gb);
dst16 += 8;
} while (--samples);
} else {
do {
dst32[0] = bytestream2_get_be24u(&gb) << 8;
dst32[1] = bytestream2_get_be24u(&gb) << 8;
dst32[2] = bytestream2_get_be24u(&gb) << 8;
dst32[6] = bytestream2_get_be24u(&gb) << 8;
dst32[4] = bytestream2_get_be24u(&gb) << 8;
dst32[5] = bytestream2_get_be24u(&gb) << 8;
dst32[7] = bytestream2_get_be24u(&gb) << 8;
dst32[3] = bytestream2_get_be24u(&gb) << 8;
dst32 += 8;
} while (--samples);
}
break;
}
}
*got_frame_ptr = 1;
retval = bytestream2_tell(&gb);
if (avctx->debug & FF_DEBUG_BITSTREAM)
ff_dlog(avctx, "pcm_bluray_decode_frame: decoded %d -> %d bytes\n",
retval, buf_size);
return retval + 4;
}
AVCodec ff_pcm_bluray_decoder = {
.name = "pcm_bluray",
.long_name = NULL_IF_CONFIG_SMALL("PCM signed 16|20|24-bit big-endian for Blu-ray media"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_PCM_BLURAY,
.decode = pcm_bluray_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.sample_fmts = (const enum AVSampleFormat[]){
AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_NONE
},
};
