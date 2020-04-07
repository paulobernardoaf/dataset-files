


















































































#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>

#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include "audio_frame_queue.h"


typedef struct LibSpeexEncContext {
AVClass *class; 
SpeexBits bits; 
SpeexHeader header; 
void *enc_state; 
int frames_per_packet; 
float vbr_quality; 
int cbr_quality; 
int abr; 
int vad; 
int dtx; 
int pkt_frame_count; 
AudioFrameQueue afq; 
} LibSpeexEncContext;

static av_cold void print_enc_params(AVCodecContext *avctx,
LibSpeexEncContext *s)
{
const char *mode_str = "unknown";

av_log(avctx, AV_LOG_DEBUG, "channels: %d\n", avctx->channels);
switch (s->header.mode) {
case SPEEX_MODEID_NB: mode_str = "narrowband"; break;
case SPEEX_MODEID_WB: mode_str = "wideband"; break;
case SPEEX_MODEID_UWB: mode_str = "ultra-wideband"; break;
}
av_log(avctx, AV_LOG_DEBUG, "mode: %s\n", mode_str);
if (s->header.vbr) {
av_log(avctx, AV_LOG_DEBUG, "rate control: VBR\n");
av_log(avctx, AV_LOG_DEBUG, " quality: %f\n", s->vbr_quality);
} else if (s->abr) {
av_log(avctx, AV_LOG_DEBUG, "rate control: ABR\n");
av_log(avctx, AV_LOG_DEBUG, " bitrate: %"PRId64" bps\n", avctx->bit_rate);
} else {
av_log(avctx, AV_LOG_DEBUG, "rate control: CBR\n");
av_log(avctx, AV_LOG_DEBUG, " bitrate: %"PRId64" bps\n", avctx->bit_rate);
}
av_log(avctx, AV_LOG_DEBUG, "complexity: %d\n",
avctx->compression_level);
av_log(avctx, AV_LOG_DEBUG, "frame size: %d samples\n",
avctx->frame_size);
av_log(avctx, AV_LOG_DEBUG, "frames per packet: %d\n",
s->frames_per_packet);
av_log(avctx, AV_LOG_DEBUG, "packet size: %d\n",
avctx->frame_size * s->frames_per_packet);
av_log(avctx, AV_LOG_DEBUG, "voice activity detection: %d\n", s->vad);
av_log(avctx, AV_LOG_DEBUG, "discontinuous transmission: %d\n", s->dtx);
}

static av_cold int encode_init(AVCodecContext *avctx)
{
LibSpeexEncContext *s = avctx->priv_data;
const SpeexMode *mode;
uint8_t *header_data;
int header_size;
int32_t complexity;


if (avctx->channels < 1 || avctx->channels > 2) {
av_log(avctx, AV_LOG_ERROR, "Invalid channels (%d). Only stereo and "
"mono are supported\n", avctx->channels);
return AVERROR(EINVAL);
}


switch (avctx->sample_rate) {
case 8000: mode = speex_lib_get_mode(SPEEX_MODEID_NB); break;
case 16000: mode = speex_lib_get_mode(SPEEX_MODEID_WB); break;
case 32000: mode = speex_lib_get_mode(SPEEX_MODEID_UWB); break;
default:
av_log(avctx, AV_LOG_ERROR, "Sample rate of %d Hz is not supported. "
"Resample to 8, 16, or 32 kHz.\n", avctx->sample_rate);
return AVERROR(EINVAL);
}


s->enc_state = speex_encoder_init(mode);
if (!s->enc_state) {
av_log(avctx, AV_LOG_ERROR, "Error initializing libspeex\n");
return -1;
}
speex_init_header(&s->header, avctx->sample_rate, avctx->channels, mode);


if (avctx->flags & AV_CODEC_FLAG_QSCALE) {

s->header.vbr = 1;
s->vad = 1; 
speex_encoder_ctl(s->enc_state, SPEEX_SET_VBR, &s->header.vbr);
s->vbr_quality = av_clipf(avctx->global_quality / (float)FF_QP2LAMBDA,
0.0f, 10.0f);
speex_encoder_ctl(s->enc_state, SPEEX_SET_VBR_QUALITY, &s->vbr_quality);
} else {
s->header.bitrate = avctx->bit_rate;
if (avctx->bit_rate > 0) {

if (s->abr) {
speex_encoder_ctl(s->enc_state, SPEEX_SET_ABR,
&s->header.bitrate);
speex_encoder_ctl(s->enc_state, SPEEX_GET_ABR,
&s->header.bitrate);
} else {
speex_encoder_ctl(s->enc_state, SPEEX_SET_BITRATE,
&s->header.bitrate);
speex_encoder_ctl(s->enc_state, SPEEX_GET_BITRATE,
&s->header.bitrate);
}
} else {

speex_encoder_ctl(s->enc_state, SPEEX_SET_QUALITY,
&s->cbr_quality);
speex_encoder_ctl(s->enc_state, SPEEX_GET_BITRATE,
&s->header.bitrate);
}


avctx->bit_rate = s->header.bitrate + (avctx->channels == 2 ? 800 : 0);
}


if (s->vad)
speex_encoder_ctl(s->enc_state, SPEEX_SET_VAD, &s->vad);


if (s->dtx) {
speex_encoder_ctl(s->enc_state, SPEEX_SET_DTX, &s->dtx);
if (!(s->abr || s->vad || s->header.vbr))
av_log(avctx, AV_LOG_WARNING, "DTX is not much of use without ABR, VAD or VBR\n");
}


if (avctx->compression_level > FF_COMPRESSION_DEFAULT) {
complexity = av_clip(avctx->compression_level, 0, 10);
speex_encoder_ctl(s->enc_state, SPEEX_SET_COMPLEXITY, &complexity);
}
speex_encoder_ctl(s->enc_state, SPEEX_GET_COMPLEXITY, &complexity);
avctx->compression_level = complexity;


avctx->frame_size = s->header.frame_size;
s->header.frames_per_packet = s->frames_per_packet;


speex_encoder_ctl(s->enc_state, SPEEX_GET_LOOKAHEAD, &avctx->initial_padding);
ff_af_queue_init(avctx, &s->afq);




header_data = speex_header_to_packet(&s->header, &header_size);


avctx->extradata = av_malloc(header_size + AV_INPUT_BUFFER_PADDING_SIZE);
if (!avctx->extradata) {
speex_header_free(header_data);
speex_encoder_destroy(s->enc_state);
av_log(avctx, AV_LOG_ERROR, "memory allocation error\n");
return AVERROR(ENOMEM);
}


memcpy(avctx->extradata, header_data, header_size);
avctx->extradata_size = header_size;
speex_header_free(header_data);


speex_bits_init(&s->bits);

print_enc_params(avctx, s);
return 0;
}

