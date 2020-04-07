









































































#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "avfilter.h"
#include "drawutils.h"
#include "formats.h"
#include "internal.h"
#include "video.h"

typedef struct NormalizeHistory {
uint16_t *history; 
uint64_t history_sum; 
} NormalizeHistory;

typedef struct NormalizeLocal {
uint16_t in; 
float smoothed; 
float out; 
} NormalizeLocal;

typedef struct NormalizeContext {
const AVClass *class;


uint8_t blackpt[4];
uint8_t whitept[4];
int smoothing;
float independence;
float strength;

uint8_t co[4]; 
int depth;
int sblackpt[4];
int swhitept[4];
int num_components; 
int step;
int history_len; 
int frame_num; 


NormalizeHistory min[3], max[3]; 
uint16_t *history_mem; 

uint16_t lut[3][65536]; 

void (*find_min_max)(struct NormalizeContext *s, AVFrame *in, NormalizeLocal min[3], NormalizeLocal max[3]);
void (*process)(struct NormalizeContext *s, AVFrame *in, AVFrame *out);
} NormalizeContext;

#define OFFSET(x) offsetof(NormalizeContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM
#define FLAGSR AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM|AV_OPT_FLAG_RUNTIME_PARAM

static const AVOption normalize_options[] = {
{ "blackpt", "output color to which darkest input color is mapped", OFFSET(blackpt), AV_OPT_TYPE_COLOR, { .str = "black" }, 0, 0, FLAGSR },
{ "whitept", "output color to which brightest input color is mapped", OFFSET(whitept), AV_OPT_TYPE_COLOR, { .str = "white" }, 0, 0, FLAGSR },
{ "smoothing", "amount of temporal smoothing of the input range, to reduce flicker", OFFSET(smoothing), AV_OPT_TYPE_INT, {.i64=0}, 0, INT_MAX/8, FLAGS },
{ "independence", "proportion of independent to linked channel normalization", OFFSET(independence), AV_OPT_TYPE_FLOAT, {.dbl=1.0}, 0.0, 1.0, FLAGSR },
{ "strength", "strength of filter, from no effect to full normalization", OFFSET(strength), AV_OPT_TYPE_FLOAT, {.dbl=1.0}, 0.0, 1.0, FLAGSR },
{ NULL }
};

AVFILTER_DEFINE_CLASS(normalize);

static void find_min_max(NormalizeContext *s, AVFrame *in, NormalizeLocal min[3], NormalizeLocal max[3])
{
for (int c = 0; c < 3; c++)
min[c].in = max[c].in = in->data[0][s->co[c]];
for (int y = 0; y < in->height; y++) {
uint8_t *inp = in->data[0] + y * in->linesize[0];
for (int x = 0; x < in->width; x++) {
for (int c = 0; c < 3; c++) {
min[c].in = FFMIN(min[c].in, inp[s->co[c]]);
max[c].in = FFMAX(max[c].in, inp[s->co[c]]);
}
inp += s->step;
}
}
}

static void process(NormalizeContext *s, AVFrame *in, AVFrame *out)
{
for (int y = 0; y < in->height; y++) {
uint8_t *inp = in->data[0] + y * in->linesize[0];
uint8_t *outp = out->data[0] + y * out->linesize[0];
for (int x = 0; x < in->width; x++) {
for (int c = 0; c < 3; c++)
outp[s->co[c]] = s->lut[c][inp[s->co[c]]];
if (s->num_components == 4)

outp[s->co[3]] = inp[s->co[3]];
inp += s->step;
outp += s->step;
}
}
}

