#include <float.h>
#include "libavcodec/avfft.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
#include "avfilter.h"
#include "audio.h"
#include "internal.h"
typedef struct AudioFragment {
int64_t position[2];
uint8_t *data;
int nsamples;
FFTSample *xdat;
} AudioFragment;
typedef enum {
YAE_LOAD_FRAGMENT,
YAE_ADJUST_POSITION,
YAE_RELOAD_FRAGMENT,
YAE_OUTPUT_OVERLAP_ADD,
YAE_FLUSH_OUTPUT,
} FilterState;
typedef struct ATempoContext {
const AVClass *class;
uint8_t *buffer;
int ring;
int size;
int head;
int tail;
int64_t position[2];
int64_t start_pts;
enum AVSampleFormat format;
int channels;
int stride;
int window;
float *hann;
double tempo;
int64_t origin[2];
AudioFragment frag[2];
uint64_t nfrag;
FilterState state;
RDFTContext *real_to_complex;
RDFTContext *complex_to_real;
FFTSample *correlation;
AVFrame *dst_buffer;
uint8_t *dst;
uint8_t *dst_end;
uint64_t nsamples_in;
uint64_t nsamples_out;
} ATempoContext;
#define YAE_ATEMPO_MIN 0.5
#define YAE_ATEMPO_MAX 100.0
#define OFFSET(x) offsetof(ATempoContext, x)
static const AVOption atempo_options[] = {
{ "tempo", "set tempo scale factor",
OFFSET(tempo), AV_OPT_TYPE_DOUBLE, { .dbl = 1.0 },
YAE_ATEMPO_MIN,
YAE_ATEMPO_MAX,
AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_FILTERING_PARAM | AV_OPT_FLAG_RUNTIME_PARAM },
{ NULL }
};
AVFILTER_DEFINE_CLASS(atempo);
inline static AudioFragment *yae_curr_frag(ATempoContext *atempo)
{
return &atempo->frag[atempo->nfrag % 2];
}
inline static AudioFragment *yae_prev_frag(ATempoContext *atempo)
{
return &atempo->frag[(atempo->nfrag + 1) % 2];
}
static void yae_clear(ATempoContext *atempo)
{
atempo->size = 0;
atempo->head = 0;
atempo->tail = 0;
atempo->nfrag = 0;
atempo->state = YAE_LOAD_FRAGMENT;
atempo->start_pts = AV_NOPTS_VALUE;
atempo->position[0] = 0;
atempo->position[1] = 0;
atempo->origin[0] = 0;
atempo->origin[1] = 0;
atempo->frag[0].position[0] = 0;
atempo->frag[0].position[1] = 0;
atempo->frag[0].nsamples = 0;
atempo->frag[1].position[0] = 0;
atempo->frag[1].position[1] = 0;
atempo->frag[1].nsamples = 0;
atempo->frag[0].position[0] = -(int64_t)(atempo->window / 2);
atempo->frag[0].position[1] = -(int64_t)(atempo->window / 2);
av_frame_free(&atempo->dst_buffer);
atempo->dst = NULL;
atempo->dst_end = NULL;
atempo->nsamples_in = 0;
atempo->nsamples_out = 0;
}
static void yae_release_buffers(ATempoContext *atempo)
{
yae_clear(atempo);
av_freep(&atempo->frag[0].data);
av_freep(&atempo->frag[1].data);
av_freep(&atempo->frag[0].xdat);
av_freep(&atempo->frag[1].xdat);
av_freep(&atempo->buffer);
av_freep(&atempo->hann);
av_freep(&atempo->correlation);
av_rdft_end(atempo->real_to_complex);
atempo->real_to_complex = NULL;
av_rdft_end(atempo->complex_to_real);
atempo->complex_to_real = NULL;
}
#define RE_MALLOC_OR_FAIL(field, field_size) do { av_freep(&field); field = av_malloc(field_size); if (!field) { yae_release_buffers(atempo); return AVERROR(ENOMEM); } } while (0)
static int yae_reset(ATempoContext *atempo,
enum AVSampleFormat format,
int sample_rate,
int channels)
{
const int sample_size = av_get_bytes_per_sample(format);
uint32_t nlevels = 0;
uint32_t pot;
int i;
atempo->format = format;
atempo->channels = channels;
atempo->stride = sample_size * channels;
atempo->window = sample_rate / 24;
nlevels = av_log2(atempo->window);
pot = 1 << nlevels;
av_assert0(pot <= atempo->window);
if (pot < atempo->window) {
atempo->window = pot * 2;
nlevels++;
}
RE_MALLOC_OR_FAIL(atempo->frag[0].data, atempo->window * atempo->stride);
RE_MALLOC_OR_FAIL(atempo->frag[1].data, atempo->window * atempo->stride);
RE_MALLOC_OR_FAIL(atempo->frag[0].xdat, atempo->window * sizeof(FFTComplex));
RE_MALLOC_OR_FAIL(atempo->frag[1].xdat, atempo->window * sizeof(FFTComplex));
av_rdft_end(atempo->real_to_complex);
atempo->real_to_complex = NULL;
av_rdft_end(atempo->complex_to_real);
atempo->complex_to_real = NULL;
atempo->real_to_complex = av_rdft_init(nlevels + 1, DFT_R2C);
if (!atempo->real_to_complex) {
yae_release_buffers(atempo);
return AVERROR(ENOMEM);
}
atempo->complex_to_real = av_rdft_init(nlevels + 1, IDFT_C2R);
if (!atempo->complex_to_real) {
yae_release_buffers(atempo);
return AVERROR(ENOMEM);
}
RE_MALLOC_OR_FAIL(atempo->correlation, atempo->window * sizeof(FFTComplex));
atempo->ring = atempo->window * 3;
RE_MALLOC_OR_FAIL(atempo->buffer, atempo->ring * atempo->stride);
RE_MALLOC_OR_FAIL(atempo->hann, atempo->window * sizeof(float));
for (i = 0; i < atempo->window; i++) {
double t = (double)i / (double)(atempo->window - 1);
double h = 0.5 * (1.0 - cos(2.0 * M_PI * t));
atempo->hann[i] = (float)h;
}
yae_clear(atempo);
return 0;
}
static int yae_update(AVFilterContext *ctx)
{
const AudioFragment *prev;
ATempoContext *atempo = ctx->priv;
prev = yae_prev_frag(atempo);
atempo->origin[0] = prev->position[0] + atempo->window / 2;
atempo->origin[1] = prev->position[1] + atempo->window / 2;
return 0;
}
#define yae_init_xdat(scalar_type, scalar_max) do { const uint8_t *src_end = src + frag->nsamples * atempo->channels * sizeof(scalar_type); FFTSample *xdat = frag->xdat; scalar_type tmp; if (atempo->channels == 1) { for (; src < src_end; xdat++) { tmp = *(const scalar_type *)src; src += sizeof(scalar_type); *xdat = (FFTSample)tmp; } } else { FFTSample s, max, ti, si; int i; for (; src < src_end; xdat++) { tmp = *(const scalar_type *)src; src += sizeof(scalar_type); max = (FFTSample)tmp; s = FFMIN((FFTSample)scalar_max, (FFTSample)fabsf(max)); for (i = 1; i < atempo->channels; i++) { tmp = *(const scalar_type *)src; src += sizeof(scalar_type); ti = (FFTSample)tmp; si = FFMIN((FFTSample)scalar_max, (FFTSample)fabsf(ti)); if (s < si) { s = si; max = ti; } } *xdat = max; } } } while (0)
static void yae_downmix(ATempoContext *atempo, AudioFragment *frag)
{
const uint8_t *src = frag->data;
memset(frag->xdat, 0, sizeof(FFTComplex) * atempo->window);
if (atempo->format == AV_SAMPLE_FMT_U8) {
yae_init_xdat(uint8_t, 127);
} else if (atempo->format == AV_SAMPLE_FMT_S16) {
yae_init_xdat(int16_t, 32767);
} else if (atempo->format == AV_SAMPLE_FMT_S32) {
yae_init_xdat(int, 2147483647);
} else if (atempo->format == AV_SAMPLE_FMT_FLT) {
yae_init_xdat(float, 1);
} else if (atempo->format == AV_SAMPLE_FMT_DBL) {
yae_init_xdat(double, 1);
}
}
static int yae_load_data(ATempoContext *atempo,
const uint8_t **src_ref,
const uint8_t *src_end,
int64_t stop_here)
{
const uint8_t *src = *src_ref;
const int read_size = stop_here - atempo->position[0];
if (stop_here <= atempo->position[0]) {
return 0;
}
av_assert0(read_size <= atempo->ring || atempo->tempo > 2.0);
while (atempo->position[0] < stop_here && src < src_end) {
int src_samples = (src_end - src) / atempo->stride;
int nsamples = FFMIN(read_size, src_samples);
int na;
int nb;
nsamples = FFMIN(nsamples, atempo->ring);
na = FFMIN(nsamples, atempo->ring - atempo->tail);
nb = FFMIN(nsamples - na, atempo->ring);
if (na) {
uint8_t *a = atempo->buffer + atempo->tail * atempo->stride;
memcpy(a, src, na * atempo->stride);
src += na * atempo->stride;
atempo->position[0] += na;
atempo->size = FFMIN(atempo->size + na, atempo->ring);
atempo->tail = (atempo->tail + na) % atempo->ring;
atempo->head =
atempo->size < atempo->ring ?
atempo->tail - atempo->size :
atempo->tail;
}
if (nb) {
uint8_t *b = atempo->buffer;
memcpy(b, src, nb * atempo->stride);
src += nb * atempo->stride;
atempo->position[0] += nb;
atempo->size = FFMIN(atempo->size + nb, atempo->ring);
atempo->tail = (atempo->tail + nb) % atempo->ring;
atempo->head =
atempo->size < atempo->ring ?
atempo->tail - atempo->size :
atempo->tail;
}
}
*src_ref = src;
av_assert0(atempo->position[0] <= stop_here);
return atempo->position[0] == stop_here ? 0 : AVERROR(EAGAIN);
}
static int yae_load_frag(ATempoContext *atempo,
const uint8_t **src_ref,
const uint8_t *src_end)
{
AudioFragment *frag = yae_curr_frag(atempo);
uint8_t *dst;
int64_t missing, start, zeros;
uint32_t nsamples;
const uint8_t *a, *b;
int i0, i1, n0, n1, na, nb;
int64_t stop_here = frag->position[0] + atempo->window;
if (src_ref && yae_load_data(atempo, src_ref, src_end, stop_here) != 0) {
return AVERROR(EAGAIN);
}
missing =
stop_here > atempo->position[0] ?
stop_here - atempo->position[0] : 0;
nsamples =
missing < (int64_t)atempo->window ?
(uint32_t)(atempo->window - missing) : 0;
frag->nsamples = nsamples;
dst = frag->data;
start = atempo->position[0] - atempo->size;
zeros = 0;
if (frag->position[0] < start) {
zeros = FFMIN(start - frag->position[0], (int64_t)nsamples);
av_assert0(zeros != nsamples);
memset(dst, 0, zeros * atempo->stride);
dst += zeros * atempo->stride;
}
if (zeros == nsamples) {
return 0;
}
na = (atempo->head < atempo->tail ?
atempo->tail - atempo->head :
atempo->ring - atempo->head);
nb = atempo->head < atempo->tail ? 0 : atempo->tail;
av_assert0(nsamples <= zeros + na + nb);
a = atempo->buffer + atempo->head * atempo->stride;
b = atempo->buffer;
i0 = frag->position[0] + zeros - start;
i1 = i0 < na ? 0 : i0 - na;
n0 = i0 < na ? FFMIN(na - i0, (int)(nsamples - zeros)) : 0;
n1 = nsamples - zeros - n0;
if (n0) {
memcpy(dst, a + i0 * atempo->stride, n0 * atempo->stride);
dst += n0 * atempo->stride;
}
if (n1) {
memcpy(dst, b + i1 * atempo->stride, n1 * atempo->stride);
}
return 0;
}
static void yae_advance_to_next_frag(ATempoContext *atempo)
{
const double fragment_step = atempo->tempo * (double)(atempo->window / 2);
const AudioFragment *prev;
AudioFragment *frag;
atempo->nfrag++;
prev = yae_prev_frag(atempo);
frag = yae_curr_frag(atempo);
frag->position[0] = prev->position[0] + (int64_t)fragment_step;
frag->position[1] = prev->position[1] + atempo->window / 2;
frag->nsamples = 0;
}
static void yae_xcorr_via_rdft(FFTSample *xcorr,
RDFTContext *complex_to_real,
const FFTComplex *xa,
const FFTComplex *xb,
const int window)
{
FFTComplex *xc = (FFTComplex *)xcorr;
int i;
xc->re = xa->re * xb->re;
xc->im = xa->im * xb->im;
xa++;
xb++;
xc++;
for (i = 1; i < window; i++, xa++, xb++, xc++) {
xc->re = (xa->re * xb->re + xa->im * xb->im);
xc->im = (xa->im * xb->re - xa->re * xb->im);
}
av_rdft_calc(complex_to_real, xcorr);
}
static int yae_align(AudioFragment *frag,
const AudioFragment *prev,
const int window,
const int delta_max,
const int drift,
FFTSample *correlation,
RDFTContext *complex_to_real)
{
int best_offset = -drift;
FFTSample best_metric = -FLT_MAX;
FFTSample *xcorr;
int i0;
int i1;
int i;
yae_xcorr_via_rdft(correlation,
complex_to_real,
(const FFTComplex *)prev->xdat,
(const FFTComplex *)frag->xdat,
window);
i0 = FFMAX(window / 2 - delta_max - drift, 0);
i0 = FFMIN(i0, window);
i1 = FFMIN(window / 2 + delta_max - drift, window - window / 16);
i1 = FFMAX(i1, 0);
xcorr = correlation + i0;
for (i = i0; i < i1; i++, xcorr++) {
FFTSample metric = *xcorr;
FFTSample drifti = (FFTSample)(drift + i);
metric *= drifti * (FFTSample)(i - i0) * (FFTSample)(i1 - i);
if (metric > best_metric) {
best_metric = metric;
best_offset = i - window / 2;
}
}
return best_offset;
}
static int yae_adjust_position(ATempoContext *atempo)
{
const AudioFragment *prev = yae_prev_frag(atempo);
AudioFragment *frag = yae_curr_frag(atempo);
const double prev_output_position =
(double)(prev->position[1] - atempo->origin[1] + atempo->window / 2) *
atempo->tempo;
const double ideal_output_position =
(double)(prev->position[0] - atempo->origin[0] + atempo->window / 2);
const int drift = (int)(prev_output_position - ideal_output_position);
const int delta_max = atempo->window / 2;
const int correction = yae_align(frag,
prev,
atempo->window,
delta_max,
drift,
atempo->correlation,
atempo->complex_to_real);
if (correction) {
frag->position[0] -= correction;
frag->nsamples = 0;
}
return correction;
}
#define yae_blend(scalar_type) do { const scalar_type *aaa = (const scalar_type *)a; const scalar_type *bbb = (const scalar_type *)b; scalar_type *out = (scalar_type *)dst; scalar_type *out_end = (scalar_type *)dst_end; int64_t i; for (i = 0; i < overlap && out < out_end; i++, atempo->position[1]++, wa++, wb++) { float w0 = *wa; float w1 = *wb; int j; for (j = 0; j < atempo->channels; j++, aaa++, bbb++, out++) { float t0 = (float)*aaa; float t1 = (float)*bbb; *out = frag->position[0] + i < 0 ? *aaa : (scalar_type)(t0 * w0 + t1 * w1); } } dst = (uint8_t *)out; } while (0)
static int yae_overlap_add(ATempoContext *atempo,
uint8_t **dst_ref,
uint8_t *dst_end)
{
const AudioFragment *prev = yae_prev_frag(atempo);
const AudioFragment *frag = yae_curr_frag(atempo);
const int64_t start_here = FFMAX(atempo->position[1],
frag->position[1]);
const int64_t stop_here = FFMIN(prev->position[1] + prev->nsamples,
frag->position[1] + frag->nsamples);
const int64_t overlap = stop_here - start_here;
const int64_t ia = start_here - prev->position[1];
const int64_t ib = start_here - frag->position[1];
const float *wa = atempo->hann + ia;
const float *wb = atempo->hann + ib;
const uint8_t *a = prev->data + ia * atempo->stride;
const uint8_t *b = frag->data + ib * atempo->stride;
uint8_t *dst = *dst_ref;
av_assert0(start_here <= stop_here &&
frag->position[1] <= start_here &&
overlap <= frag->nsamples);
if (atempo->format == AV_SAMPLE_FMT_U8) {
yae_blend(uint8_t);
} else if (atempo->format == AV_SAMPLE_FMT_S16) {
yae_blend(int16_t);
} else if (atempo->format == AV_SAMPLE_FMT_S32) {
yae_blend(int);
} else if (atempo->format == AV_SAMPLE_FMT_FLT) {
yae_blend(float);
} else if (atempo->format == AV_SAMPLE_FMT_DBL) {
yae_blend(double);
}
*dst_ref = dst;
return atempo->position[1] == stop_here ? 0 : AVERROR(EAGAIN);
}
static void
yae_apply(ATempoContext *atempo,
const uint8_t **src_ref,
const uint8_t *src_end,
uint8_t **dst_ref,
uint8_t *dst_end)
{
while (1) {
if (atempo->state == YAE_LOAD_FRAGMENT) {
if (yae_load_frag(atempo, src_ref, src_end) != 0) {
break;
}
yae_downmix(atempo, yae_curr_frag(atempo));
av_rdft_calc(atempo->real_to_complex, yae_curr_frag(atempo)->xdat);
if (!atempo->nfrag) {
yae_advance_to_next_frag(atempo);
continue;
}
atempo->state = YAE_ADJUST_POSITION;
}
if (atempo->state == YAE_ADJUST_POSITION) {
if (yae_adjust_position(atempo)) {
atempo->state = YAE_RELOAD_FRAGMENT;
} else {
atempo->state = YAE_OUTPUT_OVERLAP_ADD;
}
}
if (atempo->state == YAE_RELOAD_FRAGMENT) {
if (yae_load_frag(atempo, src_ref, src_end) != 0) {
break;
}
yae_downmix(atempo, yae_curr_frag(atempo));
av_rdft_calc(atempo->real_to_complex, yae_curr_frag(atempo)->xdat);
atempo->state = YAE_OUTPUT_OVERLAP_ADD;
}
if (atempo->state == YAE_OUTPUT_OVERLAP_ADD) {
if (yae_overlap_add(atempo, dst_ref, dst_end) != 0) {
break;
}
yae_advance_to_next_frag(atempo);
atempo->state = YAE_LOAD_FRAGMENT;
}
}
}
static int yae_flush(ATempoContext *atempo,
uint8_t **dst_ref,
uint8_t *dst_end)
{
AudioFragment *frag = yae_curr_frag(atempo);
int64_t overlap_end;
int64_t start_here;
int64_t stop_here;
int64_t offset;
const uint8_t *src;
uint8_t *dst;
int src_size;
int dst_size;
int nbytes;
atempo->state = YAE_FLUSH_OUTPUT;
if (!atempo->nfrag) {
return 0;
}
if (atempo->position[0] == frag->position[0] + frag->nsamples &&
atempo->position[1] == frag->position[1] + frag->nsamples) {
return 0;
}
if (frag->position[0] + frag->nsamples < atempo->position[0]) {
yae_load_frag(atempo, NULL, NULL);
if (atempo->nfrag) {
yae_downmix(atempo, frag);
av_rdft_calc(atempo->real_to_complex, frag->xdat);
if (yae_adjust_position(atempo)) {
yae_load_frag(atempo, NULL, NULL);
}
}
}
overlap_end = frag->position[1] + FFMIN(atempo->window / 2,
frag->nsamples);
while (atempo->position[1] < overlap_end) {
if (yae_overlap_add(atempo, dst_ref, dst_end) != 0) {
return AVERROR(EAGAIN);
}
}
if (frag->position[0] + frag->nsamples < atempo->position[0]) {
yae_advance_to_next_frag(atempo);
return AVERROR(EAGAIN);
}
start_here = FFMAX(atempo->position[1], overlap_end);
stop_here = frag->position[1] + frag->nsamples;
offset = start_here - frag->position[1];
av_assert0(start_here <= stop_here && frag->position[1] <= start_here);
src = frag->data + offset * atempo->stride;
dst = (uint8_t *)*dst_ref;
src_size = (int)(stop_here - start_here) * atempo->stride;
dst_size = dst_end - dst;
nbytes = FFMIN(src_size, dst_size);
memcpy(dst, src, nbytes);
dst += nbytes;
atempo->position[1] += (nbytes / atempo->stride);
*dst_ref = (uint8_t *)dst;
return atempo->position[1] == stop_here ? 0 : AVERROR(EAGAIN);
}
static av_cold int init(AVFilterContext *ctx)
{
ATempoContext *atempo = ctx->priv;
atempo->format = AV_SAMPLE_FMT_NONE;
atempo->state = YAE_LOAD_FRAGMENT;
return 0;
}
static av_cold void uninit(AVFilterContext *ctx)
{
ATempoContext *atempo = ctx->priv;
yae_release_buffers(atempo);
}
static int query_formats(AVFilterContext *ctx)
{
AVFilterChannelLayouts *layouts = NULL;
AVFilterFormats *formats = NULL;
static const enum AVSampleFormat sample_fmts[] = {
AV_SAMPLE_FMT_U8,
AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_S32,
AV_SAMPLE_FMT_FLT,
AV_SAMPLE_FMT_DBL,
AV_SAMPLE_FMT_NONE
};
int ret;
layouts = ff_all_channel_counts();
if (!layouts) {
return AVERROR(ENOMEM);
}
ret = ff_set_common_channel_layouts(ctx, layouts);
if (ret < 0)
return ret;
formats = ff_make_format_list(sample_fmts);
if (!formats) {
return AVERROR(ENOMEM);
}
ret = ff_set_common_formats(ctx, formats);
if (ret < 0)
return ret;
formats = ff_all_samplerates();
if (!formats) {
return AVERROR(ENOMEM);
}
return ff_set_common_samplerates(ctx, formats);
}
static int config_props(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
ATempoContext *atempo = ctx->priv;
enum AVSampleFormat format = inlink->format;
int sample_rate = (int)inlink->sample_rate;
return yae_reset(atempo, format, sample_rate, inlink->channels);
}
static int push_samples(ATempoContext *atempo,
AVFilterLink *outlink,
int n_out)
{
int ret;
atempo->dst_buffer->sample_rate = outlink->sample_rate;
atempo->dst_buffer->nb_samples = n_out;
atempo->dst_buffer->pts = atempo->start_pts +
av_rescale_q(atempo->nsamples_out,
(AVRational){ 1, outlink->sample_rate },
outlink->time_base);
ret = ff_filter_frame(outlink, atempo->dst_buffer);
atempo->dst_buffer = NULL;
atempo->dst = NULL;
atempo->dst_end = NULL;
if (ret < 0)
return ret;
atempo->nsamples_out += n_out;
return 0;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *src_buffer)
{
AVFilterContext *ctx = inlink->dst;
ATempoContext *atempo = ctx->priv;
AVFilterLink *outlink = ctx->outputs[0];
int ret = 0;
int n_in = src_buffer->nb_samples;
int n_out = (int)(0.5 + ((double)n_in) / atempo->tempo);
const uint8_t *src = src_buffer->data[0];
const uint8_t *src_end = src + n_in * atempo->stride;
if (atempo->start_pts == AV_NOPTS_VALUE)
atempo->start_pts = av_rescale_q(src_buffer->pts,
inlink->time_base,
outlink->time_base);
while (src < src_end) {
if (!atempo->dst_buffer) {
atempo->dst_buffer = ff_get_audio_buffer(outlink, n_out);
if (!atempo->dst_buffer) {
av_frame_free(&src_buffer);
return AVERROR(ENOMEM);
}
av_frame_copy_props(atempo->dst_buffer, src_buffer);
atempo->dst = atempo->dst_buffer->data[0];
atempo->dst_end = atempo->dst + n_out * atempo->stride;
}
yae_apply(atempo, &src, src_end, &atempo->dst, atempo->dst_end);
if (atempo->dst == atempo->dst_end) {
int n_samples = ((atempo->dst - atempo->dst_buffer->data[0]) /
atempo->stride);
ret = push_samples(atempo, outlink, n_samples);
if (ret < 0)
goto end;
}
}
atempo->nsamples_in += n_in;
end:
av_frame_free(&src_buffer);
return ret;
}
static int request_frame(AVFilterLink *outlink)
{
AVFilterContext *ctx = outlink->src;
ATempoContext *atempo = ctx->priv;
int ret;
ret = ff_request_frame(ctx->inputs[0]);
if (ret == AVERROR_EOF) {
int n_max = atempo->ring;
int n_out;
int err = AVERROR(EAGAIN);
while (err == AVERROR(EAGAIN)) {
if (!atempo->dst_buffer) {
atempo->dst_buffer = ff_get_audio_buffer(outlink, n_max);
if (!atempo->dst_buffer)
return AVERROR(ENOMEM);
atempo->dst = atempo->dst_buffer->data[0];
atempo->dst_end = atempo->dst + n_max * atempo->stride;
}
err = yae_flush(atempo, &atempo->dst, atempo->dst_end);
n_out = ((atempo->dst - atempo->dst_buffer->data[0]) /
atempo->stride);
if (n_out) {
ret = push_samples(atempo, outlink, n_out);
if (ret < 0)
return ret;
}
}
av_frame_free(&atempo->dst_buffer);
atempo->dst = NULL;
atempo->dst_end = NULL;
return AVERROR_EOF;
}
return ret;
}
static int process_command(AVFilterContext *ctx,
const char *cmd,
const char *arg,
char *res,
int res_len,
int flags)
{
int ret = ff_filter_process_command(ctx, cmd, arg, res, res_len, flags);
if (ret < 0)
return ret;
return yae_update(ctx);
}
static const AVFilterPad atempo_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
.config_props = config_props,
},
{ NULL }
};
static const AVFilterPad atempo_outputs[] = {
{
.name = "default",
.request_frame = request_frame,
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
AVFilter ff_af_atempo = {
.name = "atempo",
.description = NULL_IF_CONFIG_SMALL("Adjust audio tempo."),
.init = init,
.uninit = uninit,
.query_formats = query_formats,
.process_command = process_command,
.priv_size = sizeof(ATempoContext),
.priv_class = &atempo_class,
.inputs = atempo_inputs,
.outputs = atempo_outputs,
};
