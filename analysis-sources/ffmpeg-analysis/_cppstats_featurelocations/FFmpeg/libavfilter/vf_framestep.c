
























#include "libavutil/opt.h"
#include "avfilter.h"
#include "internal.h"
#include "video.h"

typedef struct NullContext {
const AVClass *class;
int frame_step;
} FrameStepContext;

#define OFFSET(x) offsetof(FrameStepContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM

static const AVOption framestep_options[] = {
{ "step", "set frame step", OFFSET(frame_step), AV_OPT_TYPE_INT, {.i64=1}, 1, INT_MAX, FLAGS},
{ NULL },
};

AVFILTER_DEFINE_CLASS(framestep);

static int config_output_props(AVFilterLink *outlink)
{
AVFilterContext *ctx = outlink->src;
FrameStepContext *framestep = ctx->priv;
AVFilterLink *inlink = ctx->inputs[0];

outlink->frame_rate =
av_div_q(inlink->frame_rate, (AVRational){framestep->frame_step, 1});

av_log(ctx, AV_LOG_VERBOSE, "step:%d frame_rate:%d/%d(%f) -> frame_rate:%d/%d(%f)\n",
framestep->frame_step,
inlink->frame_rate.num, inlink->frame_rate.den, av_q2d(inlink->frame_rate),
outlink->frame_rate.num, outlink->frame_rate.den, av_q2d(outlink->frame_rate));
return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *ref)
{
FrameStepContext *framestep = inlink->dst->priv;

if (!(inlink->frame_count_out % framestep->frame_step)) {
return ff_filter_frame(inlink->dst->outputs[0], ref);
} else {
av_frame_free(&ref);
return 0;
}
}

static const AVFilterPad framestep_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad framestep_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_output_props,
},
{ NULL }
};

AVFilter ff_vf_framestep = {
.name = "framestep",
.description = NULL_IF_CONFIG_SMALL("Select one frame every N frames."),
.priv_size = sizeof(FrameStepContext),
.priv_class = &framestep_class,
.inputs = framestep_inputs,
.outputs = framestep_outputs,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