static void find_min_max_planar(NormalizeContext *s, AVFrame *in, NormalizeLocal min[3], NormalizeLocal max[3])
{
min[0].in = max[0].in = in->data[2][0];
min[1].in = max[1].in = in->data[0][0];
min[2].in = max[2].in = in->data[1][0];
for (int y = 0; y < in->height; y++) {
uint8_t *inrp = in->data[2] + y * in->linesize[2];
uint8_t *ingp = in->data[0] + y * in->linesize[0];
uint8_t *inbp = in->data[1] + y * in->linesize[1];
for (int x = 0; x < in->width; x++) {
min[0].in = FFMIN(min[0].in, inrp[x]);
max[0].in = FFMAX(max[0].in, inrp[x]);
min[1].in = FFMIN(min[1].in, ingp[x]);
max[1].in = FFMAX(max[1].in, ingp[x]);
min[2].in = FFMIN(min[2].in, inbp[x]);
max[2].in = FFMAX(max[2].in, inbp[x]);
}
}
}

static void process_planar(NormalizeContext *s, AVFrame *in, AVFrame *out)
{
for (int y = 0; y < in->height; y++) {
uint8_t *inrp = in->data[2] + y * in->linesize[2];
uint8_t *ingp = in->data[0] + y * in->linesize[0];
uint8_t *inbp = in->data[1] + y * in->linesize[1];
uint8_t *inap = in->data[3] + y * in->linesize[3];
uint8_t *outrp = out->data[2] + y * out->linesize[2];
uint8_t *outgp = out->data[0] + y * out->linesize[0];
uint8_t *outbp = out->data[1] + y * out->linesize[1];
uint8_t *outap = out->data[3] + y * out->linesize[3];
for (int x = 0; x < in->width; x++) {
outrp[x] = s->lut[0][inrp[x]];
outgp[x] = s->lut[1][ingp[x]];
outbp[x] = s->lut[2][inbp[x]];
if (s->num_components == 4)
outap[x] = inap[x];
}
}
}

static void find_min_max_16(NormalizeContext *s, AVFrame *in, NormalizeLocal min[3], NormalizeLocal max[3])
{
for (int c = 0; c < 3; c++)
min[c].in = max[c].in = AV_RN16(in->data[0] + 2 * s->co[c]);
for (int y = 0; y < in->height; y++) {
uint16_t *inp = (uint16_t *)(in->data[0] + y * in->linesize[0]);
for (int x = 0; x < in->width; x++) {
for (int c = 0; c < 3; c++) {
min[c].in = FFMIN(min[c].in, inp[s->co[c]]);
max[c].in = FFMAX(max[c].in, inp[s->co[c]]);
}
inp += s->step;
}
}
}

static void process_16(NormalizeContext *s, AVFrame *in, AVFrame *out)
{
for (int y = 0; y < in->height; y++) {
uint16_t *inp = (uint16_t *)(in->data[0] + y * in->linesize[0]);
uint16_t *outp = (uint16_t *)(out->data[0] + y * out->linesize[0]);
for (int x = 0; x < in->width; x++) {
for (int c = 0; c < 3; c++)
outp[s->co[c]] = s->lut[c][inp[s->co[c]]];
if (s->num_components == 4)

outp[s->co[3]] = inp[s->co[3]];
inp += s->step;
outp += s->step;
}
}
}

static void find_min_max_planar_16(NormalizeContext *s, AVFrame *in, NormalizeLocal min[3], NormalizeLocal max[3])
{
min[0].in = max[0].in = AV_RN16(in->data[2]);
min[1].in = max[1].in = AV_RN16(in->data[0]);
min[2].in = max[2].in = AV_RN16(in->data[1]);
for (int y = 0; y < in->height; y++) {
uint16_t *inrp = (uint16_t *)(in->data[2] + y * in->linesize[2]);
uint16_t *ingp = (uint16_t *)(in->data[0] + y * in->linesize[0]);
uint16_t *inbp = (uint16_t *)(in->data[1] + y * in->linesize[1]);
for (int x = 0; x < in->width; x++) {
min[0].in = FFMIN(min[0].in, inrp[x]);
max[0].in = FFMAX(max[0].in, inrp[x]);
min[1].in = FFMIN(min[1].in, ingp[x]);
max[1].in = FFMAX(max[1].in, ingp[x]);
min[2].in = FFMIN(min[2].in, inbp[x]);
max[2].in = FFMAX(max[2].in, inbp[x]);
}
}
}

