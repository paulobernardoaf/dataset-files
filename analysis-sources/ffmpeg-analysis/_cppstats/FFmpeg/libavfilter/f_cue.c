#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "avfilter.h"
#include "filters.h"
#include "internal.h"
typedef struct CueContext {
const AVClass *class;
int64_t first_pts;
int64_t cue;
int64_t preroll;
int64_t buffer;
int status;
} CueContext;
static int activate(AVFilterContext *ctx)
{
AVFilterLink *inlink = ctx->inputs[0];
AVFilterLink *outlink = ctx->outputs[0];
CueContext *s = ctx->priv;
FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);
if (ff_inlink_queued_frames(inlink)) {
AVFrame *frame = ff_inlink_peek_frame(inlink, 0);
int64_t pts = av_rescale_q(frame->pts, inlink->time_base, AV_TIME_BASE_Q);
if (!s->status) {
s->first_pts = pts;
s->status++;
}
if (s->status == 1) {
if (pts - s->first_pts < s->preroll) {
int ret = ff_inlink_consume_frame(inlink, &frame);
if (ret < 0)
return ret;
return ff_filter_frame(outlink, frame);
}
s->first_pts = pts;
s->status++;
}
if (s->status == 2) {
frame = ff_inlink_peek_frame(inlink, ff_inlink_queued_frames(inlink) - 1);
pts = av_rescale_q(frame->pts, inlink->time_base, AV_TIME_BASE_Q);
if (!(pts - s->first_pts < s->buffer && (av_gettime() - s->cue) < 0))
s->status++;
}
if (s->status == 3) {
int64_t diff;
while ((diff = (av_gettime() - s->cue)) < 0)
av_usleep(av_clip(-diff / 2, 100, 1000000));
s->status++;
}
if (s->status == 4) {
int ret = ff_inlink_consume_frame(inlink, &frame);
if (ret < 0)
return ret;
return ff_filter_frame(outlink, frame);
}
}
FF_FILTER_FORWARD_STATUS(inlink, outlink);
FF_FILTER_FORWARD_WANTED(outlink, inlink);
return FFERROR_NOT_READY;
}
#define OFFSET(x) offsetof(CueContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_FILTERING_PARAM
static const AVOption options[] = {
{ "cue", "cue unix timestamp in microseconds", OFFSET(cue), AV_OPT_TYPE_INT64, { .i64 = 0 }, 0, INT64_MAX, FLAGS },
{ "preroll", "preroll duration in seconds", OFFSET(preroll), AV_OPT_TYPE_DURATION, { .i64 = 0 }, 0, INT64_MAX, FLAGS },
{ "buffer", "buffer duration in seconds", OFFSET(buffer), AV_OPT_TYPE_DURATION, { .i64 = 0 }, 0, INT64_MAX, FLAGS },
{ NULL }
};
#if CONFIG_CUE_FILTER
#define cue_options options
AVFILTER_DEFINE_CLASS(cue);
static const AVFilterPad cue_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
static const AVFilterPad cue_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
AVFilter ff_vf_cue = {
.name = "cue",
.description = NULL_IF_CONFIG_SMALL("Delay filtering to match a cue."),
.priv_size = sizeof(CueContext),
.priv_class = &cue_class,
.inputs = cue_inputs,
.outputs = cue_outputs,
.activate = activate,
};
#endif 
#if CONFIG_ACUE_FILTER
#define acue_options options
AVFILTER_DEFINE_CLASS(acue);
static const AVFilterPad acue_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
static const AVFilterPad acue_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
AVFilter ff_af_acue = {
.name = "acue",
.description = NULL_IF_CONFIG_SMALL("Delay filtering to match a cue."),
.priv_size = sizeof(CueContext),
.priv_class = &acue_class,
.inputs = acue_inputs,
.outputs = acue_outputs,
.activate = activate,
};
#endif 
