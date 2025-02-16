#include "avcodec.h"
#include "bsf.h"
#include "get_bits.h"
#include "ac3_parser_internal.h"
static int eac3_core_filter(AVBSFContext *ctx, AVPacket *pkt)
{
AC3HeaderInfo hdr;
GetBitContext gbc;
int ret;
ret = ff_bsf_get_packet_ref(ctx, pkt);
if (ret < 0)
return ret;
ret = init_get_bits8(&gbc, pkt->data, pkt->size);
if (ret < 0)
goto fail;
ret = ff_ac3_parse_header(&gbc, &hdr);
if (ret < 0) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
if (hdr.frame_type == EAC3_FRAME_TYPE_INDEPENDENT ||
hdr.frame_type == EAC3_FRAME_TYPE_AC3_CONVERT) {
pkt->size = FFMIN(hdr.frame_size, pkt->size);
} else if (hdr.frame_type == EAC3_FRAME_TYPE_DEPENDENT && pkt->size > hdr.frame_size) {
AC3HeaderInfo hdr2;
ret = init_get_bits8(&gbc, pkt->data + hdr.frame_size, pkt->size - hdr.frame_size);
if (ret < 0)
goto fail;
ret = ff_ac3_parse_header(&gbc, &hdr2);
if (ret < 0) {
ret = AVERROR_INVALIDDATA;
goto fail;
}
if (hdr2.frame_type == EAC3_FRAME_TYPE_INDEPENDENT ||
hdr2.frame_type == EAC3_FRAME_TYPE_AC3_CONVERT) {
pkt->size -= hdr.frame_size;
pkt->data += hdr.frame_size;
} else {
pkt->size = 0;
}
} else {
pkt->size = 0;
}
return 0;
fail:
av_packet_unref(pkt);
return ret;
}
static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_EAC3, AV_CODEC_ID_NONE,
};
const AVBitStreamFilter ff_eac3_core_bsf = {
.name = "eac3_core",
.filter = eac3_core_filter,
.codec_ids = codec_ids,
};
