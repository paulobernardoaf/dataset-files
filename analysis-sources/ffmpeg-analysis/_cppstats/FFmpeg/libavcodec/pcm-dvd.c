#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
typedef struct PCMDVDContext {
uint32_t last_header; 
int block_size; 
int last_block_size; 
int samples_per_block; 
int groups_per_block; 
uint8_t *extra_samples; 
int extra_sample_count; 
} PCMDVDContext;
static av_cold int pcm_dvd_decode_init(AVCodecContext *avctx)
{
PCMDVDContext *s = avctx->priv_data;
s->last_header = -1;
if (!(s->extra_samples = av_malloc(8 * 3 * 4)))
return AVERROR(ENOMEM);
return 0;
}
static av_cold int pcm_dvd_decode_uninit(AVCodecContext *avctx)
{
PCMDVDContext *s = avctx->priv_data;
av_freep(&s->extra_samples);
return 0;
}
static int pcm_dvd_parse_header(AVCodecContext *avctx, const uint8_t *header)
{
static const uint32_t frequencies[4] = { 48000, 96000, 44100, 32000 };
PCMDVDContext *s = avctx->priv_data;
int header_int = (header[0] & 0xe0) | (header[1] << 8) | (header[2] << 16);
if (s->last_header == header_int)
return 0;
s->last_header = -1;
if (avctx->debug & FF_DEBUG_PICT_INFO)
av_log(avctx, AV_LOG_DEBUG, "pcm_dvd_parse_header: header = %02x%02x%02x\n",
header[0], header[1], header[2]);
s->extra_sample_count = 0;
avctx->bits_per_coded_sample = 16 + (header[1] >> 6 & 3) * 4;
if (avctx->bits_per_coded_sample == 28) {
av_log(avctx, AV_LOG_ERROR,
"PCM DVD unsupported sample depth %i\n",
avctx->bits_per_coded_sample);
return AVERROR_INVALIDDATA;
}
avctx->sample_fmt = avctx->bits_per_coded_sample == 16 ? AV_SAMPLE_FMT_S16
: AV_SAMPLE_FMT_S32;
avctx->bits_per_raw_sample = avctx->bits_per_coded_sample;
avctx->sample_rate = frequencies[header[1] >> 4 & 3];
avctx->channels = 1 + (header[1] & 7);
avctx->bit_rate = avctx->channels *
avctx->sample_rate *
avctx->bits_per_coded_sample;
if (avctx->bits_per_coded_sample == 16) {
s->samples_per_block = 1;
s->block_size = avctx->channels * 2;
} else {
switch (avctx->channels) {
case 1:
case 2:
case 4:
s->block_size = 4 * avctx->bits_per_coded_sample / 8;
s->samples_per_block = 4 / avctx->channels;
s->groups_per_block = 1;
break;
case 8:
s->block_size = 8 * avctx->bits_per_coded_sample / 8;
s->samples_per_block = 1;
s->groups_per_block = 2;
break;
default:
s->block_size = 4 * avctx->channels *
avctx->bits_per_coded_sample / 8;
s->samples_per_block = 4;
s->groups_per_block = avctx->channels;
break;
}
}
if (avctx->debug & FF_DEBUG_PICT_INFO)
ff_dlog(avctx,
"pcm_dvd_parse_header: %d channels, %d bits per sample, %d Hz, %"PRId64" bit/s\n",
avctx->channels, avctx->bits_per_coded_sample,
avctx->sample_rate, avctx->bit_rate);
s->last_header = header_int;
return 0;
}
static void *pcm_dvd_decode_samples(AVCodecContext *avctx, const uint8_t *src,
void *dst, int blocks)
{
PCMDVDContext *s = avctx->priv_data;
int16_t *dst16 = dst;
int32_t *dst32 = dst;
GetByteContext gb;
int i;
uint8_t t;
bytestream2_init(&gb, src, blocks * s->block_size);
switch (avctx->bits_per_coded_sample) {
case 16: {
#if HAVE_BIGENDIAN
bytestream2_get_buffer(&gb, dst16, blocks * s->block_size);
dst16 += blocks * s->block_size / 2;
#else
int samples = blocks * avctx->channels;
do {
*dst16++ = bytestream2_get_be16u(&gb);
} while (--samples);
#endif
return dst16;
}
case 20:
if (avctx->channels == 1) {
do {
for (i = 2; i; i--) {
dst32[0] = bytestream2_get_be16u(&gb) << 16;
dst32[1] = bytestream2_get_be16u(&gb) << 16;
t = bytestream2_get_byteu(&gb);
*dst32++ += (t & 0xf0) << 8;
*dst32++ += (t & 0x0f) << 12;
}
} while (--blocks);
} else {
do {
for (i = s->groups_per_block; i; i--) {
dst32[0] = bytestream2_get_be16u(&gb) << 16;
dst32[1] = bytestream2_get_be16u(&gb) << 16;
dst32[2] = bytestream2_get_be16u(&gb) << 16;
dst32[3] = bytestream2_get_be16u(&gb) << 16;
t = bytestream2_get_byteu(&gb);
*dst32++ += (t & 0xf0) << 8;
*dst32++ += (t & 0x0f) << 12;
t = bytestream2_get_byteu(&gb);
*dst32++ += (t & 0xf0) << 8;
*dst32++ += (t & 0x0f) << 12;
}
} while (--blocks);
}
return dst32;
case 24:
if (avctx->channels == 1) {
do {
for (i = 2; i; i--) {
dst32[0] = bytestream2_get_be16u(&gb) << 16;
dst32[1] = bytestream2_get_be16u(&gb) << 16;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
}
} while (--blocks);
} else {
do {
for (i = s->groups_per_block; i; i--) {
dst32[0] = bytestream2_get_be16u(&gb) << 16;
dst32[1] = bytestream2_get_be16u(&gb) << 16;
dst32[2] = bytestream2_get_be16u(&gb) << 16;
dst32[3] = bytestream2_get_be16u(&gb) << 16;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
*dst32++ += bytestream2_get_byteu(&gb) << 8;
}
} while (--blocks);
}
return dst32;
default:
return NULL;
}
}
static int pcm_dvd_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *src = avpkt->data;
int buf_size = avpkt->size;
PCMDVDContext *s = avctx->priv_data;
int retval;
int blocks;
void *dst;
if (buf_size < 3) {
av_log(avctx, AV_LOG_ERROR, "PCM packet too small\n");
return AVERROR_INVALIDDATA;
}
if ((retval = pcm_dvd_parse_header(avctx, src)))
return retval;
if (s->last_block_size && s->last_block_size != s->block_size) {
av_log(avctx, AV_LOG_WARNING, "block_size has changed %d != %d\n", s->last_block_size, s->block_size);
s->extra_sample_count = 0;
}
s->last_block_size = s->block_size;
src += 3;
buf_size -= 3;
blocks = (buf_size + s->extra_sample_count) / s->block_size;
frame->nb_samples = blocks * s->samples_per_block;
if ((retval = ff_get_buffer(avctx, frame, 0)) < 0)
return retval;
dst = frame->data[0];
if (s->extra_sample_count) {
int missing_samples = s->block_size - s->extra_sample_count;
if (buf_size >= missing_samples) {
memcpy(s->extra_samples + s->extra_sample_count, src,
missing_samples);
dst = pcm_dvd_decode_samples(avctx, s->extra_samples, dst, 1);
src += missing_samples;
buf_size -= missing_samples;
s->extra_sample_count = 0;
blocks--;
} else {
memcpy(s->extra_samples + s->extra_sample_count, src, buf_size);
s->extra_sample_count += buf_size;
return avpkt->size;
}
}
if (blocks) {
pcm_dvd_decode_samples(avctx, src, dst, blocks);
buf_size -= blocks * s->block_size;
}
if (buf_size) {
src += blocks * s->block_size;
memcpy(s->extra_samples, src, buf_size);
s->extra_sample_count = buf_size;
}
*got_frame_ptr = 1;
return avpkt->size;
}
AVCodec ff_pcm_dvd_decoder = {
.name = "pcm_dvd",
.long_name = NULL_IF_CONFIG_SMALL("PCM signed 16|20|24-bit big-endian for DVD media"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_PCM_DVD,
.priv_data_size = sizeof(PCMDVDContext),
.init = pcm_dvd_decode_init,
.decode = pcm_dvd_decode_frame,
.close = pcm_dvd_decode_uninit,
.capabilities = AV_CODEC_CAP_DR1,
.sample_fmts = (const enum AVSampleFormat[]) {
AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S32, AV_SAMPLE_FMT_NONE
}
};
