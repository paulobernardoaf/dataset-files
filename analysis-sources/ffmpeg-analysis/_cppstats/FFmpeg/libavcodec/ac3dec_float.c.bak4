




























#include "ac3dec.h"
#include "eac3dec.c"
#include "ac3dec.c"

static const AVOption options[] = {
{ "cons_noisegen", "enable consistent noise generation", OFFSET(consistent_noise_generation), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, PAR },
{ "drc_scale", "percentage of dynamic range compression to apply", OFFSET(drc_scale), AV_OPT_TYPE_FLOAT, {.dbl = 1.0}, 0.0, 6.0, PAR },
{ "heavy_compr", "enable heavy dynamic range compression", OFFSET(heavy_compression), AV_OPT_TYPE_BOOL, {.i64 = 0 }, 0, 1, PAR },
{ "target_level", "target level in -dBFS (0 not applied)", OFFSET(target_level), AV_OPT_TYPE_INT, {.i64 = 0 }, -31, 0, PAR },

{"dmix_mode", "Preferred Stereo Downmix Mode", OFFSET(preferred_stereo_downmix), AV_OPT_TYPE_INT, {.i64 = -1 }, -1, 2, 0, "dmix_mode"},
{"ltrt_cmixlev", "Lt/Rt Center Mix Level", OFFSET(ltrt_center_mix_level), AV_OPT_TYPE_FLOAT, {.dbl = -1.0 }, -1.0, 2.0, 0},
{"ltrt_surmixlev", "Lt/Rt Surround Mix Level", OFFSET(ltrt_surround_mix_level), AV_OPT_TYPE_FLOAT, {.dbl = -1.0 }, -1.0, 2.0, 0},
{"loro_cmixlev", "Lo/Ro Center Mix Level", OFFSET(loro_center_mix_level), AV_OPT_TYPE_FLOAT, {.dbl = -1.0 }, -1.0, 2.0, 0},
{"loro_surmixlev", "Lo/Ro Surround Mix Level", OFFSET(loro_surround_mix_level), AV_OPT_TYPE_FLOAT, {.dbl = -1.0 }, -1.0, 2.0, 0},

{ NULL},
};

static const AVClass ac3_decoder_class = {
.class_name = "AC3 decoder",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_ac3_decoder = {
.name = "ac3",
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_AC3,
.priv_data_size = sizeof (AC3DecodeContext),
.init = ac3_decode_init,
.close = ac3_decode_end,
.decode = ac3_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.long_name = NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
.priv_class = &ac3_decoder_class,
};

#if CONFIG_EAC3_DECODER
static const AVClass eac3_decoder_class = {
.class_name = "E-AC3 decoder",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_eac3_decoder = {
.name = "eac3",
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_EAC3,
.priv_data_size = sizeof (AC3DecodeContext),
.init = ac3_decode_init,
.close = ac3_decode_end,
.decode = ac3_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.long_name = NULL_IF_CONFIG_SMALL("ATSC A/52B (AC-3, E-AC-3)"),
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
.priv_class = &eac3_decoder_class,
};
#endif
