

















#include <string.h>

#include "libavutil/common.h"
#include "libavutil/mem.h"

#include "bsf.h"
#include "cbs.h"
#include "cbs_h264.h"
#include "h264.h"


typedef struct H264RedundantPPSContext {
CodedBitstreamContext *input;
CodedBitstreamContext *output;

CodedBitstreamFragment access_unit;

int global_pic_init_qp;
int current_pic_init_qp;
int extradata_pic_init_qp;
} H264RedundantPPSContext;


static int h264_redundant_pps_fixup_pps(H264RedundantPPSContext *ctx,
H264RawPPS *pps)
{


ctx->current_pic_init_qp = pps->pic_init_qp_minus26 + 26;
pps->pic_init_qp_minus26 = ctx->global_pic_init_qp - 26;




pps->weighted_pred_flag = 1;

return 0;
}

static int h264_redundant_pps_fixup_slice(H264RedundantPPSContext *ctx,
H264RawSliceHeader *slice)
{
int qp;

qp = ctx->current_pic_init_qp + slice->slice_qp_delta;
slice->slice_qp_delta = qp - ctx->global_pic_init_qp;

return 0;
}

static int h264_redundant_pps_filter(AVBSFContext *bsf, AVPacket *pkt)
{
H264RedundantPPSContext *ctx = bsf->priv_data;
CodedBitstreamFragment *au = &ctx->access_unit;
int au_has_sps;
int err, i;

err = ff_bsf_get_packet_ref(bsf, pkt);
if (err < 0)
return err;

err = ff_cbs_read_packet(ctx->input, au, pkt);
if (err < 0)
goto fail;

au_has_sps = 0;
for (i = 0; i < au->nb_units; i++) {
CodedBitstreamUnit *nal = &au->units[i];

if (nal->type == H264_NAL_SPS)
au_has_sps = 1;
if (nal->type == H264_NAL_PPS) {
err = h264_redundant_pps_fixup_pps(ctx, nal->content);
if (err < 0)
goto fail;
if (!au_has_sps) {
av_log(bsf, AV_LOG_VERBOSE, "Deleting redundant PPS "
"at %"PRId64".\n", pkt->pts);
ff_cbs_delete_unit(ctx->input, au, i);
i--;
continue;
}
}
if (nal->type == H264_NAL_SLICE ||
nal->type == H264_NAL_IDR_SLICE) {
H264RawSlice *slice = nal->content;
h264_redundant_pps_fixup_slice(ctx, &slice->header);
}
}

err = ff_cbs_write_packet(ctx->output, pkt, au);
if (err < 0)
goto fail;

err = 0;
fail:
ff_cbs_fragment_reset(ctx->output, au);
if (err < 0)
av_packet_unref(pkt);

return err;
}

static int h264_redundant_pps_init(AVBSFContext *bsf)
{
H264RedundantPPSContext *ctx = bsf->priv_data;
CodedBitstreamFragment *au = &ctx->access_unit;
int err, i;

err = ff_cbs_init(&ctx->input, AV_CODEC_ID_H264, bsf);
if (err < 0)
return err;

err = ff_cbs_init(&ctx->output, AV_CODEC_ID_H264, bsf);
if (err < 0)
return err;

ctx->global_pic_init_qp = 26;

if (bsf->par_in->extradata) {
err = ff_cbs_read_extradata(ctx->input, au, bsf->par_in);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to read extradata.\n");
goto fail;
}

for (i = 0; i < au->nb_units; i++) {
if (au->units[i].type == H264_NAL_PPS) {
err = h264_redundant_pps_fixup_pps(ctx, au->units[i].content);
if (err < 0)
goto fail;
}
}

ctx->extradata_pic_init_qp = ctx->current_pic_init_qp;
err = ff_cbs_write_extradata(ctx->output, bsf->par_out, au);
if (err < 0) {
av_log(bsf, AV_LOG_ERROR, "Failed to write extradata.\n");
goto fail;
}
}

err = 0;
fail:
ff_cbs_fragment_reset(ctx->output, au);
return err;
}

static void h264_redundant_pps_flush(AVBSFContext *bsf)
{
H264RedundantPPSContext *ctx = bsf->priv_data;
ctx->current_pic_init_qp = ctx->extradata_pic_init_qp;
}

static void h264_redundant_pps_close(AVBSFContext *bsf)
{
H264RedundantPPSContext *ctx = bsf->priv_data;

ff_cbs_fragment_free(ctx->input, &ctx->access_unit);
ff_cbs_close(&ctx->input);
ff_cbs_close(&ctx->output);
}

static const enum AVCodecID h264_redundant_pps_codec_ids[] = {
AV_CODEC_ID_H264, AV_CODEC_ID_NONE,
};

const AVBitStreamFilter ff_h264_redundant_pps_bsf = {
.name = "h264_redundant_pps",
.priv_data_size = sizeof(H264RedundantPPSContext),
.init = &h264_redundant_pps_init,
.flush = &h264_redundant_pps_flush,
.close = &h264_redundant_pps_close,
.filter = &h264_redundant_pps_filter,
.codec_ids = h264_redundant_pps_codec_ids,
};