static int encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr)
{
LibSpeexEncContext *s = avctx->priv_data;
int16_t *samples = frame ? (int16_t *)frame->data[0] : NULL;
int ret;

if (samples) {

if (avctx->channels == 2)
speex_encode_stereo_int(samples, s->header.frame_size, &s->bits);
speex_encode_int(s->enc_state, samples, &s->bits);
s->pkt_frame_count++;
if ((ret = ff_af_queue_add(&s->afq, frame)) < 0)
return ret;
} else {

if (!s->pkt_frame_count)
return 0;

while (s->pkt_frame_count < s->frames_per_packet) {
speex_bits_pack(&s->bits, 15, 5);
s->pkt_frame_count++;
}
}


if (s->pkt_frame_count == s->frames_per_packet) {
s->pkt_frame_count = 0;
if ((ret = ff_alloc_packet2(avctx, avpkt, speex_bits_nbytes(&s->bits), 0)) < 0)
return ret;
ret = speex_bits_write(&s->bits, avpkt->data, avpkt->size);
speex_bits_reset(&s->bits);


ff_af_queue_remove(&s->afq, s->frames_per_packet * avctx->frame_size,
&avpkt->pts, &avpkt->duration);

avpkt->size = ret;
*got_packet_ptr = 1;
return 0;
}
return 0;
}

static av_cold int encode_close(AVCodecContext *avctx)
{
LibSpeexEncContext *s = avctx->priv_data;

speex_bits_destroy(&s->bits);
speex_encoder_destroy(s->enc_state);

ff_af_queue_close(&s->afq);
av_freep(&avctx->extradata);

return 0;
}

#define OFFSET(x) offsetof(LibSpeexEncContext, x)
#define AE AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "abr", "Use average bit rate", OFFSET(abr), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, AE },
{ "cbr_quality", "Set quality value (0 to 10) for CBR", OFFSET(cbr_quality), AV_OPT_TYPE_INT, { .i64 = 8 }, 0, 10, AE },
{ "frames_per_packet", "Number of frames to encode in each packet", OFFSET(frames_per_packet), AV_OPT_TYPE_INT, { .i64 = 1 }, 1, 8, AE },
{ "vad", "Voice Activity Detection", OFFSET(vad), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, AE },
{ "dtx", "Discontinuous Transmission", OFFSET(dtx), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, AE },
{ NULL },
};

static const AVClass speex_class = {
.class_name = "libspeex",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

static const AVCodecDefault defaults[] = {
{ "b", "0" },
{ "compression_level", "3" },
{ NULL },
};

AVCodec ff_libspeex_encoder = {
.name = "libspeex",
.long_name = NULL_IF_CONFIG_SMALL("libspeex Speex"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_SPEEX,
.priv_data_size = sizeof(LibSpeexEncContext),
.init = encode_init,
.encode2 = encode_frame,
.close = encode_close,
.capabilities = AV_CODEC_CAP_DELAY,
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_NONE },
.channel_layouts = (const uint64_t[]){ AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
0 },
.supported_samplerates = (const int[]){ 8000, 16000, 32000, 0 },
.priv_class = &speex_class,
.defaults = defaults,
.wrapper_name = "libspeex",
};
