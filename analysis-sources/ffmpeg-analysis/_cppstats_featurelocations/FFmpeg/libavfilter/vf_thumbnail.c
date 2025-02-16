




























#include "libavutil/opt.h"
#include "avfilter.h"
#include "internal.h"

#define HIST_SIZE (3*256)

struct thumb_frame {
AVFrame *buf; 
int histogram[HIST_SIZE]; 
};

typedef struct ThumbContext {
const AVClass *class;
int n; 
int n_frames; 
struct thumb_frame *frames; 
AVRational tb; 
} ThumbContext;

#define OFFSET(x) offsetof(ThumbContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption thumbnail_options[] = {
{ "n", "set the frames batch size", OFFSET(n_frames), AV_OPT_TYPE_INT, {.i64=100}, 2, INT_MAX, FLAGS },
{ NULL }
};

AVFILTER_DEFINE_CLASS(thumbnail);

static av_cold int init(AVFilterContext *ctx)
{
ThumbContext *s = ctx->priv;

s->frames = av_calloc(s->n_frames, sizeof(*s->frames));
if (!s->frames) {
av_log(ctx, AV_LOG_ERROR,
"Allocation failure, try to lower the number of frames\n");
return AVERROR(ENOMEM);
}
av_log(ctx, AV_LOG_VERBOSE, "batch size: %d frames\n", s->n_frames);
return 0;
}







static double frame_sum_square_err(const int *hist, const double *median)
{
int i;
double err, sum_sq_err = 0;

for (i = 0; i < HIST_SIZE; i++) {
err = median[i] - (double)hist[i];
sum_sq_err += err*err;
}
return sum_sq_err;
}

static AVFrame *get_best_frame(AVFilterContext *ctx)
{
AVFrame *picref;
ThumbContext *s = ctx->priv;
int i, j, best_frame_idx = 0;
int nb_frames = s->n;
double avg_hist[HIST_SIZE] = {0}, sq_err, min_sq_err = -1;


for (j = 0; j < FF_ARRAY_ELEMS(avg_hist); j++) {
for (i = 0; i < nb_frames; i++)
avg_hist[j] += (double)s->frames[i].histogram[j];
avg_hist[j] /= nb_frames;
}


for (i = 0; i < nb_frames; i++) {
sq_err = frame_sum_square_err(s->frames[i].histogram, avg_hist);
if (i == 0 || sq_err < min_sq_err)
best_frame_idx = i, min_sq_err = sq_err;
}


for (i = 0; i < nb_frames; i++) {
memset(s->frames[i].histogram, 0, sizeof(s->frames[i].histogram));
if (i != best_frame_idx)
av_frame_free(&s->frames[i].buf);
}
s->n = 0;


picref = s->frames[best_frame_idx].buf;
av_log(ctx, AV_LOG_INFO, "frame id #%d (pts_time=%f) selected "
"from a set of %d images\n", best_frame_idx,
picref->pts * av_q2d(s->tb), nb_frames);
s->frames[best_frame_idx].buf = NULL;

return picref;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *frame)
{
int i, j;
AVFilterContext *ctx = inlink->dst;
ThumbContext *s = ctx->priv;
AVFilterLink *outlink = ctx->outputs[0];
int *hist = s->frames[s->n].histogram;
const uint8_t *p = frame->data[0];


s->frames[s->n].buf = frame;


for (j = 0; j < inlink->h; j++) {
for (i = 0; i < inlink->w; i++) {
hist[0*256 + p[i*3 ]]++;
hist[1*256 + p[i*3 + 1]]++;
hist[2*256 + p[i*3 + 2]]++;
}
p += frame->linesize[0];
}


s->n++;
if (s->n < s->n_frames)
return 0;

return ff_filter_frame(outlink, get_best_frame(ctx));
}

static av_cold void uninit(AVFilterContext *ctx)
{
int i;
ThumbContext *s = ctx->priv;
for (i = 0; i < s->n_frames && s->frames && s->frames[i].buf; i++)
av_frame_free(&s->frames[i].buf);
av_freep(&s->frames);
}

static int request_frame(AVFilterLink *link)
{
AVFilterContext *ctx = link->src;
ThumbContext *s = ctx->priv;
int ret = ff_request_frame(ctx->inputs[0]);

if (ret == AVERROR_EOF && s->n) {
ret = ff_filter_frame(link, get_best_frame(ctx));
if (ret < 0)
return ret;
ret = AVERROR_EOF;
}
if (ret < 0)
return ret;
return 0;
}

static int config_props(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
ThumbContext *s = ctx->priv;

s->tb = inlink->time_base;
return 0;
}

static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_RGB24, AV_PIX_FMT_BGR24,
AV_PIX_FMT_NONE
};
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}

static const AVFilterPad thumbnail_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_props,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad thumbnail_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.request_frame = request_frame,
},
{ NULL }
};

AVFilter ff_vf_thumbnail = {
.name = "thumbnail",
.description = NULL_IF_CONFIG_SMALL("Select the most representative frame in a given sequence of consecutive frames."),
.priv_size = sizeof(ThumbContext),
.init = init,
.uninit = uninit,
.query_formats = query_formats,
.inputs = thumbnail_inputs,
.outputs = thumbnail_outputs,
.priv_class = &thumbnail_class,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC,
};
