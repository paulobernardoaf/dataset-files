#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/pixdesc.h"
#include "libavutil/log.h"
#include "libavutil/base64.h"
#include "avcodec.h"
#include "internal.h"
#include <theora/theoraenc.h>
typedef struct TheoraContext {
th_enc_ctx *t_state;
uint8_t *stats;
int stats_size;
int stats_offset;
int uv_hshift;
int uv_vshift;
int keyframe_mask;
} TheoraContext;
static int concatenate_packet(unsigned int* offset,
AVCodecContext* avc_context,
const ogg_packet* packet)
{
const char* message = NULL;
int newsize = avc_context->extradata_size + 2 + packet->bytes;
int err = AVERROR_INVALIDDATA;
if (packet->bytes < 0) {
message = "ogg_packet has negative size";
} else if (packet->bytes > 0xffff) {
message = "ogg_packet is larger than 65535 bytes";
} else if (newsize < avc_context->extradata_size) {
message = "extradata_size would overflow";
} else {
if ((err = av_reallocp(&avc_context->extradata, newsize)) < 0) {
avc_context->extradata_size = 0;
message = "av_realloc failed";
}
}
if (message) {
av_log(avc_context, AV_LOG_ERROR, "concatenate_packet failed: %s\n", message);
return err;
}
avc_context->extradata_size = newsize;
AV_WB16(avc_context->extradata + (*offset), packet->bytes);
*offset += 2;
memcpy(avc_context->extradata + (*offset), packet->packet, packet->bytes);
(*offset) += packet->bytes;
return 0;
}
static int get_stats(AVCodecContext *avctx, int eos)
{
#if defined(TH_ENCCTL_2PASS_OUT)
TheoraContext *h = avctx->priv_data;
uint8_t *buf;
int bytes;
bytes = th_encode_ctl(h->t_state, TH_ENCCTL_2PASS_OUT, &buf, sizeof(buf));
if (bytes < 0) {
av_log(avctx, AV_LOG_ERROR, "Error getting first pass stats\n");
return AVERROR_EXTERNAL;
}
if (!eos) {
void *tmp = av_fast_realloc(h->stats, &h->stats_size,
h->stats_offset + bytes);
if (!tmp)
return AVERROR(ENOMEM);
h->stats = tmp;
memcpy(h->stats + h->stats_offset, buf, bytes);
h->stats_offset += bytes;
} else {
int b64_size = AV_BASE64_SIZE(h->stats_offset);
memcpy(h->stats, buf, bytes);
avctx->stats_out = av_malloc(b64_size);
if (!avctx->stats_out)
return AVERROR(ENOMEM);
av_base64_encode(avctx->stats_out, b64_size, h->stats, h->stats_offset);
}
return 0;
#else
av_log(avctx, AV_LOG_ERROR, "libtheora too old to support 2pass\n");
return AVERROR(ENOSUP);
#endif
}
static int submit_stats(AVCodecContext *avctx)
{
#if defined(TH_ENCCTL_2PASS_IN)
TheoraContext *h = avctx->priv_data;
int bytes;
if (!h->stats) {
if (!avctx->stats_in) {
av_log(avctx, AV_LOG_ERROR, "No statsfile for second pass\n");
return AVERROR(EINVAL);
}
h->stats_size = strlen(avctx->stats_in) * 3/4;
h->stats = av_malloc(h->stats_size);
if (!h->stats) {
h->stats_size = 0;
return AVERROR(ENOMEM);
}
h->stats_size = av_base64_decode(h->stats, avctx->stats_in, h->stats_size);
}
while (h->stats_size - h->stats_offset > 0) {
bytes = th_encode_ctl(h->t_state, TH_ENCCTL_2PASS_IN,
h->stats + h->stats_offset,
h->stats_size - h->stats_offset);
if (bytes < 0) {
av_log(avctx, AV_LOG_ERROR, "Error submitting stats\n");
return AVERROR_EXTERNAL;
}
if (!bytes)
return 0;
h->stats_offset += bytes;
}
return 0;
#else
av_log(avctx, AV_LOG_ERROR, "libtheora too old to support 2pass\n");
return AVERROR(ENOSUP);
#endif
}
static av_cold int encode_init(AVCodecContext* avc_context)
{
th_info t_info;
th_comment t_comment;
ogg_packet o_packet;
unsigned int offset;
TheoraContext *h = avc_context->priv_data;
uint32_t gop_size = avc_context->gop_size;
int ret;
th_info_init(&t_info);
t_info.frame_width = FFALIGN(avc_context->width, 16);
t_info.frame_height = FFALIGN(avc_context->height, 16);
t_info.pic_width = avc_context->width;
t_info.pic_height = avc_context->height;
t_info.pic_x = 0;
t_info.pic_y = 0;
t_info.fps_numerator = avc_context->time_base.den;
t_info.fps_denominator = avc_context->time_base.num;
if (avc_context->sample_aspect_ratio.num) {
t_info.aspect_numerator = avc_context->sample_aspect_ratio.num;
t_info.aspect_denominator = avc_context->sample_aspect_ratio.den;
} else {
t_info.aspect_numerator = 1;
t_info.aspect_denominator = 1;
}
if (avc_context->color_primaries == AVCOL_PRI_BT470M)
t_info.colorspace = TH_CS_ITU_REC_470M;
else if (avc_context->color_primaries == AVCOL_PRI_BT470BG)
t_info.colorspace = TH_CS_ITU_REC_470BG;
else
t_info.colorspace = TH_CS_UNSPECIFIED;
if (avc_context->pix_fmt == AV_PIX_FMT_YUV420P)
t_info.pixel_fmt = TH_PF_420;
else if (avc_context->pix_fmt == AV_PIX_FMT_YUV422P)
t_info.pixel_fmt = TH_PF_422;
else if (avc_context->pix_fmt == AV_PIX_FMT_YUV444P)
t_info.pixel_fmt = TH_PF_444;
else {
av_log(avc_context, AV_LOG_ERROR, "Unsupported pix_fmt\n");
return AVERROR(EINVAL);
}
ret = av_pix_fmt_get_chroma_sub_sample(avc_context->pix_fmt, &h->uv_hshift, &h->uv_vshift);
if (ret)
return ret;
if (avc_context->flags & AV_CODEC_FLAG_QSCALE) {
t_info.quality = av_clipf(avc_context->global_quality / (float)FF_QP2LAMBDA, 0, 10) * 6.3;
t_info.target_bitrate = 0;
} else {
t_info.target_bitrate = avc_context->bit_rate;
t_info.quality = 0;
}
h->t_state = th_encode_alloc(&t_info);
if (!h->t_state) {
av_log(avc_context, AV_LOG_ERROR, "theora_encode_init failed\n");
return AVERROR_EXTERNAL;
}
h->keyframe_mask = (1 << t_info.keyframe_granule_shift) - 1;
th_info_clear(&t_info);
if (th_encode_ctl(h->t_state, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE,
&gop_size, sizeof(gop_size))) {
av_log(avc_context, AV_LOG_ERROR, "Error setting GOP size\n");
return AVERROR_EXTERNAL;
}
if (avc_context->flags & AV_CODEC_FLAG_PASS1) {
if ((ret = get_stats(avc_context, 0)) < 0)
return ret;
} else if (avc_context->flags & AV_CODEC_FLAG_PASS2) {
if ((ret = submit_stats(avc_context)) < 0)
return ret;
}
offset = 0;
th_comment_init(&t_comment);
while (th_encode_flushheader(h->t_state, &t_comment, &o_packet))
if ((ret = concatenate_packet(&offset, avc_context, &o_packet)) < 0)
return ret;
th_comment_clear(&t_comment);
return 0;
}
static int encode_frame(AVCodecContext* avc_context, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
th_ycbcr_buffer t_yuv_buffer;
TheoraContext *h = avc_context->priv_data;
ogg_packet o_packet;
int result, i, ret;
if (!frame) {
th_encode_packetout(h->t_state, 1, &o_packet);
if (avc_context->flags & AV_CODEC_FLAG_PASS1)
if ((ret = get_stats(avc_context, 1)) < 0)
return ret;
return 0;
}
for (i = 0; i < 3; i++) {
t_yuv_buffer[i].width = FFALIGN(avc_context->width, 16) >> (i && h->uv_hshift);
t_yuv_buffer[i].height = FFALIGN(avc_context->height, 16) >> (i && h->uv_vshift);
t_yuv_buffer[i].stride = frame->linesize[i];
t_yuv_buffer[i].data = frame->data[i];
}
if (avc_context->flags & AV_CODEC_FLAG_PASS2)
if ((ret = submit_stats(avc_context)) < 0)
return ret;
result = th_encode_ycbcr_in(h->t_state, t_yuv_buffer);
if (result) {
const char* message;
switch (result) {
case -1:
message = "differing frame sizes";
break;
case TH_EINVAL:
message = "encoder is not ready or is finished";
break;
default:
message = "unknown reason";
break;
}
av_log(avc_context, AV_LOG_ERROR, "theora_encode_YUVin failed (%s) [%d]\n", message, result);
return AVERROR_EXTERNAL;
}
if (avc_context->flags & AV_CODEC_FLAG_PASS1)
if ((ret = get_stats(avc_context, 0)) < 0)
return ret;
result = th_encode_packetout(h->t_state, 0, &o_packet);
switch (result) {
case 0:
return 0;
case 1:
break;
default:
av_log(avc_context, AV_LOG_ERROR, "theora_encode_packetout failed [%d]\n", result);
return AVERROR_EXTERNAL;
}
if ((ret = ff_alloc_packet2(avc_context, pkt, o_packet.bytes, 0)) < 0)
return ret;
memcpy(pkt->data, o_packet.packet, o_packet.bytes);
pkt->pts = pkt->dts = frame->pts;
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avc_context->coded_frame->key_frame = !(o_packet.granulepos & h->keyframe_mask);
FF_ENABLE_DEPRECATION_WARNINGS
#endif
if (!(o_packet.granulepos & h->keyframe_mask))
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
return 0;
}
static av_cold int encode_close(AVCodecContext* avc_context)
{
TheoraContext *h = avc_context->priv_data;
th_encode_free(h->t_state);
av_freep(&h->stats);
av_freep(&avc_context->stats_out);
av_freep(&avc_context->extradata);
avc_context->extradata_size = 0;
return 0;
}
AVCodec ff_libtheora_encoder = {
.name = "libtheora",
.long_name = NULL_IF_CONFIG_SMALL("libtheora Theora"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_THEORA,
.priv_data_size = sizeof(TheoraContext),
.init = encode_init,
.close = encode_close,
.encode2 = encode_frame,
.capabilities = AV_CODEC_CAP_DELAY, 
.pix_fmts = (const enum AVPixelFormat[]){
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUV444P, AV_PIX_FMT_NONE
},
.wrapper_name = "libtheora",
};
