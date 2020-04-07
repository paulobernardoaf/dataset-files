


























#include "avcodec.h"
#include "bsf.h"
#include "bytestream.h"
#include "hap.h"

typedef struct HapqaExtractContext {
const AVClass *class;
int texture;
} HapqaExtractContext;

static int check_texture(HapqaExtractContext *ctx, int section_type) {
if (((ctx->texture == 0)&&((section_type & 0x0F) == 0x0F)) || 
((ctx->texture == 1)&&((section_type & 0x0F) == 0x01))) 
{
return 1; 
} else {
return 0;
}
}

static int hapqa_extract(AVBSFContext *bsf, AVPacket *pkt)
{
HapqaExtractContext *ctx = bsf->priv_data;
GetByteContext gbc;
int section_size;
enum HapSectionType section_type;
int start_section_size;
int target_packet_size = 0;
int ret = 0;

ret = ff_bsf_get_packet_ref(bsf, pkt);
if (ret < 0)
return ret;

bytestream2_init(&gbc, pkt->data, pkt->size);
ret = ff_hap_parse_section_header(&gbc, &section_size, &section_type);
if (ret != 0)
goto fail;

if ((section_type & 0x0F) != 0x0D) {
av_log(bsf, AV_LOG_ERROR, "Invalid section type for HAPQA %#04x.\n", section_type & 0x0F);
ret = AVERROR_INVALIDDATA;
goto fail;
}

start_section_size = 4;

bytestream2_seek(&gbc, start_section_size, SEEK_SET);

ret = ff_hap_parse_section_header(&gbc, &section_size, &section_type);
if (ret != 0)
goto fail;

target_packet_size = section_size + 4;

if (check_texture(ctx, section_type) == 0) { 
start_section_size += 4 + section_size;
bytestream2_seek(&gbc, start_section_size, SEEK_SET);
ret = ff_hap_parse_section_header(&gbc, &section_size, &section_type);
if (ret != 0)
goto fail;

target_packet_size = section_size + 4;

if (check_texture(ctx, section_type) == 0){ 
av_log(bsf, AV_LOG_ERROR, "No valid texture found.\n");
ret = AVERROR_INVALIDDATA;
goto fail;
}
}

pkt->data += start_section_size;
pkt->size = target_packet_size;

fail:
if (ret < 0)
av_packet_unref(pkt);
return ret;
}

static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_HAP, AV_CODEC_ID_NONE,
};

#define OFFSET(x) offsetof(HapqaExtractContext, x)
#define FLAGS (AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_BSF_PARAM)
static const AVOption options[] = {
{ "texture", "texture to keep", OFFSET(texture), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, FLAGS, "texture" },
{ "color", "keep HapQ texture", 0, AV_OPT_TYPE_CONST, { .i64 = 0 }, 0, 0, FLAGS, "texture" },
{ "alpha", "keep HapAlphaOnly texture", 0, AV_OPT_TYPE_CONST, { .i64 = 1 }, 0, 0, FLAGS, "texture" },
{ NULL },
};

static const AVClass hapqa_extract_class = {
.class_name = "hapqa_extract_bsf",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};

const AVBitStreamFilter ff_hapqa_extract_bsf = {
.name = "hapqa_extract",
.filter = hapqa_extract,
.priv_data_size = sizeof(HapqaExtractContext),
.priv_class = &hapqa_extract_class,
.codec_ids = codec_ids,
};
