



















#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bsf.h"

static int text2movsub(AVBSFContext *ctx, AVPacket *out)
{
AVPacket *in;
int ret = 0;

ret = ff_bsf_get_packet(ctx, &in);
if (ret < 0)
return ret;

if (in->size > 0xffff) {
ret = AVERROR_INVALIDDATA;
goto fail;
}

ret = av_new_packet(out, in->size + 2);
if (ret < 0) {
ret = AVERROR(ENOMEM);
goto fail;
}

ret = av_packet_copy_props(out, in);
if (ret < 0)
goto fail;

AV_WB16(out->data, in->size);
memcpy(out->data + 2, in->data, in->size);

fail:
if (ret < 0)
av_packet_unref(out);
av_packet_free(&in);
return ret;
}

const AVBitStreamFilter ff_text2movsub_bsf = {
.name = "text2movsub",
.filter = text2movsub,
};

static int mov2textsub(AVBSFContext *ctx, AVPacket *pkt)
{
int ret = 0;

ret = ff_bsf_get_packet_ref(ctx, pkt);
if (ret < 0)
return ret;

if (pkt->size < 2) {
av_packet_unref(pkt);
return AVERROR_INVALIDDATA;
}

pkt->size = FFMIN(pkt->size - 2, AV_RB16(pkt->data));
pkt->data += 2;

return 0;
}

const AVBitStreamFilter ff_mov2textsub_bsf = {
.name = "mov2textsub",
.filter = mov2textsub,
};