static void process_planar_16(NormalizeContext *s, AVFrame *in, AVFrame *out)
{
for (int y = 0; y < in->height; y++) {
uint16_t *inrp = (uint16_t *)(in->data[2] + y * in->linesize[2]);
uint16_t *ingp = (uint16_t *)(in->data[0] + y * in->linesize[0]);
uint16_t *inbp = (uint16_t *)(in->data[1] + y * in->linesize[1]);
uint16_t *inap = (uint16_t *)(in->data[3] + y * in->linesize[3]);
uint16_t *outrp = (uint16_t *)(out->data[2] + y * out->linesize[2]);
uint16_t *outgp = (uint16_t *)(out->data[0] + y * out->linesize[0]);
uint16_t *outbp = (uint16_t *)(out->data[1] + y * out->linesize[1]);
uint16_t *outap = (uint16_t *)(out->data[3] + y * out->linesize[3]);
for (int x = 0; x < in->width; x++) {
outrp[x] = s->lut[0][inrp[x]];
outgp[x] = s->lut[1][ingp[x]];
outbp[x] = s->lut[2][inbp[x]];
if (s->num_components == 4)
outap[x] = inap[x];
}
}
}




static void normalize(NormalizeContext *s, AVFrame *in, AVFrame *out)
{

NormalizeLocal min[3], max[3]; 

float rgb_min_smoothed; 
float rgb_max_smoothed; 
int c;



s->find_min_max(s, in, min, max);



{
int history_idx = s->frame_num % s->history_len;


int num_history_vals = s->frame_num + 1;
if (s->frame_num >= s->history_len) {

for (c = 0; c < 3; c++) {
s->min[c].history_sum -= s->min[c].history[history_idx];
s->max[c].history_sum -= s->max[c].history[history_idx];
}
num_history_vals = s->history_len;
}


for (c = 0; c < 3; c++) {
s->min[c].history_sum += (s->min[c].history[history_idx] = min[c].in);
min[c].smoothed = s->min[c].history_sum / (float)num_history_vals;
s->max[c].history_sum += (s->max[c].history[history_idx] = max[c].in);
max[c].smoothed = s->max[c].history_sum / (float)num_history_vals;
}
}




rgb_min_smoothed = FFMIN3(min[0].smoothed, min[1].smoothed, min[2].smoothed);
rgb_max_smoothed = FFMAX3(max[0].smoothed, max[1].smoothed, max[2].smoothed);



for (c = 0; c < 3; c++) {
int in_val;



min[c].smoothed = (min[c].smoothed * s->independence)
+ (rgb_min_smoothed * (1.0f - s->independence));
max[c].smoothed = (max[c].smoothed * s->independence)
+ (rgb_max_smoothed * (1.0f - s->independence));




min[c].out = (s->sblackpt[c] * s->strength)
+ (min[c].in * (1.0f - s->strength));
max[c].out = (s->swhitept[c] * s->strength)
+ (max[c].in * (1.0f - s->strength));






if (min[c].smoothed == max[c].smoothed) {

for (in_val = min[c].in; in_val <= max[c].in; in_val++)
s->lut[c][in_val] = min[c].out;
} else {




float scale = (max[c].out - min[c].out) / (max[c].smoothed - min[c].smoothed);
for (in_val = min[c].in; in_val <= max[c].in; in_val++) {
int out_val = (in_val - min[c].smoothed) * scale + min[c].out + 0.5f;
out_val = av_clip_uintp2_c(out_val, s->depth);
s->lut[c][in_val] = out_val;
}
}
}


s->process(s, in, out);

s->frame_num++;
}








