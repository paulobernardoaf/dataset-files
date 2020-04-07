


























#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "bsf.h"

typedef struct ProresMetadataContext {
const AVClass *class;

int color_primaries;
int transfer_characteristics;
int matrix_coefficients;
} ProresMetadataContext;

static int prores_metadata(AVBSFContext *bsf, AVPacket *pkt)
{
ProresMetadataContext *ctx = bsf->priv_data;
int ret = 0;
int buf_size;
uint8_t *buf;

ret = ff_bsf_get_packet_ref(bsf, pkt);
if (ret < 0)
return ret;

ret = av_packet_make_writable(pkt);
if (ret < 0)
goto fail;

buf = pkt->data;
buf_size = pkt->size;


if (buf_size < 28) {
av_log(bsf, AV_LOG_ERROR, "not enough data in prores frame\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}

if (AV_RL32(buf + 4) != AV_RL32("icpf")) {
av_log(bsf, AV_LOG_ERROR, "invalid frame header\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}

if (AV_RB16(buf + 8) < 28) {
av_log(bsf, AV_LOG_ERROR, "invalid frame header size\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}


if (ctx->color_primaries != -1)
buf[8+14] = ctx->color_primaries;
if (ctx->transfer_characteristics != -1)
buf[8+15] = ctx->transfer_characteristics;
if (ctx->matrix_coefficients != -1)
buf[8+16] = ctx->matrix_coefficients;

fail:
if (ret < 0)
av_packet_unref(pkt);
return ret;
}

static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_PRORES, AV_CODEC_ID_NONE,
};

static int prores_metadata_init(AVBSFContext *bsf)
{
ProresMetadataContext *ctx = bsf->priv_data;

switch (ctx->color_primaries) {
case -1:
case 0:
case AVCOL_PRI_BT709:
case AVCOL_PRI_BT470BG:
case AVCOL_PRI_SMPTE170M:
case AVCOL_PRI_BT2020:
case AVCOL_PRI_SMPTE431:
case AVCOL_PRI_SMPTE432:
break;
default:
av_log(bsf, AV_LOG_ERROR, "Color primaries %d is not a valid value\n", ctx->color_primaries);
return AVERROR(EINVAL);
}

switch (ctx->matrix_coefficients) {
case -1:
case 0:
case AVCOL_SPC_BT709:
case AVCOL_SPC_SMPTE170M:
case AVCOL_SPC_BT2020_NCL:
break;
default:
av_log(bsf, AV_LOG_ERROR, "Colorspace %d is not a valid value\n", ctx->matrix_coefficients);
return AVERROR(EINVAL);
}

return 0;
}

#define OFFSET(x) offsetof(ProresMetadataContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
{"color_primaries", "select color primaries", OFFSET(color_primaries), AV_OPT_TYPE_INT, {.i64=-1}, -1, AVCOL_PRI_SMPTE432, FLAGS, "color_primaries"},
{"auto", "keep the same color primaries", 0, AV_OPT_TYPE_CONST, {.i64=-1}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"unknown", NULL, 0, AV_OPT_TYPE_CONST, {.i64=0}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"bt709", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_BT709}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"bt470bg", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_BT470BG}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"smpte170m", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_SMPTE170M}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"bt2020", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_BT2020}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"smpte431", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_SMPTE431}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},
{"smpte432", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_PRI_SMPTE432}, INT_MIN, INT_MAX, FLAGS, "color_primaries"},

{"color_trc", "select color transfer", OFFSET(transfer_characteristics), AV_OPT_TYPE_INT, {.i64=-1}, -1, AVCOL_TRC_BT709, FLAGS, "color_trc"},
{"auto", "keep the same color transfer", 0, AV_OPT_TYPE_CONST, {.i64=-1}, INT_MIN, INT_MAX, FLAGS, "color_trc"},
{"unknown", NULL, 0, AV_OPT_TYPE_CONST, {.i64=0}, INT_MIN, INT_MAX, FLAGS, "color_trc"},
{"bt709", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_TRC_BT709}, INT_MIN, INT_MAX, FLAGS, "color_trc"},

{"colorspace", "select colorspace", OFFSET(matrix_coefficients), AV_OPT_TYPE_INT, {.i64=-1}, -1, AVCOL_SPC_BT2020_NCL, FLAGS, "colorspace"},
{"auto", "keep the same colorspace", 0, AV_OPT_TYPE_CONST, {.i64=-1}, INT_MIN, INT_MAX, FLAGS, "colorspace"},
{"unknown", NULL, 0, AV_OPT_TYPE_CONST, {.i64=0}, INT_MIN, INT_MAX, FLAGS, "colorspace"},
{"bt709", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_SPC_BT709}, INT_MIN, INT_MAX, FLAGS, "colorspace"},
{"smpte170m", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_SPC_SMPTE170M}, INT_MIN, INT_MAX, FLAGS, "colorspace"},
{"bt2020nc", NULL, 0, AV_OPT_TYPE_CONST, {.i64=AVCOL_SPC_BT2020_NCL}, INT_MIN, INT_MAX, FLAGS, "colorspace"},

{ NULL },
};

static const AVClass prores_metadata_class = {
.class_name = "prores_metadata_bsf",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_prores_metadata_bsf = {
.name = "prores_metadata",
.init = prores_metadata_init,
.filter = prores_metadata,
.priv_data_size = sizeof(ProresMetadataContext),
.priv_class = &prores_metadata_class,
.codec_ids = codec_ids,
};
