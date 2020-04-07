




















#include <string.h>

#include "avcodec.h"
#include "ass.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

static av_cold int ass_decode_init(AVCodecContext *avctx)
{
avctx->subtitle_header = av_malloc(avctx->extradata_size + 1);
if (!avctx->subtitle_header)
return AVERROR(ENOMEM);
if (avctx->extradata_size)
memcpy(avctx->subtitle_header, avctx->extradata, avctx->extradata_size);
avctx->subtitle_header[avctx->extradata_size] = 0;
avctx->subtitle_header_size = avctx->extradata_size;
return 0;
}

static int ass_decode_frame(AVCodecContext *avctx, void *data, int *got_sub_ptr,
AVPacket *avpkt)
{
AVSubtitle *sub = data;

if (avpkt->size <= 0)
return avpkt->size;

sub->rects = av_malloc(sizeof(*sub->rects));
if (!sub->rects)
return AVERROR(ENOMEM);
sub->rects[0] = av_mallocz(sizeof(*sub->rects[0]));
if (!sub->rects[0])
return AVERROR(ENOMEM);
sub->num_rects = 1;
sub->rects[0]->type = SUBTITLE_ASS;
sub->rects[0]->ass = av_strdup(avpkt->data);
if (!sub->rects[0]->ass)
return AVERROR(ENOMEM);
*got_sub_ptr = 1;
return avpkt->size;
}

#if CONFIG_SSA_DECODER
AVCodec ff_ssa_decoder = {
.name = "ssa",
.long_name = NULL_IF_CONFIG_SMALL("ASS (Advanced SubStation Alpha) subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_ASS,
.init = ass_decode_init,
.decode = ass_decode_frame,
};
#endif

#if CONFIG_ASS_DECODER
AVCodec ff_ass_decoder = {
.name = "ass",
.long_name = NULL_IF_CONFIG_SMALL("ASS (Advanced SubStation Alpha) subtitle"),
.type = AVMEDIA_TYPE_SUBTITLE,
.id = AV_CODEC_ID_ASS,
.init = ass_decode_init,
.decode = ass_decode_frame,
};
#endif
