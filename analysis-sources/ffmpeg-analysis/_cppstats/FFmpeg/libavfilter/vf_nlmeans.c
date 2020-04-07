#include "libavutil/avassert.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avfilter.h"
#include "formats.h"
#include "internal.h"
#include "vf_nlmeans.h"
#include "video.h"
struct weighted_avg {
float total_weight;
float sum;
};
typedef struct NLMeansContext {
const AVClass *class;
int nb_planes;
int chroma_w, chroma_h;
double pdiff_scale; 
double sigma; 
int patch_size, patch_hsize; 
int patch_size_uv, patch_hsize_uv; 
int research_size, research_hsize; 
int research_size_uv, research_hsize_uv; 
uint32_t *ii_orig; 
uint32_t *ii; 
int ii_w, ii_h; 
ptrdiff_t ii_lz_32; 
struct weighted_avg *wa; 
ptrdiff_t wa_linesize; 
float *weight_lut; 
uint32_t max_meaningful_diff; 
NLMeansDSPContext dsp;
} NLMeansContext;
#define OFFSET(x) offsetof(NLMeansContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption nlmeans_options[] = {
{ "s", "denoising strength", OFFSET(sigma), AV_OPT_TYPE_DOUBLE, { .dbl = 1.0 }, 1.0, 30.0, FLAGS },
{ "p", "patch size", OFFSET(patch_size), AV_OPT_TYPE_INT, { .i64 = 3*2+1 }, 0, 99, FLAGS },
{ "pc", "patch size for chroma planes", OFFSET(patch_size_uv), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 99, FLAGS },
{ "r", "research window", OFFSET(research_size), AV_OPT_TYPE_INT, { .i64 = 7*2+1 }, 0, 99, FLAGS },
{ "rc", "research window for chroma planes", OFFSET(research_size_uv), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 99, FLAGS },
{ NULL }
};
AVFILTER_DEFINE_CLASS(nlmeans);
static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV411P,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P,
AV_PIX_FMT_YUV440P, AV_PIX_FMT_YUV444P,
AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_YUVJ440P,
AV_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUVJ420P,
AV_PIX_FMT_YUVJ411P,
AV_PIX_FMT_GRAY8, AV_PIX_FMT_GBRP,
AV_PIX_FMT_NONE
};
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}
static void compute_safe_ssd_integral_image_c(uint32_t *dst, ptrdiff_t dst_linesize_32,
const uint8_t *s1, ptrdiff_t linesize1,
const uint8_t *s2, ptrdiff_t linesize2,
int w, int h)
{
int x, y;
const uint32_t *dst_top = dst - dst_linesize_32;
av_assert2(!(w & 0xf) && w >= 16 && h >= 1);
for (y = 0; y < h; y++) {
for (x = 0; x < w; x += 4) {
const int d0 = s1[x ] - s2[x ];
const int d1 = s1[x + 1] - s2[x + 1];
const int d2 = s1[x + 2] - s2[x + 2];
const int d3 = s1[x + 3] - s2[x + 3];
dst[x ] = dst_top[x ] - dst_top[x - 1] + d0*d0;
dst[x + 1] = dst_top[x + 1] - dst_top[x ] + d1*d1;
dst[x + 2] = dst_top[x + 2] - dst_top[x + 1] + d2*d2;
dst[x + 3] = dst_top[x + 3] - dst_top[x + 2] + d3*d3;
dst[x ] += dst[x - 1];
dst[x + 1] += dst[x ];
dst[x + 2] += dst[x + 1];
dst[x + 3] += dst[x + 2];
}
s1 += linesize1;
s2 += linesize2;
dst += dst_linesize_32;
dst_top += dst_linesize_32;
}
}
static inline void compute_unsafe_ssd_integral_image(uint32_t *dst, ptrdiff_t dst_linesize_32,
int startx, int starty,
const uint8_t *src, ptrdiff_t linesize,
int offx, int offy, int r, int sw, int sh,
int w, int h)
{
int x, y;
for (y = starty; y < starty + h; y++) {
uint32_t acc = dst[y*dst_linesize_32 + startx - 1] - dst[(y-1)*dst_linesize_32 + startx - 1];
const int s1y = av_clip(y - r, 0, sh - 1);
const int s2y = av_clip(y - (r + offy), 0, sh - 1);
for (x = startx; x < startx + w; x++) {
const int s1x = av_clip(x - r, 0, sw - 1);
const int s2x = av_clip(x - (r + offx), 0, sw - 1);
const uint8_t v1 = src[s1y*linesize + s1x];
const uint8_t v2 = src[s2y*linesize + s2x];
const int d = v1 - v2;
acc += d * d;
dst[y*dst_linesize_32 + x] = dst[(y-1)*dst_linesize_32 + x] + acc;
}
}
}
static void compute_ssd_integral_image(const NLMeansDSPContext *dsp,
uint32_t *ii, ptrdiff_t ii_linesize_32,
const uint8_t *src, ptrdiff_t linesize, int offx, int offy,
int e, int w, int h)
{
const int ii_w = w + e*2;
const int ii_h = h + e*2;
const int s1x = e;
const int s1y = e;
const int s2x = e + offx;
const int s2y = e + offy;
const int startx_safe = FFMAX(s1x, s2x);
const int starty_safe = FFMAX(s1y, s2y);
const int u_endx_safe = FFMIN(s1x + w, s2x + w); 
const int endy_safe = FFMIN(s1y + h, s2y + h);
const int safe_pw = (u_endx_safe - startx_safe) & ~0xf;
const int safe_ph = endy_safe - starty_safe;
const int endx_safe = startx_safe + safe_pw;
compute_unsafe_ssd_integral_image(ii, ii_linesize_32,
0, 0,
src, linesize,
offx, offy, e, w, h,
ii_w, starty_safe);
compute_unsafe_ssd_integral_image(ii, ii_linesize_32,
0, starty_safe,
src, linesize,
offx, offy, e, w, h,
startx_safe, safe_ph);
av_assert1(startx_safe - s1x >= 0); av_assert1(startx_safe - s1x < w);
av_assert1(starty_safe - s1y >= 0); av_assert1(starty_safe - s1y < h);
av_assert1(startx_safe - s2x >= 0); av_assert1(startx_safe - s2x < w);
av_assert1(starty_safe - s2y >= 0); av_assert1(starty_safe - s2y < h);
if (safe_pw && safe_ph)
dsp->compute_safe_ssd_integral_image(ii + starty_safe*ii_linesize_32 + startx_safe, ii_linesize_32,
src + (starty_safe - s1y) * linesize + (startx_safe - s1x), linesize,
src + (starty_safe - s2y) * linesize + (startx_safe - s2x), linesize,
safe_pw, safe_ph);
compute_unsafe_ssd_integral_image(ii, ii_linesize_32,
endx_safe, starty_safe,
src, linesize,
offx, offy, e, w, h,
ii_w - endx_safe, safe_ph);
compute_unsafe_ssd_integral_image(ii, ii_linesize_32,
0, endy_safe,
src, linesize,
offx, offy, e, w, h,
ii_w, ii_h - endy_safe);
}
static int config_input(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
NLMeansContext *s = ctx->priv;
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(inlink->format);
const int e = FFMAX(s->research_hsize, s->research_hsize_uv)
+ FFMAX(s->patch_hsize, s->patch_hsize_uv);
s->chroma_w = AV_CEIL_RSHIFT(inlink->w, desc->log2_chroma_w);
s->chroma_h = AV_CEIL_RSHIFT(inlink->h, desc->log2_chroma_h);
s->nb_planes = av_pix_fmt_count_planes(inlink->format);
s->ii_w = inlink->w + e*2;
s->ii_h = inlink->h + e*2;
s->ii_lz_32 = FFALIGN(s->ii_w + 1, 4);
s->ii_orig = av_mallocz_array(s->ii_h + 1, s->ii_lz_32 * sizeof(*s->ii_orig));
if (!s->ii_orig)
return AVERROR(ENOMEM);
s->ii = s->ii_orig + s->ii_lz_32 + 1;
s->wa_linesize = inlink->w;
s->wa = av_malloc_array(s->wa_linesize, inlink->h * sizeof(*s->wa));
if (!s->wa)
return AVERROR(ENOMEM);
return 0;
}
struct thread_data {
const uint8_t *src;
ptrdiff_t src_linesize;
int startx, starty;
int endx, endy;
const uint32_t *ii_start;
int p;
};
static int nlmeans_slice(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs)
{
int x, y;
NLMeansContext *s = ctx->priv;
const struct thread_data *td = arg;
const ptrdiff_t src_linesize = td->src_linesize;
const int process_h = td->endy - td->starty;
const int slice_start = (process_h * jobnr ) / nb_jobs;
const int slice_end = (process_h * (jobnr+1)) / nb_jobs;
const int starty = td->starty + slice_start;
const int endy = td->starty + slice_end;
const int p = td->p;
const uint32_t *ii = td->ii_start + (starty - p - 1) * s->ii_lz_32 - p - 1;
const int dist_b = 2*p + 1;
const int dist_d = dist_b * s->ii_lz_32;
const int dist_e = dist_d + dist_b;
for (y = starty; y < endy; y++) {
const uint8_t *src = td->src + y*src_linesize;
struct weighted_avg *wa = s->wa + y*s->wa_linesize;
for (x = td->startx; x < td->endx; x++) {
const uint32_t a = ii[x];
const uint32_t b = ii[x + dist_b];
const uint32_t d = ii[x + dist_d];
const uint32_t e = ii[x + dist_e];
const uint32_t patch_diff_sq = e - d - b + a;
if (patch_diff_sq < s->max_meaningful_diff) {
const float weight = s->weight_lut[patch_diff_sq]; 
wa[x].total_weight += weight;
wa[x].sum += weight * src[x];
}
}
ii += s->ii_lz_32;
}
return 0;
}
static void weight_averages(uint8_t *dst, ptrdiff_t dst_linesize,
const uint8_t *src, ptrdiff_t src_linesize,
struct weighted_avg *wa, ptrdiff_t wa_linesize,
int w, int h)
{
int x, y;
for (y = 0; y < h; y++) {
for (x = 0; x < w; x++) {
wa[x].total_weight += 1.f;
wa[x].sum += 1.f * src[x];
dst[x] = av_clip_uint8(wa[x].sum / wa[x].total_weight + 0.5f);
}
dst += dst_linesize;
src += src_linesize;
wa += wa_linesize;
}
}
static int nlmeans_plane(AVFilterContext *ctx, int w, int h, int p, int r,
uint8_t *dst, ptrdiff_t dst_linesize,
const uint8_t *src, ptrdiff_t src_linesize)
{
int offx, offy;
NLMeansContext *s = ctx->priv;
const int e = r + p;
const uint32_t *centered_ii = s->ii + e*s->ii_lz_32 + e;
memset(s->wa, 0, s->wa_linesize * h * sizeof(*s->wa));
for (offy = -r; offy <= r; offy++) {
for (offx = -r; offx <= r; offx++) {
if (offx || offy) {
struct thread_data td = {
.src = src + offy*src_linesize + offx,
.src_linesize = src_linesize,
.startx = FFMAX(0, -offx),
.starty = FFMAX(0, -offy),
.endx = FFMIN(w, w - offx),
.endy = FFMIN(h, h - offy),
.ii_start = centered_ii + offy*s->ii_lz_32 + offx,
.p = p,
};
compute_ssd_integral_image(&s->dsp, s->ii, s->ii_lz_32,
src, src_linesize,
offx, offy, e, w, h);
ctx->internal->execute(ctx, nlmeans_slice, &td, NULL,
FFMIN(td.endy - td.starty, ff_filter_get_nb_threads(ctx)));
}
}
}
weight_averages(dst, dst_linesize, src, src_linesize,
s->wa, s->wa_linesize, w, h);
return 0;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
int i;
AVFilterContext *ctx = inlink->dst;
NLMeansContext *s = ctx->priv;
AVFilterLink *outlink = ctx->outputs[0];
AVFrame *out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out) {
av_frame_free(&in);
return AVERROR(ENOMEM);
}
av_frame_copy_props(out, in);
for (i = 0; i < s->nb_planes; i++) {
const int w = i ? s->chroma_w : inlink->w;
const int h = i ? s->chroma_h : inlink->h;
const int p = i ? s->patch_hsize_uv : s->patch_hsize;
const int r = i ? s->research_hsize_uv : s->research_hsize;
nlmeans_plane(ctx, w, h, p, r,
out->data[i], out->linesize[i],
in->data[i], in->linesize[i]);
}
av_frame_free(&in);
return ff_filter_frame(outlink, out);
}
#define CHECK_ODD_FIELD(field, name) do { if (!(s->field & 1)) { s->field |= 1; av_log(ctx, AV_LOG_WARNING, name " size must be odd, " "setting it to %d\n", s->field); } } while (0)
void ff_nlmeans_init(NLMeansDSPContext *dsp)
{
dsp->compute_safe_ssd_integral_image = compute_safe_ssd_integral_image_c;
if (ARCH_AARCH64)
ff_nlmeans_init_aarch64(dsp);
}
static av_cold int init(AVFilterContext *ctx)
{
int i;
NLMeansContext *s = ctx->priv;
const double h = s->sigma * 10.;
s->pdiff_scale = 1. / (h * h);
s->max_meaningful_diff = log(255.) / s->pdiff_scale;
s->weight_lut = av_calloc(s->max_meaningful_diff, sizeof(*s->weight_lut));
if (!s->weight_lut)
return AVERROR(ENOMEM);
for (i = 0; i < s->max_meaningful_diff; i++)
s->weight_lut[i] = exp(-i * s->pdiff_scale);
CHECK_ODD_FIELD(research_size, "Luma research window");
CHECK_ODD_FIELD(patch_size, "Luma patch");
if (!s->research_size_uv) s->research_size_uv = s->research_size;
if (!s->patch_size_uv) s->patch_size_uv = s->patch_size;
CHECK_ODD_FIELD(research_size_uv, "Chroma research window");
CHECK_ODD_FIELD(patch_size_uv, "Chroma patch");
s->research_hsize = s->research_size / 2;
s->research_hsize_uv = s->research_size_uv / 2;
s->patch_hsize = s->patch_size / 2;
s->patch_hsize_uv = s->patch_size_uv / 2;
av_log(ctx, AV_LOG_INFO, "Research window: %dx%d / %dx%d, patch size: %dx%d / %dx%d\n",
s->research_size, s->research_size, s->research_size_uv, s->research_size_uv,
s->patch_size, s->patch_size, s->patch_size_uv, s->patch_size_uv);
ff_nlmeans_init(&s->dsp);
return 0;
}
static av_cold void uninit(AVFilterContext *ctx)
{
NLMeansContext *s = ctx->priv;
av_freep(&s->weight_lut);
av_freep(&s->ii_orig);
av_freep(&s->wa);
}
static const AVFilterPad nlmeans_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_input,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad nlmeans_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};
AVFilter ff_vf_nlmeans = {
.name = "nlmeans",
.description = NULL_IF_CONFIG_SMALL("Non-local means denoiser."),
.priv_size = sizeof(NLMeansContext),
.init = init,
.uninit = uninit,
.query_formats = query_formats,
.inputs = nlmeans_inputs,
.outputs = nlmeans_outputs,
.priv_class = &nlmeans_class,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_GENERIC | AVFILTER_FLAG_SLICE_THREADS,
};
