#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avfilter.h"
#include "internal.h"
#include "tinterlace.h"
#define OFFSET(x) offsetof(TInterlaceContext, x)
#define FLAGS AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_VIDEO_PARAM
static const AVOption tinterlace_options[] = {
{"mode", "select interlace mode", OFFSET(mode), AV_OPT_TYPE_INT, {.i64=MODE_MERGE}, 0, MODE_NB-1, FLAGS, "mode"},
{"merge", "merge fields", 0, AV_OPT_TYPE_CONST, {.i64=MODE_MERGE}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"drop_even", "drop even fields", 0, AV_OPT_TYPE_CONST, {.i64=MODE_DROP_EVEN}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"drop_odd", "drop odd fields", 0, AV_OPT_TYPE_CONST, {.i64=MODE_DROP_ODD}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"pad", "pad alternate lines with black", 0, AV_OPT_TYPE_CONST, {.i64=MODE_PAD}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"interleave_top", "interleave top and bottom fields", 0, AV_OPT_TYPE_CONST, {.i64=MODE_INTERLEAVE_TOP}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"interleave_bottom", "interleave bottom and top fields", 0, AV_OPT_TYPE_CONST, {.i64=MODE_INTERLEAVE_BOTTOM}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"interlacex2", "interlace fields from two consecutive frames", 0, AV_OPT_TYPE_CONST, {.i64=MODE_INTERLACEX2}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"mergex2", "merge fields keeping same frame rate", 0, AV_OPT_TYPE_CONST, {.i64=MODE_MERGEX2}, INT_MIN, INT_MAX, FLAGS, "mode"},
{"flags", "set flags", OFFSET(flags), AV_OPT_TYPE_FLAGS, {.i64 = 0}, 0, INT_MAX, 0, "flags" },
{"low_pass_filter", "enable vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_VLPF}, INT_MIN, INT_MAX, FLAGS, "flags" },
{"vlpf", "enable vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_VLPF}, INT_MIN, INT_MAX, FLAGS, "flags" },
{"complex_filter", "enable complex vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_CVLPF},INT_MIN, INT_MAX, FLAGS, "flags" },
{"cvlpf", "enable complex vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_CVLPF},INT_MIN, INT_MAX, FLAGS, "flags" },
{"exact_tb", "force a timebase which can represent timestamps exactly", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_EXACT_TB}, INT_MIN, INT_MAX, FLAGS, "flags" },
{"bypass_il", "bypass already interlaced frames", 0, AV_OPT_TYPE_CONST, {.i64 = TINTERLACE_FLAG_BYPASS_IL}, INT_MIN, INT_MAX, FLAGS, "flags" },
{NULL}
};
AVFILTER_DEFINE_CLASS(tinterlace);
static const AVOption interlace_options[] = {
{ "scan", "scanning mode", OFFSET(mode), AV_OPT_TYPE_INT, {.i64 = MODE_TFF}, 0, 1, FLAGS, "mode"},
{ "tff", "top field first", 0, AV_OPT_TYPE_CONST, {.i64 = MODE_TFF}, INT_MIN, INT_MAX, FLAGS, .unit = "mode"},
{ "bff", "bottom field first", 0, AV_OPT_TYPE_CONST, {.i64 = MODE_BFF}, INT_MIN, INT_MAX, FLAGS, .unit = "mode"},
{ "lowpass", "set vertical low-pass filter", OFFSET(lowpass), AV_OPT_TYPE_INT, {.i64 = VLPF_LIN}, 0, 2, FLAGS, "lowpass" },
{ "off", "disable vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = VLPF_OFF}, INT_MIN, INT_MAX, FLAGS, "lowpass" },
{ "linear", "linear vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = VLPF_LIN}, INT_MIN, INT_MAX, FLAGS, "lowpass" },
{ "complex", "complex vertical low-pass filter", 0, AV_OPT_TYPE_CONST, {.i64 = VLPF_CMP}, INT_MIN, INT_MAX, FLAGS, "lowpass" },
{ NULL }
};
AVFILTER_DEFINE_CLASS(interlace);
#define FULL_SCALE_YUVJ_FORMATS AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUVJ444P, AV_PIX_FMT_YUVJ440P
static const enum AVPixelFormat full_scale_yuvj_pix_fmts[] = {
FULL_SCALE_YUVJ_FORMATS, AV_PIX_FMT_NONE
};
static const AVRational standard_tbs[] = {
{1, 25},
{1, 30},
{1001, 30000},
};
static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pix_fmts[] = {
AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUV411P,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P,
AV_PIX_FMT_YUV440P, AV_PIX_FMT_YUV444P,
AV_PIX_FMT_YUV420P10LE, AV_PIX_FMT_YUV422P10LE,
AV_PIX_FMT_YUV440P10LE, AV_PIX_FMT_YUV444P10LE,
AV_PIX_FMT_YUV420P12LE, AV_PIX_FMT_YUV422P12LE,
AV_PIX_FMT_YUV440P12LE, AV_PIX_FMT_YUV444P12LE,
AV_PIX_FMT_YUVA420P, AV_PIX_FMT_YUVA422P, AV_PIX_FMT_YUVA444P,
AV_PIX_FMT_YUVA420P10LE, AV_PIX_FMT_YUVA422P10LE, AV_PIX_FMT_YUVA444P10LE,
AV_PIX_FMT_GRAY8, FULL_SCALE_YUVJ_FORMATS,
AV_PIX_FMT_NONE
};
AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
if (!fmts_list)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, fmts_list);
}
static void lowpass_line_c(uint8_t *dstp, ptrdiff_t width, const uint8_t *srcp,
ptrdiff_t mref, ptrdiff_t pref, int clip_max)
{
const uint8_t *srcp_above = srcp + mref;
const uint8_t *srcp_below = srcp + pref;
int i;
for (i = 0; i < width; i++) {
dstp[i] = (1 + srcp[i] + srcp[i] + srcp_above[i] + srcp_below[i]) >> 2;
}
}
static void lowpass_line_c_16(uint8_t *dst8, ptrdiff_t width, const uint8_t *src8,
ptrdiff_t mref, ptrdiff_t pref, int clip_max)
{
uint16_t *dstp = (uint16_t *)dst8;
const uint16_t *srcp = (const uint16_t *)src8;
const uint16_t *srcp_above = srcp + mref / 2;
const uint16_t *srcp_below = srcp + pref / 2;
int i, src_x;
for (i = 0; i < width; i++) {
src_x = av_le2ne16(srcp[i]) << 1;
dstp[i] = av_le2ne16((1 + src_x + av_le2ne16(srcp_above[i])
+ av_le2ne16(srcp_below[i])) >> 2);
}
}
static void lowpass_line_complex_c(uint8_t *dstp, ptrdiff_t width, const uint8_t *srcp,
ptrdiff_t mref, ptrdiff_t pref, int clip_max)
{
const uint8_t *srcp_above = srcp + mref;
const uint8_t *srcp_below = srcp + pref;
const uint8_t *srcp_above2 = srcp + mref * 2;
const uint8_t *srcp_below2 = srcp + pref * 2;
int i, src_x, src_ab;
for (i = 0; i < width; i++) {
src_x = srcp[i] << 1;
src_ab = srcp_above[i] + srcp_below[i];
dstp[i] = av_clip_uint8((4 + ((srcp[i] + src_x + src_ab) << 1)
- srcp_above2[i] - srcp_below2[i]) >> 3);
if (src_ab > src_x) {
if (dstp[i] < srcp[i])
dstp[i] = srcp[i];
} else if (dstp[i] > srcp[i])
dstp[i] = srcp[i];
}
}
static void lowpass_line_complex_c_16(uint8_t *dst8, ptrdiff_t width, const uint8_t *src8,
ptrdiff_t mref, ptrdiff_t pref, int clip_max)
{
uint16_t *dstp = (uint16_t *)dst8;
const uint16_t *srcp = (const uint16_t *)src8;
const uint16_t *srcp_above = srcp + mref / 2;
const uint16_t *srcp_below = srcp + pref / 2;
const uint16_t *srcp_above2 = srcp + mref;
const uint16_t *srcp_below2 = srcp + pref;
int i, dst_le, src_le, src_x, src_ab;
for (i = 0; i < width; i++) {
src_le = av_le2ne16(srcp[i]);
src_x = src_le << 1;
src_ab = av_le2ne16(srcp_above[i]) + av_le2ne16(srcp_below[i]);
dst_le = av_clip((4 + ((src_le + src_x + src_ab) << 1)
- av_le2ne16(srcp_above2[i])
- av_le2ne16(srcp_below2[i])) >> 3, 0, clip_max);
if (src_ab > src_x) {
if (dst_le < src_le)
dstp[i] = av_le2ne16(src_le);
else
dstp[i] = av_le2ne16(dst_le);
} else if (dst_le > src_le) {
dstp[i] = av_le2ne16(src_le);
} else
dstp[i] = av_le2ne16(dst_le);
}
}
static av_cold void uninit(AVFilterContext *ctx)
{
TInterlaceContext *tinterlace = ctx->priv;
av_frame_free(&tinterlace->cur );
av_frame_free(&tinterlace->next);
av_freep(&tinterlace->black_data[0]);
}
static int config_out_props(AVFilterLink *outlink)
{
AVFilterContext *ctx = outlink->src;
AVFilterLink *inlink = outlink->src->inputs[0];
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(outlink->format);
TInterlaceContext *tinterlace = ctx->priv;
int i;
tinterlace->vsub = desc->log2_chroma_h;
outlink->w = inlink->w;
outlink->h = tinterlace->mode == MODE_MERGE || tinterlace->mode == MODE_PAD || tinterlace->mode == MODE_MERGEX2?
inlink->h*2 : inlink->h;
if (tinterlace->mode == MODE_MERGE || tinterlace->mode == MODE_PAD || tinterlace->mode == MODE_MERGEX2)
outlink->sample_aspect_ratio = av_mul_q(inlink->sample_aspect_ratio,
av_make_q(2, 1));
if (tinterlace->mode == MODE_PAD) {
uint8_t black[4] = { 0, 0, 0, 16 };
int ret;
ff_draw_init(&tinterlace->draw, outlink->format, 0);
ff_draw_color(&tinterlace->draw, &tinterlace->color, black);
if (ff_fmt_is_in(outlink->format, full_scale_yuvj_pix_fmts))
tinterlace->color.comp[0].u8[0] = 0;
ret = av_image_alloc(tinterlace->black_data, tinterlace->black_linesize,
outlink->w, outlink->h, outlink->format, 16);
if (ret < 0)
return ret;
ff_fill_rectangle(&tinterlace->draw, &tinterlace->color, tinterlace->black_data,
tinterlace->black_linesize, 0, 0, outlink->w, outlink->h);
}
if (tinterlace->flags & (TINTERLACE_FLAG_VLPF | TINTERLACE_FLAG_CVLPF)
&& !(tinterlace->mode == MODE_INTERLEAVE_TOP
|| tinterlace->mode == MODE_INTERLEAVE_BOTTOM)) {
av_log(ctx, AV_LOG_WARNING, "low_pass_filter flags ignored with mode %d\n",
tinterlace->mode);
tinterlace->flags &= ~(TINTERLACE_FLAG_VLPF | TINTERLACE_FLAG_CVLPF);
}
tinterlace->preout_time_base = inlink->time_base;
if (tinterlace->mode == MODE_INTERLACEX2) {
tinterlace->preout_time_base.den *= 2;
outlink->frame_rate = av_mul_q(inlink->frame_rate, (AVRational){2,1});
outlink->time_base = av_mul_q(inlink->time_base , (AVRational){1,2});
} else if (tinterlace->mode == MODE_MERGEX2) {
outlink->frame_rate = inlink->frame_rate;
outlink->time_base = inlink->time_base;
} else if (tinterlace->mode != MODE_PAD) {
outlink->frame_rate = av_mul_q(inlink->frame_rate, (AVRational){1,2});
outlink->time_base = av_mul_q(inlink->time_base , (AVRational){2,1});
}
for (i = 0; i<FF_ARRAY_ELEMS(standard_tbs); i++){
if (!av_cmp_q(standard_tbs[i], outlink->time_base))
break;
}
if (i == FF_ARRAY_ELEMS(standard_tbs) ||
(tinterlace->flags & TINTERLACE_FLAG_EXACT_TB))
outlink->time_base = tinterlace->preout_time_base;
tinterlace->csp = av_pix_fmt_desc_get(outlink->format);
if (tinterlace->flags & TINTERLACE_FLAG_CVLPF) {
if (tinterlace->csp->comp[0].depth > 8)
tinterlace->lowpass_line = lowpass_line_complex_c_16;
else
tinterlace->lowpass_line = lowpass_line_complex_c;
if (ARCH_X86)
ff_tinterlace_init_x86(tinterlace);
} else if (tinterlace->flags & TINTERLACE_FLAG_VLPF) {
if (tinterlace->csp->comp[0].depth > 8)
tinterlace->lowpass_line = lowpass_line_c_16;
else
tinterlace->lowpass_line = lowpass_line_c;
if (ARCH_X86)
ff_tinterlace_init_x86(tinterlace);
}
av_log(ctx, AV_LOG_VERBOSE, "mode:%d filter:%s h:%d -> h:%d\n", tinterlace->mode,
(tinterlace->flags & TINTERLACE_FLAG_CVLPF) ? "complex" :
(tinterlace->flags & TINTERLACE_FLAG_VLPF) ? "linear" : "off",
inlink->h, outlink->h);
return 0;
}
#define FIELD_UPPER 0
#define FIELD_LOWER 1
#define FIELD_UPPER_AND_LOWER 2
static inline
void copy_picture_field(TInterlaceContext *tinterlace,
uint8_t *dst[4], int dst_linesize[4],
const uint8_t *src[4], int src_linesize[4],
enum AVPixelFormat format, int w, int src_h,
int src_field, int interleave, int dst_field,
int flags)
{
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(format);
int hsub = desc->log2_chroma_w;
int plane, vsub = desc->log2_chroma_h;
int k = src_field == FIELD_UPPER_AND_LOWER ? 1 : 2;
int h;
for (plane = 0; plane < desc->nb_components; plane++) {
int lines = plane == 1 || plane == 2 ? AV_CEIL_RSHIFT(src_h, vsub) : src_h;
int cols = plane == 1 || plane == 2 ? AV_CEIL_RSHIFT( w, hsub) : w;
uint8_t *dstp = dst[plane];
const uint8_t *srcp = src[plane];
int srcp_linesize = src_linesize[plane] * k;
int dstp_linesize = dst_linesize[plane] * (interleave ? 2 : 1);
int clip_max = (1 << tinterlace->csp->comp[plane].depth) - 1;
lines = (lines + (src_field == FIELD_UPPER)) / k;
if (src_field == FIELD_LOWER)
srcp += src_linesize[plane];
if (interleave && dst_field == FIELD_LOWER)
dstp += dst_linesize[plane];
if (flags & (TINTERLACE_FLAG_VLPF | TINTERLACE_FLAG_CVLPF)) {
int x = !!(flags & TINTERLACE_FLAG_CVLPF);
for (h = lines; h > 0; h--) {
ptrdiff_t pref = src_linesize[plane];
ptrdiff_t mref = -pref;
if (h >= (lines - x)) mref = 0; 
else if (h <= (1 + x)) pref = 0; 
tinterlace->lowpass_line(dstp, cols, srcp, mref, pref, clip_max);
dstp += dstp_linesize;
srcp += srcp_linesize;
}
} else {
if (tinterlace->csp->comp[plane].depth > 8)
cols *= 2;
av_image_copy_plane(dstp, dstp_linesize, srcp, srcp_linesize, cols, lines);
}
}
}
static int filter_frame(AVFilterLink *inlink, AVFrame *picref)
{
AVFilterContext *ctx = inlink->dst;
AVFilterLink *outlink = ctx->outputs[0];
TInterlaceContext *tinterlace = ctx->priv;
AVFrame *cur, *next, *out;
int field, tff, ret;
av_frame_free(&tinterlace->cur);
tinterlace->cur = tinterlace->next;
tinterlace->next = picref;
cur = tinterlace->cur;
next = tinterlace->next;
if (!tinterlace->cur)
return 0;
switch (tinterlace->mode) {
case MODE_MERGEX2: 
case MODE_MERGE: 
out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out)
return AVERROR(ENOMEM);
av_frame_copy_props(out, cur);
out->height = outlink->h;
out->interlaced_frame = 1;
out->top_field_first = 1;
out->sample_aspect_ratio = av_mul_q(cur->sample_aspect_ratio, av_make_q(2, 1));
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)cur->data, cur->linesize,
inlink->format, inlink->w, inlink->h,
FIELD_UPPER_AND_LOWER, 1, tinterlace->mode == MODE_MERGEX2 ? inlink->frame_count_out & 1 ? FIELD_LOWER : FIELD_UPPER : FIELD_UPPER, tinterlace->flags);
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)next->data, next->linesize,
inlink->format, inlink->w, inlink->h,
FIELD_UPPER_AND_LOWER, 1, tinterlace->mode == MODE_MERGEX2 ? inlink->frame_count_out & 1 ? FIELD_UPPER : FIELD_LOWER : FIELD_LOWER, tinterlace->flags);
if (tinterlace->mode != MODE_MERGEX2)
av_frame_free(&tinterlace->next);
break;
case MODE_DROP_ODD: 
case MODE_DROP_EVEN: 
out = av_frame_clone(tinterlace->mode == MODE_DROP_EVEN ? cur : next);
if (!out)
return AVERROR(ENOMEM);
av_frame_free(&tinterlace->next);
break;
case MODE_PAD: 
out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out)
return AVERROR(ENOMEM);
av_frame_copy_props(out, cur);
out->height = outlink->h;
out->sample_aspect_ratio = av_mul_q(cur->sample_aspect_ratio, av_make_q(2, 1));
field = (1 + tinterlace->frame) & 1 ? FIELD_UPPER : FIELD_LOWER;
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)cur->data, cur->linesize,
inlink->format, inlink->w, inlink->h,
FIELD_UPPER_AND_LOWER, 1, field, tinterlace->flags);
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)tinterlace->black_data, tinterlace->black_linesize,
inlink->format, inlink->w, inlink->h,
FIELD_UPPER_AND_LOWER, 1, !field, tinterlace->flags);
break;
case MODE_INTERLEAVE_TOP: 
case MODE_INTERLEAVE_BOTTOM: 
if ((tinterlace->flags & TINTERLACE_FLAG_BYPASS_IL) && cur->interlaced_frame) {
av_log(ctx, AV_LOG_WARNING,
"video is already interlaced, adjusting framerate only\n");
out = av_frame_clone(cur);
if (!out)
return AVERROR(ENOMEM);
out->pts /= 2; 
ret = ff_filter_frame(outlink, out);
return ret;
}
tff = tinterlace->mode == MODE_INTERLEAVE_TOP;
out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out)
return AVERROR(ENOMEM);
av_frame_copy_props(out, cur);
out->interlaced_frame = 1;
out->top_field_first = tff;
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)cur->data, cur->linesize,
inlink->format, inlink->w, inlink->h,
tff ? FIELD_UPPER : FIELD_LOWER, 1, tff ? FIELD_UPPER : FIELD_LOWER,
tinterlace->flags);
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)next->data, next->linesize,
inlink->format, inlink->w, inlink->h,
tff ? FIELD_LOWER : FIELD_UPPER, 1, tff ? FIELD_LOWER : FIELD_UPPER,
tinterlace->flags);
av_frame_free(&tinterlace->next);
break;
case MODE_INTERLACEX2: 
out = av_frame_clone(cur);
if (!out)
return AVERROR(ENOMEM);
out->interlaced_frame = 1;
if (cur->pts != AV_NOPTS_VALUE)
out->pts = cur->pts*2;
out->pts = av_rescale_q(out->pts, tinterlace->preout_time_base, outlink->time_base);
if ((ret = ff_filter_frame(outlink, out)) < 0)
return ret;
tff = next->top_field_first;
out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out)
return AVERROR(ENOMEM);
av_frame_copy_props(out, next);
out->interlaced_frame = 1;
out->top_field_first = !tff;
if (next->pts != AV_NOPTS_VALUE && cur->pts != AV_NOPTS_VALUE)
out->pts = cur->pts + next->pts;
else
out->pts = AV_NOPTS_VALUE;
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)cur->data, cur->linesize,
inlink->format, inlink->w, inlink->h,
tff ? FIELD_LOWER : FIELD_UPPER, 1, tff ? FIELD_LOWER : FIELD_UPPER,
tinterlace->flags);
copy_picture_field(tinterlace, out->data, out->linesize,
(const uint8_t **)next->data, next->linesize,
inlink->format, inlink->w, inlink->h,
tff ? FIELD_UPPER : FIELD_LOWER, 1, tff ? FIELD_UPPER : FIELD_LOWER,
tinterlace->flags);
break;
default:
av_assert0(0);
}
out->pts = av_rescale_q(out->pts, tinterlace->preout_time_base, outlink->time_base);
ret = ff_filter_frame(outlink, out);
tinterlace->frame++;
return ret;
}
static int init_interlace(AVFilterContext *ctx)
{
TInterlaceContext *tinterlace = ctx->priv;
if (tinterlace->mode <= MODE_BFF)
tinterlace->mode += MODE_INTERLEAVE_TOP;
tinterlace->flags |= TINTERLACE_FLAG_BYPASS_IL;
if (tinterlace->lowpass == VLPF_LIN)
tinterlace->flags |= TINTERLACE_FLAG_VLPF;
if (tinterlace->lowpass == VLPF_CMP)
tinterlace->flags |= TINTERLACE_FLAG_CVLPF;
return 0;
}
static const AVFilterPad tinterlace_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad tinterlace_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.config_props = config_out_props,
},
{ NULL }
};
AVFilter ff_vf_tinterlace = {
.name = "tinterlace",
.description = NULL_IF_CONFIG_SMALL("Perform temporal field interlacing."),
.priv_size = sizeof(TInterlaceContext),
.uninit = uninit,
.query_formats = query_formats,
.inputs = tinterlace_inputs,
.outputs = tinterlace_outputs,
.priv_class = &tinterlace_class,
};
AVFilter ff_vf_interlace = {
.name = "interlace",
.description = NULL_IF_CONFIG_SMALL("Convert progressive video into interlaced."),
.priv_size = sizeof(TInterlaceContext),
.init = init_interlace,
.uninit = uninit,
.query_formats = query_formats,
.inputs = tinterlace_inputs,
.outputs = tinterlace_outputs,
.priv_class = &interlace_class,
};
