



















#include "avcodec.h"
#include "bsf.h"
#include "bytestream.h"
#include "dca_syncwords.h"
#include "libavutil/mem.h"

static int dca_core_filter(AVBSFContext *ctx, AVPacket *pkt)
{
GetByteContext gb;
uint32_t syncword;
int core_size = 0, ret;

ret = ff_bsf_get_packet_ref(ctx, pkt);
if (ret < 0)
return ret;

bytestream2_init(&gb, pkt->data, pkt->size);
syncword = bytestream2_get_be32(&gb);
bytestream2_skip(&gb, 1);

switch (syncword) {
case DCA_SYNCWORD_CORE_BE:
core_size = ((bytestream2_get_be24(&gb) >> 4) & 0x3fff) + 1;
break;
}

if (core_size > 0 && core_size <= pkt->size) {
pkt->size = core_size;
}

return 0;
}

static const enum AVCodecID codec_ids[] = {
AV_CODEC_ID_DTS, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_dca_core_bsf = {
.name = "dca_core",
.filter = dca_core_filter,
.codec_ids = codec_ids,
};
