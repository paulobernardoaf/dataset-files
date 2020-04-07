#include <stdio.h>
#include <inttypes.h>
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "video.h"
typedef struct BlackFrameContext {
const AVClass *class;
int bamount; 
int bthresh; 
unsigned int frame; 
unsigned int nblack; 
unsigned int last_keyframe; 
} BlackFrameContext;
static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV420P, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NV12,
AV_PIX_FMT_NV21, AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUV411P,
AV_PIX_FMT_NONE
};
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}
#define SET_META(key, format, value) snprintf(buf, sizeof(buf), format, value); av_dict_set(metadata, key, buf, 0)
static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
AVFilterContext *ctx = inlink->dst;
BlackFrameContext *s = ctx->priv;
int x, i;
int pblack = 0;
uint8_t *p = frame->data[0];
AVDictionary **metadata;
char buf[32];
for (i = 0; i < frame->height; i++) {
for (x = 0; x < inlink->w; x++)
s->nblack += p[x] < s->bthresh;
p += frame->linesize[0];
}
if (frame->key_frame)
s->last_keyframe = s->frame;
pblack = s->nblack * 100 / (inlink->w * inlink->h);
if (pblack >= s->bamount) {
metadata = &frame->metadata;
av_log(ctx, AV_LOG_INFO, "frame:%u pblack:%u pts:%"PRId64" t:%f "
"type:%c last_keyframe:%d\n",
s->frame, pblack, frame->pts,
frame->pts == AV_NOPTS_VALUE ? -1 : frame->pts * av_q2d(inlink->time_base),
av_get_picture_type_char(frame->pict_type), s->last_keyframe);
SET_META("lavfi.blackframe.pblack", "%u", pblack);
}
s->frame++;
s->nblack = 0;
return ff_filter_frame(inlink->dst->outputs[0], frame);
}
#define OFFSET(x) offsetof(BlackFrameContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM
static const AVOption blackframe_options[] = {
{ "amount", "percentage of the pixels that have to be below the threshold "
"for the frame to be considered black", OFFSET(bamount), AV_OPT_TYPE_INT, { .i64 = 98 }, 0, 100, FLAGS },
{ "threshold", "threshold below which a pixel value is considered black",
OFFSET(bthresh), AV_OPT_TYPE_INT, { .i64 = 32 }, 0, 255, FLAGS },
{ "thresh", "threshold below which a pixel value is considered black",
OFFSET(bthresh), AV_OPT_TYPE_INT, { .i64 = 32 }, 0, 255, FLAGS },
{ NULL }
};
AVFILTER_DEFINE_CLASS(blackframe);
static const AVFilterPad avfilter_vf_blackframe_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad avfilter_vf_blackframe_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO
},
{ NULL }
};
AVFilter ff_vf_blackframe = {
.name = "blackframe",
.description = NULL_IF_CONFIG_SMALL("Detect frames that are (almost) black."),
.priv_size = sizeof(BlackFrameContext),
.priv_class = &blackframe_class,
.query_formats = query_formats,
.inputs = avfilter_vf_blackframe_inputs,
.outputs = avfilter_vf_blackframe_outputs,
};
