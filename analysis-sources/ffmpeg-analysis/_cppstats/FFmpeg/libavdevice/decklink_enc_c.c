#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "decklink_common_c.h"
#include "decklink_enc.h"
#define OFFSET(x) offsetof(struct decklink_cctx, x)
#define ENC AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "list_devices", "list available devices" , OFFSET(list_devices), AV_OPT_TYPE_INT , { .i64 = 0 }, 0, 1, ENC },
{ "list_formats", "list supported formats" , OFFSET(list_formats), AV_OPT_TYPE_INT , { .i64 = 0 }, 0, 1, ENC },
{ "preroll" , "video preroll in seconds", OFFSET(preroll ), AV_OPT_TYPE_DOUBLE, { .dbl = 0.5 }, 0, 5, ENC },
{ "duplex_mode" , "duplex mode" , OFFSET(duplex_mode ), AV_OPT_TYPE_INT , { .i64 = 0 }, 0, 2, ENC, "duplex_mode"},
{ "unset" , NULL , 0 , AV_OPT_TYPE_CONST , { .i64 = 0 }, 0, 0, ENC, "duplex_mode"},
{ "half" , NULL , 0 , AV_OPT_TYPE_CONST , { .i64 = 1 }, 0, 0, ENC, "duplex_mode"},
{ "full" , NULL , 0 , AV_OPT_TYPE_CONST , { .i64 = 2 }, 0, 0, ENC, "duplex_mode"},
{ "timing_offset", "genlock timing pixel offset", OFFSET(timing_offset), AV_OPT_TYPE_INT, { .i64 = INT_MIN }, INT_MIN, INT_MAX, ENC, "timing_offset"},
{ "unset" , NULL , 0 , AV_OPT_TYPE_CONST, { .i64 = INT_MIN }, 0, 0, ENC, "timing_offset"},
{ NULL },
};
static const AVClass decklink_muxer_class = {
.class_name = "Blackmagic DeckLink outdev",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
.category = AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT,
};
AVOutputFormat ff_decklink_muxer = {
.name = "decklink",
.long_name = NULL_IF_CONFIG_SMALL("Blackmagic DeckLink output"),
.audio_codec = AV_CODEC_ID_PCM_S16LE,
.video_codec = AV_CODEC_ID_WRAPPED_AVFRAME,
.subtitle_codec = AV_CODEC_ID_NONE,
.flags = AVFMT_NOFILE,
.get_device_list = ff_decklink_list_output_devices,
.priv_class = &decklink_muxer_class,
.priv_data_size = sizeof(struct decklink_cctx),
.write_header = ff_decklink_write_header,
.write_packet = ff_decklink_write_packet,
.write_trailer = ff_decklink_write_trailer,
};
