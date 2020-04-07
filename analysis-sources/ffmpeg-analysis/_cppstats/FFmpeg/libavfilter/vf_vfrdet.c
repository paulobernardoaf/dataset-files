#include "libavutil/common.h"
#include "libavutil/opt.h"
#include "internal.h"
typedef struct VFRDETContext {
const AVClass *class;
int64_t prev_pts;
int64_t delta;
int64_t min_delta;
int64_t max_delta;
int64_t avg_delta;
uint64_t vfr;
uint64_t cfr;
} VFRDETContext;
static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
AVFilterContext *ctx = inlink->dst;
VFRDETContext *s = ctx->priv;
if (s->prev_pts != AV_NOPTS_VALUE) {
int64_t delta = in->pts - s->prev_pts;
if (s->delta == AV_NOPTS_VALUE) {
s->delta = delta;
s->min_delta = delta;
s->max_delta = delta;
}
if (s->delta != delta) {
s->vfr++;
s->delta = delta;
s->min_delta = FFMIN(delta, s->min_delta);
s->max_delta = FFMAX(delta, s->max_delta);
s->avg_delta += delta;
} else {
s->cfr++;
}
}
s->prev_pts = in->pts;
return ff_filter_frame(ctx->outputs[0], in);
}
static av_cold int init(AVFilterContext *ctx)
{
VFRDETContext *s = ctx->priv;
s->prev_pts = AV_NOPTS_VALUE;
s->delta = AV_NOPTS_VALUE;
s->min_delta = INT64_MAX;
s->max_delta = INT64_MIN;
return 0;
}
static av_cold void uninit(AVFilterContext *ctx)
{
VFRDETContext *s = ctx->priv;
av_log(ctx, AV_LOG_INFO, "VFR:%f (%"PRIu64"/%"PRIu64")", s->vfr / (float)(s->vfr + s->cfr), s->vfr, s->cfr);
if (s->vfr)
av_log(ctx, AV_LOG_INFO, " min: %"PRId64" max: %"PRId64" avg: %"PRId64, s->min_delta, s->max_delta, s->avg_delta / s->vfr);
av_log(ctx, AV_LOG_INFO, "\n");
}
static const AVFilterPad vfrdet_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad vfrdet_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
AVFilter ff_vf_vfrdet = {
.name = "vfrdet",
.description = NULL_IF_CONFIG_SMALL("Variable frame rate detect filter."),
.priv_size = sizeof(VFRDETContext),
.init = init,
.uninit = uninit,
.inputs = vfrdet_inputs,
.outputs = vfrdet_outputs,
};
