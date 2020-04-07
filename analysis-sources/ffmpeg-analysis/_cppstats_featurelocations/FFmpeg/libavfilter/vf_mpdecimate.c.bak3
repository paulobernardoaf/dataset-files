

























#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixelutils.h"
#include "libavutil/timestamp.h"
#include "avfilter.h"
#include "internal.h"
#include "formats.h"
#include "video.h"

typedef struct DecimateContext {
const AVClass *class;
int lo, hi; 


float frac; 

int max_drop_count; 


int drop_count; 


int hsub, vsub; 
AVFrame *ref; 
av_pixelutils_sad_fn sad; 
} DecimateContext;

#define OFFSET(x) offsetof(DecimateContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption mpdecimate_options[] = {
{ "max", "set the maximum number of consecutive dropped frames (positive), or the minimum interval between dropped frames (negative)",
OFFSET(max_drop_count), AV_OPT_TYPE_INT, {.i64=0}, INT_MIN, INT_MAX, FLAGS },
{ "hi", "set high dropping threshold", OFFSET(hi), AV_OPT_TYPE_INT, {.i64=64*12}, INT_MIN, INT_MAX, FLAGS },
{ "lo", "set low dropping threshold", OFFSET(lo), AV_OPT_TYPE_INT, {.i64=64*5}, INT_MIN, INT_MAX, FLAGS },
{ "frac", "set fraction dropping threshold", OFFSET(frac), AV_OPT_TYPE_FLOAT, {.dbl=0.33}, 0, 1, FLAGS },
{ NULL }
};

AVFILTER_DEFINE_CLASS(mpdecimate);




static int diff_planes(AVFilterContext *ctx,
uint8_t *cur, int cur_linesize,
uint8_t *ref, int ref_linesize,
int w, int h)
{
DecimateContext *decimate = ctx->priv;

int x, y;
int d, c = 0;
int t = (w/16)*(h/16)*decimate->frac;


for (y = 0; y < h-7; y += 4) {
for (x = 8; x < w-7; x += 4) {
d = decimate->sad(cur + y*cur_linesize + x, cur_linesize,
ref + y*ref_linesize + x, ref_linesize);
if (d > decimate->hi) {
av_log(ctx, AV_LOG_DEBUG, "%d>=hi ", d);
return 1;
}
if (d > decimate->lo) {
c++;
if (c > t) {
av_log(ctx, AV_LOG_DEBUG, "lo:%d>=%d ", c, t);
return 1;
}
}
}
}

av_log(ctx, AV_LOG_DEBUG, "lo:%d<%d ", c, t);
return 0;
}





static int decimate_frame(AVFilterContext *ctx,
AVFrame *cur, AVFrame *ref)
{
DecimateContext *decimate = ctx->priv;
int plane;

if (decimate->max_drop_count > 0 &&
decimate->drop_count >= decimate->max_drop_count)
return 0;
if (decimate->max_drop_count < 0 &&
(decimate->drop_count-1) > decimate->max_drop_count)
return 0;

for (plane = 0; ref->data[plane] && ref->linesize[plane]; plane++) {





int vsub = plane == 1 || plane == 2 ? decimate->vsub : 0;
int hsub = plane == 1 || plane == 2 ? decimate->hsub : 0;
if (diff_planes(ctx,
cur->data[plane], cur->linesize[plane],
ref->data[plane], ref->linesize[plane],
AV_CEIL_RSHIFT(ref->width, hsub),
AV_CEIL_RSHIFT(ref->height, vsub))) {
emms_c();
return 0;
}
}

emms_c();
return 1;
}

static av_cold int init(AVFilterContext *ctx)
{
DecimateContext *decimate = ctx->priv;

decimate->sad = av_pixelutils_get_sad_fn(3, 3, 0, ctx); 
if (!decimate->sad)
return AVERROR(EINVAL);

av_log(ctx, AV_LOG_VERBOSE, "max_drop_count:%d hi:%d lo:%d frac:%f\n",
decimate->max_drop_count, decimate->hi, decimate->lo, decimate->frac);

return 0;
}

static av_cold void uninit(AVFilterContext *ctx)
{
DecimateContext *decimate = ctx->priv;
av_frame_free(&decimate->ref);
}

static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUV422P,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV411P,
AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV440P,
AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_YUVJ422P,
AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ440P,
AV_PIX_FMT_YUVA420P,

AV_PIX_FMT_GBRP,

AV_PIX_FMT_YUVA444P,
AV_PIX_FMT_YUVA422P,

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
DecimateContext *decimate = ctx->priv;
const AVPixFmtDescriptor *pix_desc = av_pix_fmt_desc_get(inlink->format);
decimate->hsub = pix_desc->log2_chroma_w;
decimate->vsub = pix_desc->log2_chroma_h;

return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *cur)
{
DecimateContext *decimate = inlink->dst->priv;
AVFilterLink *outlink = inlink->dst->outputs[0];
int ret;

if (decimate->ref && decimate_frame(inlink->dst, cur, decimate->ref)) {
decimate->drop_count = FFMAX(1, decimate->drop_count+1);
} else {
av_frame_free(&decimate->ref);
decimate->ref = cur;
decimate->drop_count = FFMIN(-1, decimate->drop_count-1);

if ((ret = ff_filter_frame(outlink, av_frame_clone(cur))) < 0)
return ret;
}

av_log(inlink->dst, AV_LOG_DEBUG,
"%s pts:%s pts_time:%s drop_count:%d\n",
decimate->drop_count > 0 ? "drop" : "keep",
av_ts2str(cur->pts), av_ts2timestr(cur->pts, &inlink->time_base),
decimate->drop_count);

if (decimate->drop_count > 0)
av_frame_free(&cur);

return 0;
}

static const AVFilterPad mpdecimate_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_input,
.filter_frame = filter_frame,
},
{ NULL }
};

static const AVFilterPad mpdecimate_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

AVFilter ff_vf_mpdecimate = {
.name = "mpdecimate",
.description = NULL_IF_CONFIG_SMALL("Remove near-duplicate frames."),
.init = init,
.uninit = uninit,
.priv_size = sizeof(DecimateContext),
.priv_class = &mpdecimate_class,
.query_formats = query_formats,
.inputs = mpdecimate_inputs,
.outputs = mpdecimate_outputs,
};
