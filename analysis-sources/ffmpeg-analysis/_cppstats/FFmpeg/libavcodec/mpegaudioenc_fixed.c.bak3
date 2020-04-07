




















#include "mpegaudioenc_template.c"

AVCodec ff_mp2fixed_encoder = {
.name = "mp2fixed",
.long_name = NULL_IF_CONFIG_SMALL("MP2 fixed point (MPEG audio layer 2)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_MP2,
.priv_data_size = sizeof(MpegAudioContext),
.init = MPA_encode_init,
.encode2 = MPA_encode_frame,
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_NONE },
.supported_samplerates = (const int[]){
44100, 48000, 32000, 22050, 24000, 16000, 0
},
.channel_layouts = (const uint64_t[]){ AV_CH_LAYOUT_MONO,
AV_CH_LAYOUT_STEREO,
0 },
.defaults = mp2_defaults,
};
