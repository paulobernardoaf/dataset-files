#include <float.h>
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"
#include "avfilter.h"
#include "internal.h"
typedef struct BlackDetectContext {
const AVClass *class;
double black_min_duration_time; 
int64_t black_min_duration; 
int64_t black_start; 
int64_t black_end; 
int64_t last_picref_pts; 
int black_started;
double picture_black_ratio_th;
double pixel_black_th;
unsigned int pixel_black_th_i;
unsigned int nb_black_pixels; 
} BlackDetectContext;
#define OFFSET(x) offsetof(BlackDetectContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM
static const AVOption blackdetect_options[] = {
{ "d", "set minimum detected black duration in seconds", OFFSET(black_min_duration_time), AV_OPT_TYPE_DOUBLE, {.dbl=2}, 0, DBL_MAX, FLAGS },
{ "black_min_duration", "set minimum detected black duration in seconds", OFFSET(black_min_duration_time), AV_OPT_TYPE_DOUBLE, {.dbl=2}, 0, DBL_MAX, FLAGS },
{ "picture_black_ratio_th", "set the picture black ratio threshold", OFFSET(picture_black_ratio_th), AV_OPT_TYPE_DOUBLE, {.dbl=.98}, 0, 1, FLAGS },
{ "pic_th", "set the picture black ratio threshold", OFFSET(picture_black_ratio_th), AV_OPT_TYPE_DOUBLE, {.dbl=.98}, 0, 1, FLAGS },
{ "pixel_black_th", "set the pixel black threshold", OFFSET(pixel_black_th), AV_OPT_TYPE_DOUBLE, {.dbl=.10}, 0, 1, FLAGS },
{ "pix_th", "set the pixel black threshold", OFFSET(pixel_black_th), AV_OPT_TYPE_DOUBLE, {.dbl=.10}, 0, 1, FLAGS },
{ NULL }
};
AVFILTER_DEFINE_CLASS(blackdetect);
#define YUVJ_FORMATS AV_PIX_FMT_YUVJ411P, AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_YUVJ440P
static const enum AVPixelFormat yuvj_formats[] = {
YUVJ_FORMATS, AV_PIX_FMT_NONE
};
static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_GRAY8,
AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV411P,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P,
AV_PIX_FMT_YUV440P, AV_PIX_FMT_YUV444P,
AV_PIX_FMT_NV12, AV_PIX_FMT_NV21,
YUVJ_FORMATS,
AV_PIX_FMT_NONE
};
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}
static int config_input(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
BlackDetectContext *blackdetect = ctx->priv;
blackdetect->black_min_duration =
blackdetect->black_min_duration_time / av_q2d(inlink->time_base);
blackdetect->pixel_black_th_i = ff_fmt_is_in(inlink->format, yuvj_formats) ?
blackdetect->pixel_black_th * 255 :
16 + blackdetect->pixel_black_th * (235 - 16);
av_log(blackdetect, AV_LOG_VERBOSE,
"black_min_duration:%s pixel_black_th:%f pixel_black_th_i:%d picture_black_ratio_th:%f\n",
av_ts2timestr(blackdetect->black_min_duration, &inlink->time_base),
blackdetect->pixel_black_th, blackdetect->pixel_black_th_i,
blackdetect->picture_black_ratio_th);
return 0;
}
static void check_black_end(AVFilterContext *ctx)
{
BlackDetectContext *blackdetect = ctx->priv;
AVFilterLink *inlink = ctx->inputs[0];
if ((blackdetect->black_end - blackdetect->black_start) >= blackdetect->black_min_duration) {
av_log(blackdetect, AV_LOG_INFO,
"black_start:%s black_end:%s black_duration:%s\n",
av_ts2timestr(blackdetect->black_start, &inlink->time_base),
av_ts2timestr(blackdetect->black_end, &inlink->time_base),
av_ts2timestr(blackdetect->black_end - blackdetect->black_start, &inlink->time_base));
}
}
static int request_frame(AVFilterLink *outlink)
{
AVFilterContext *ctx = outlink->src;
BlackDetectContext *blackdetect = ctx->priv;
AVFilterLink *inlink = ctx->inputs[0];
int ret = ff_request_frame(inlink);
if (ret == AVERROR_EOF && blackdetect->black_started) {
blackdetect->black_end = blackdetect->last_picref_pts;
check_black_end(ctx);
}
return ret;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *picref)
{
AVFilterContext *ctx = inlink->dst;
BlackDetectContext *blackdetect = ctx->priv;
double picture_black_ratio = 0;
const uint8_t *p = picref->data[0];
int x, i;
for (i = 0; i < inlink->h; i++) {
for (x = 0; x < inlink->w; x++)
blackdetect->nb_black_pixels += p[x] <= blackdetect->pixel_black_th_i;
p += picref->linesize[0];
}
picture_black_ratio = (double)blackdetect->nb_black_pixels / (inlink->w * inlink->h);
av_log(ctx, AV_LOG_DEBUG,
"frame:%"PRId64" picture_black_ratio:%f pts:%s t:%s type:%c\n",
inlink->frame_count_out, picture_black_ratio,
av_ts2str(picref->pts), av_ts2timestr(picref->pts, &inlink->time_base),
av_get_picture_type_char(picref->pict_type));
if (picture_black_ratio >= blackdetect->picture_black_ratio_th) {
if (!blackdetect->black_started) {
blackdetect->black_started = 1;
blackdetect->black_start = picref->pts;
av_dict_set(&picref->metadata, "lavfi.black_start",
av_ts2timestr(blackdetect->black_start, &inlink->time_base), 0);
}
} else if (blackdetect->black_started) {
blackdetect->black_started = 0;
blackdetect->black_end = picref->pts;
check_black_end(ctx);
av_dict_set(&picref->metadata, "lavfi.black_end",
av_ts2timestr(blackdetect->black_end, &inlink->time_base), 0);
}
blackdetect->last_picref_pts = picref->pts;
blackdetect->nb_black_pixels = 0;
return ff_filter_frame(inlink->dst->outputs[0], picref);
}
static const AVFilterPad blackdetect_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_input,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad blackdetect_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.request_frame = request_frame,
},
{ NULL }
};
AVFilter ff_vf_blackdetect = {
.name = "blackdetect",
.description = NULL_IF_CONFIG_SMALL("Detect video intervals that are (almost) black."),
.priv_size = sizeof(BlackDetectContext),
.query_formats = query_formats,
.inputs = blackdetect_inputs,
.outputs = blackdetect_outputs,
.priv_class = &blackdetect_class,
};
