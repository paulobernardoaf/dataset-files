#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "audio.h"
#include "filters.h"
#include "formats.h"
#include "hermite.h"
typedef struct AudioGateContext {
const AVClass *class;
double level_in;
double level_sc;
double attack;
double release;
double threshold;
double ratio;
double knee;
double makeup;
double range;
int link;
int detection;
int mode;
double thres;
double knee_start;
double knee_stop;
double lin_knee_start;
double lin_knee_stop;
double lin_slope;
double attack_coeff;
double release_coeff;
AVAudioFifo *fifo[2];
int64_t pts;
} AudioGateContext;
#define OFFSET(x) offsetof(AudioGateContext, x)
#define A AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_FILTERING_PARAM
static const AVOption options[] = {
{ "level_in", "set input level", OFFSET(level_in), AV_OPT_TYPE_DOUBLE, {.dbl=1}, 0.015625, 64, A },
{ "mode", "set mode", OFFSET(mode), AV_OPT_TYPE_INT, {.i64=0}, 0, 1, A, "mode" },
{ "downward",0, 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, A, "mode" },
{ "upward", 0, 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, A, "mode" },
{ "range", "set max gain reduction", OFFSET(range), AV_OPT_TYPE_DOUBLE, {.dbl=0.06125}, 0, 1, A },
{ "threshold", "set threshold", OFFSET(threshold), AV_OPT_TYPE_DOUBLE, {.dbl=0.125}, 0, 1, A },
{ "ratio", "set ratio", OFFSET(ratio), AV_OPT_TYPE_DOUBLE, {.dbl=2}, 1, 9000, A },
{ "attack", "set attack", OFFSET(attack), AV_OPT_TYPE_DOUBLE, {.dbl=20}, 0.01, 9000, A },
{ "release", "set release", OFFSET(release), AV_OPT_TYPE_DOUBLE, {.dbl=250}, 0.01, 9000, A },
{ "makeup", "set makeup gain", OFFSET(makeup), AV_OPT_TYPE_DOUBLE, {.dbl=1}, 1, 64, A },
{ "knee", "set knee", OFFSET(knee), AV_OPT_TYPE_DOUBLE, {.dbl=2.828427125}, 1, 8, A },
{ "detection", "set detection", OFFSET(detection), AV_OPT_TYPE_INT, {.i64=1}, 0, 1, A, "detection" },
{ "peak", 0, 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, A, "detection" },
{ "rms", 0, 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, A, "detection" },
{ "link", "set link", OFFSET(link), AV_OPT_TYPE_INT, {.i64=0}, 0, 1, A, "link" },
{ "average", 0, 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, A, "link" },
{ "maximum", 0, 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, A, "link" },
{ "level_sc", "set sidechain gain", OFFSET(level_sc), AV_OPT_TYPE_DOUBLE, {.dbl=1}, 0.015625, 64, A },
{ NULL }
};
static int agate_config_input(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
AudioGateContext *s = ctx->priv;
double lin_threshold = s->threshold;
double lin_knee_sqrt = sqrt(s->knee);
if (s->detection)
lin_threshold *= lin_threshold;
s->attack_coeff = FFMIN(1., 1. / (s->attack * inlink->sample_rate / 4000.));
s->release_coeff = FFMIN(1., 1. / (s->release * inlink->sample_rate / 4000.));
s->lin_knee_stop = lin_threshold * lin_knee_sqrt;
s->lin_knee_start = lin_threshold / lin_knee_sqrt;
s->thres = log(lin_threshold);
s->knee_start = log(s->lin_knee_start);
s->knee_stop = log(s->lin_knee_stop);
return 0;
}
#define FAKE_INFINITY (65536.0 * 65536.0)
#define IS_FAKE_INFINITY(value) (fabs(value-FAKE_INFINITY) < 1.0)
static double output_gain(double lin_slope, double ratio, double thres,
double knee, double knee_start, double knee_stop,
double range, int mode)
{
double slope = log(lin_slope);
double tratio = ratio;
double gain = 0.;
double delta = 0.;
if (IS_FAKE_INFINITY(ratio))
tratio = 1000.;
gain = (slope - thres) * tratio + thres;
delta = tratio;
if (mode) {
if (knee > 1. && slope < knee_stop)
gain = hermite_interpolation(slope, knee_stop, knee_start, ((knee_stop - thres) * tratio + thres), knee_start, delta, 1.);
} else {
if (knee > 1. && slope > knee_start)
gain = hermite_interpolation(slope, knee_start, knee_stop, ((knee_start - thres) * tratio + thres), knee_stop, delta, 1.);
}
return FFMAX(range, exp(gain - slope));
}
static void gate(AudioGateContext *s,
const double *src, double *dst, const double *scsrc,
int nb_samples, double level_in, double level_sc,
AVFilterLink *inlink, AVFilterLink *sclink)
{
const double makeup = s->makeup;
const double attack_coeff = s->attack_coeff;
const double release_coeff = s->release_coeff;
int n, c;
for (n = 0; n < nb_samples; n++, src += inlink->channels, dst += inlink->channels, scsrc += sclink->channels) {
double abs_sample = fabs(scsrc[0] * level_sc), gain = 1.0;
int detected;
if (s->link == 1) {
for (c = 1; c < sclink->channels; c++)
abs_sample = FFMAX(fabs(scsrc[c] * level_sc), abs_sample);
} else {
for (c = 1; c < sclink->channels; c++)
abs_sample += fabs(scsrc[c] * level_sc);
abs_sample /= sclink->channels;
}
if (s->detection)
abs_sample *= abs_sample;
s->lin_slope += (abs_sample - s->lin_slope) * (abs_sample > s->lin_slope ? attack_coeff : release_coeff);
if (s->mode)
detected = s->lin_slope > s->lin_knee_start;
else
detected = s->lin_slope < s->lin_knee_stop;
if (s->lin_slope > 0.0 && detected)
gain = output_gain(s->lin_slope, s->ratio, s->thres,
s->knee, s->knee_start, s->knee_stop,
s->range, s->mode);
for (c = 0; c < inlink->channels; c++)
dst[c] = src[c] * level_in * gain * makeup;
}
}
#if CONFIG_AGATE_FILTER
#define agate_options options
AVFILTER_DEFINE_CLASS(agate);
static int query_formats(AVFilterContext *ctx)
{
AVFilterFormats *formats = NULL;
AVFilterChannelLayouts *layouts;
int ret;
if ((ret = ff_add_format(&formats, AV_SAMPLE_FMT_DBL)) < 0)
return ret;
ret = ff_set_common_formats(ctx, formats);
if (ret < 0)
return ret;
layouts = ff_all_channel_counts();
if (!layouts)
return AVERROR(ENOMEM);
ret = ff_set_common_channel_layouts(ctx, layouts);
if (ret < 0)
return ret;
formats = ff_all_samplerates();
if (!formats)
return AVERROR(ENOMEM);
return ff_set_common_samplerates(ctx, formats);
}
static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
const double *src = (const double *)in->data[0];
AVFilterContext *ctx = inlink->dst;
AVFilterLink *outlink = ctx->outputs[0];
AudioGateContext *s = ctx->priv;
AVFrame *out;
double *dst;
if (av_frame_is_writable(in)) {
out = in;
} else {
out = ff_get_audio_buffer(outlink, in->nb_samples);
if (!out) {
av_frame_free(&in);
return AVERROR(ENOMEM);
}
av_frame_copy_props(out, in);
}
dst = (double *)out->data[0];
gate(s, src, dst, src, in->nb_samples,
s->level_in, s->level_in, inlink, inlink);
if (out != in)
av_frame_free(&in);
return ff_filter_frame(outlink, out);
}
static const AVFilterPad inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
.config_props = agate_config_input,
},
{ NULL }
};
static const AVFilterPad outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
AVFilter ff_af_agate = {
.name = "agate",
.description = NULL_IF_CONFIG_SMALL("Audio gate."),
.query_formats = query_formats,
.priv_size = sizeof(AudioGateContext),
.priv_class = &agate_class,
.inputs = inputs,
.outputs = outputs,
};
#endif 
#if CONFIG_SIDECHAINGATE_FILTER
#define sidechaingate_options options
AVFILTER_DEFINE_CLASS(sidechaingate);
static int activate(AVFilterContext *ctx)
{
AudioGateContext *s = ctx->priv;
AVFrame *out = NULL, *in[2] = { NULL };
int ret, i, nb_samples;
double *dst;
FF_FILTER_FORWARD_STATUS_BACK_ALL(ctx->outputs[0], ctx);
if ((ret = ff_inlink_consume_frame(ctx->inputs[0], &in[0])) > 0) {
av_audio_fifo_write(s->fifo[0], (void **)in[0]->extended_data,
in[0]->nb_samples);
av_frame_free(&in[0]);
}
if (ret < 0)
return ret;
if ((ret = ff_inlink_consume_frame(ctx->inputs[1], &in[1])) > 0) {
av_audio_fifo_write(s->fifo[1], (void **)in[1]->extended_data,
in[1]->nb_samples);
av_frame_free(&in[1]);
}
if (ret < 0)
return ret;
nb_samples = FFMIN(av_audio_fifo_size(s->fifo[0]), av_audio_fifo_size(s->fifo[1]));
if (nb_samples) {
out = ff_get_audio_buffer(ctx->outputs[0], nb_samples);
if (!out)
return AVERROR(ENOMEM);
for (i = 0; i < 2; i++) {
in[i] = ff_get_audio_buffer(ctx->inputs[i], nb_samples);
if (!in[i]) {
av_frame_free(&in[0]);
av_frame_free(&in[1]);
av_frame_free(&out);
return AVERROR(ENOMEM);
}
av_audio_fifo_read(s->fifo[i], (void **)in[i]->data, nb_samples);
}
dst = (double *)out->data[0];
out->pts = s->pts;
s->pts += av_rescale_q(nb_samples, (AVRational){1, ctx->outputs[0]->sample_rate}, ctx->outputs[0]->time_base);
gate(s, (double *)in[0]->data[0], dst,
(double *)in[1]->data[0], nb_samples,
s->level_in, s->level_sc,
ctx->inputs[0], ctx->inputs[1]);
av_frame_free(&in[0]);
av_frame_free(&in[1]);
ret = ff_filter_frame(ctx->outputs[0], out);
if (ret < 0)
return ret;
}
FF_FILTER_FORWARD_STATUS(ctx->inputs[0], ctx->outputs[0]);
FF_FILTER_FORWARD_STATUS(ctx->inputs[1], ctx->outputs[0]);
if (ff_outlink_frame_wanted(ctx->outputs[0])) {
if (!av_audio_fifo_size(s->fifo[0]))
ff_inlink_request_frame(ctx->inputs[0]);
if (!av_audio_fifo_size(s->fifo[1]))
ff_inlink_request_frame(ctx->inputs[1]);
}
return 0;
}
static int scquery_formats(AVFilterContext *ctx)
{
AVFilterFormats *formats;
AVFilterChannelLayouts *layouts = NULL;
static const enum AVSampleFormat sample_fmts[] = {
AV_SAMPLE_FMT_DBL,
AV_SAMPLE_FMT_NONE
};
int ret, i;
if (!ctx->inputs[0]->in_channel_layouts ||
!ctx->inputs[0]->in_channel_layouts->nb_channel_layouts) {
av_log(ctx, AV_LOG_WARNING,
"No channel layout for input 1\n");
return AVERROR(EAGAIN);
}
if ((ret = ff_add_channel_layout(&layouts, ctx->inputs[0]->in_channel_layouts->channel_layouts[0])) < 0 ||
(ret = ff_channel_layouts_ref(layouts, &ctx->outputs[0]->in_channel_layouts)) < 0)
return ret;
for (i = 0; i < 2; i++) {
layouts = ff_all_channel_counts();
if ((ret = ff_channel_layouts_ref(layouts, &ctx->inputs[i]->out_channel_layouts)) < 0)
return ret;
}
formats = ff_make_format_list(sample_fmts);
if ((ret = ff_set_common_formats(ctx, formats)) < 0)
return ret;
formats = ff_all_samplerates();
return ff_set_common_samplerates(ctx, formats);
}
static int scconfig_output(AVFilterLink *outlink)
{
AVFilterContext *ctx = outlink->src;
AudioGateContext *s = ctx->priv;
if (ctx->inputs[0]->sample_rate != ctx->inputs[1]->sample_rate) {
av_log(ctx, AV_LOG_ERROR,
"Inputs must have the same sample rate "
"%d for in0 vs %d for in1\n",
ctx->inputs[0]->sample_rate, ctx->inputs[1]->sample_rate);
return AVERROR(EINVAL);
}
outlink->sample_rate = ctx->inputs[0]->sample_rate;
outlink->time_base = ctx->inputs[0]->time_base;
outlink->channel_layout = ctx->inputs[0]->channel_layout;
outlink->channels = ctx->inputs[0]->channels;
s->fifo[0] = av_audio_fifo_alloc(ctx->inputs[0]->format, ctx->inputs[0]->channels, 1024);
s->fifo[1] = av_audio_fifo_alloc(ctx->inputs[1]->format, ctx->inputs[1]->channels, 1024);
if (!s->fifo[0] || !s->fifo[1])
return AVERROR(ENOMEM);
agate_config_input(ctx->inputs[0]);
return 0;
}
static av_cold void uninit(AVFilterContext *ctx)
{
AudioGateContext *s = ctx->priv;
av_audio_fifo_free(s->fifo[0]);
av_audio_fifo_free(s->fifo[1]);
}
static const AVFilterPad sidechaingate_inputs[] = {
{
.name = "main",
.type = AVMEDIA_TYPE_AUDIO,
},{
.name = "sidechain",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
static const AVFilterPad sidechaingate_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.config_props = scconfig_output,
},
{ NULL }
};
AVFilter ff_af_sidechaingate = {
.name = "sidechaingate",
.description = NULL_IF_CONFIG_SMALL("Audio sidechain gate."),
.priv_size = sizeof(AudioGateContext),
.priv_class = &sidechaingate_class,
.query_formats = scquery_formats,
.activate = activate,
.uninit = uninit,
.inputs = sidechaingate_inputs,
.outputs = sidechaingate_outputs,
};
#endif 