static int query_formats(AVFilterContext *ctx)
{
static const enum AVPixelFormat pixel_fmts[] = {
AV_PIX_FMT_RGB24,
AV_PIX_FMT_BGR24,
AV_PIX_FMT_ARGB,
AV_PIX_FMT_RGBA,
AV_PIX_FMT_ABGR,
AV_PIX_FMT_BGRA,
AV_PIX_FMT_0RGB,
AV_PIX_FMT_RGB0,
AV_PIX_FMT_0BGR,
AV_PIX_FMT_BGR0,
AV_PIX_FMT_RGB48, AV_PIX_FMT_BGR48,
AV_PIX_FMT_RGBA64, AV_PIX_FMT_BGRA64,
AV_PIX_FMT_GBRP, AV_PIX_FMT_GBRP9, AV_PIX_FMT_GBRP10,
AV_PIX_FMT_GBRP12, AV_PIX_FMT_GBRP14, AV_PIX_FMT_GBRP16,
AV_PIX_FMT_GBRAP, AV_PIX_FMT_GBRAP10, AV_PIX_FMT_GBRAP12, AV_PIX_FMT_GBRAP16,
AV_PIX_FMT_NONE
};



AVFilterFormats *formats = ff_make_format_list(pixel_fmts);
if (!formats)
return AVERROR(ENOMEM);
return ff_set_common_formats(ctx, formats);
}




static int config_input(AVFilterLink *inlink)
{
NormalizeContext *s = inlink->dst->priv;

const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(inlink->format);
int c, planar, scale;

ff_fill_rgba_map(s->co, inlink->format);
s->depth = desc->comp[0].depth;
scale = 1 << (s->depth - 8);
s->num_components = desc->nb_components;
s->step = av_get_padded_bits_per_pixel(desc) >> (3 + (s->depth > 8));





s->history_len = s->smoothing + 1;



s->history_mem = av_malloc(s->history_len * 6 * sizeof(*s->history_mem));
if (s->history_mem == NULL)
return AVERROR(ENOMEM);

for (c = 0; c < 3; c++) {
s->min[c].history = s->history_mem + (c*2) * s->history_len;
s->max[c].history = s->history_mem + (c*2+1) * s->history_len;
s->sblackpt[c] = scale * s->blackpt[c] + (s->blackpt[c] >> (s->depth - 8));
s->swhitept[c] = scale * s->whitept[c] + (s->whitept[c] >> (s->depth - 8));
}

planar = desc->flags & AV_PIX_FMT_FLAG_PLANAR;

if (s->depth <= 8) {
s->find_min_max = planar ? find_min_max_planar : find_min_max;
s->process = planar? process_planar : process;
} else {
s->find_min_max = planar ? find_min_max_planar_16 : find_min_max_16;
s->process = planar? process_planar_16 : process_16;
}

return 0;
}


static av_cold void uninit(AVFilterContext *ctx)
{
NormalizeContext *s = ctx->priv;

av_freep(&s->history_mem);
}




static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
AVFilterContext *ctx = inlink->dst;
AVFilterLink *outlink = ctx->outputs[0];
NormalizeContext *s = ctx->priv;
AVFrame *out;


int direct = av_frame_is_writable(in) && !ctx->is_disabled;

if (direct) {
out = in;
} else {
out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
if (!out) {
av_frame_free(&in);
return AVERROR(ENOMEM);
}
av_frame_copy_props(out, in);
}



normalize(s, in, out);

if (ctx->is_disabled) {
av_frame_free(&out);
return ff_filter_frame(outlink, in);
}

if (!direct)
av_frame_free(&in);

return ff_filter_frame(outlink, out);
}

static const AVFilterPad inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
.filter_frame = filter_frame,
.config_props = config_input,
},
{ NULL }
};

static const AVFilterPad outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_VIDEO,
},
{ NULL }
};

AVFilter ff_vf_normalize = {
.name = "normalize",
.description = NULL_IF_CONFIG_SMALL("Normalize RGB video."),
.priv_size = sizeof(NormalizeContext),
.priv_class = &normalize_class,
.uninit = uninit,
.query_formats = query_formats,
.inputs = inputs,
.outputs = outputs,
.flags = AVFILTER_FLAG_SUPPORT_TIMELINE_INTERNAL,
.process_command = ff_filter_process_command,
};
