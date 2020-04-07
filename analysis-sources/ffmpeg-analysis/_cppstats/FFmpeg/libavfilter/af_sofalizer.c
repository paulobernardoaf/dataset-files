#include <math.h>
#include <mysofa.h>
#include "libavcodec/avfft.h"
#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/intmath.h"
#include "libavutil/opt.h"
#include "avfilter.h"
#include "filters.h"
#include "internal.h"
#include "audio.h"
#define TIME_DOMAIN 0
#define FREQUENCY_DOMAIN 1
typedef struct MySofa { 
struct MYSOFA_HRTF *hrtf;
struct MYSOFA_LOOKUP *lookup;
struct MYSOFA_NEIGHBORHOOD *neighborhood;
int ir_samples; 
int n_samples; 
float *lir, *rir; 
float *fir;
int max_delay;
} MySofa;
typedef struct VirtualSpeaker {
uint8_t set;
float azim;
float elev;
} VirtualSpeaker;
typedef struct SOFAlizerContext {
const AVClass *class;
char *filename; 
MySofa sofa; 
int sample_rate; 
float *speaker_azim; 
float *speaker_elev; 
char *speakers_pos; 
float lfe_gain; 
float gain_lfe; 
int lfe_channel; 
int n_conv; 
float *ringbuffer[2]; 
int write[2]; 
int buffer_length; 
int n_fft; 
int nb_samples;
int *delay[2]; 
float *data_ir[2]; 
float *temp_src[2];
FFTComplex *temp_fft[2]; 
FFTComplex *temp_afft[2]; 
float gain; 
float rotation; 
float elevation; 
float radius; 
int type; 
int framesize; 
int normalize; 
int interpolate; 
int minphase; 
float anglestep; 
float radstep; 
VirtualSpeaker vspkrpos[64];
FFTContext *fft[2], *ifft[2];
FFTComplex *data_hrtf[2];
AVFloatDSPContext *fdsp;
} SOFAlizerContext;
static int close_sofa(struct MySofa *sofa)
{
if (sofa->neighborhood)
mysofa_neighborhood_free(sofa->neighborhood);
sofa->neighborhood = NULL;
if (sofa->lookup)
mysofa_lookup_free(sofa->lookup);
sofa->lookup = NULL;
if (sofa->hrtf)
mysofa_free(sofa->hrtf);
sofa->hrtf = NULL;
av_freep(&sofa->fir);
return 0;
}
static int preload_sofa(AVFilterContext *ctx, char *filename, int *samplingrate)
{
struct SOFAlizerContext *s = ctx->priv;
struct MYSOFA_HRTF *mysofa;
char *license;
int ret;
mysofa = mysofa_load(filename, &ret);
s->sofa.hrtf = mysofa;
if (ret || !mysofa) {
av_log(ctx, AV_LOG_ERROR, "Can't find SOFA-file '%s'\n", filename);
return AVERROR(EINVAL);
}
ret = mysofa_check(mysofa);
if (ret != MYSOFA_OK) {
av_log(ctx, AV_LOG_ERROR, "Selected SOFA file is invalid. Please select valid SOFA file.\n");
return ret;
}
if (s->normalize)
mysofa_loudness(s->sofa.hrtf);
if (s->minphase)
mysofa_minphase(s->sofa.hrtf, 0.01f);
mysofa_tocartesian(s->sofa.hrtf);
s->sofa.lookup = mysofa_lookup_init(s->sofa.hrtf);
if (s->sofa.lookup == NULL)
return AVERROR(EINVAL);
if (s->interpolate)
s->sofa.neighborhood = mysofa_neighborhood_init_withstepdefine(s->sofa.hrtf,
s->sofa.lookup,
s->anglestep,
s->radstep);
s->sofa.fir = av_calloc(s->sofa.hrtf->N * s->sofa.hrtf->R, sizeof(*s->sofa.fir));
if (!s->sofa.fir)
return AVERROR(ENOMEM);
if (mysofa->DataSamplingRate.elements != 1)
return AVERROR(EINVAL);
av_log(ctx, AV_LOG_DEBUG, "Original IR length: %d.\n", mysofa->N);
*samplingrate = mysofa->DataSamplingRate.values[0];
license = mysofa_getAttribute(mysofa->attributes, (char *)"License");
if (license)
av_log(ctx, AV_LOG_INFO, "SOFA license: %s\n", license);
return 0;
}
static int parse_channel_name(char **arg, int *rchannel, char *buf)
{
int len, i, channel_id = 0;
int64_t layout, layout0;
if (av_sscanf(*arg, "%7[A-Z]%n", buf, &len)) {
layout0 = layout = av_get_channel_layout(buf);
for (i = 32; i > 0; i >>= 1) {
if (layout >= 1LL << i) {
channel_id += i;
layout >>= i;
}
}
if (channel_id >= 64 || layout0 != 1LL << channel_id)
return AVERROR(EINVAL);
*rchannel = channel_id;
*arg += len;
return 0;
}
return AVERROR(EINVAL);
}
static void parse_speaker_pos(AVFilterContext *ctx, int64_t in_channel_layout)
{
SOFAlizerContext *s = ctx->priv;
char *arg, *tokenizer, *p, *args = av_strdup(s->speakers_pos);
if (!args)
return;
p = args;
while ((arg = av_strtok(p, "|", &tokenizer))) {
char buf[8];
float azim, elev;
int out_ch_id;
p = NULL;
if (parse_channel_name(&arg, &out_ch_id, buf)) {
av_log(ctx, AV_LOG_WARNING, "Failed to parse \'%s\' as channel name.\n", buf);
continue;
}
if (av_sscanf(arg, "%f %f", &azim, &elev) == 2) {
s->vspkrpos[out_ch_id].set = 1;
s->vspkrpos[out_ch_id].azim = azim;
s->vspkrpos[out_ch_id].elev = elev;
} else if (av_sscanf(arg, "%f", &azim) == 1) {
s->vspkrpos[out_ch_id].set = 1;
s->vspkrpos[out_ch_id].azim = azim;
s->vspkrpos[out_ch_id].elev = 0;
}
}
av_free(args);
}
static int get_speaker_pos(AVFilterContext *ctx,
float *speaker_azim, float *speaker_elev)
{
struct SOFAlizerContext *s = ctx->priv;
uint64_t channels_layout = ctx->inputs[0]->channel_layout;
float azim[16] = { 0 };
float elev[16] = { 0 };
int m, ch, n_conv = ctx->inputs[0]->channels; 
if (n_conv > 16)
return AVERROR(EINVAL);
s->lfe_channel = -1;
if (s->speakers_pos)
parse_speaker_pos(ctx, channels_layout);
for (m = 0, ch = 0; ch < n_conv && m < 64; m++) {
uint64_t mask = channels_layout & (1ULL << m);
switch (mask) {
case AV_CH_FRONT_LEFT: azim[ch] = 30; break;
case AV_CH_FRONT_RIGHT: azim[ch] = 330; break;
case AV_CH_FRONT_CENTER: azim[ch] = 0; break;
case AV_CH_LOW_FREQUENCY:
case AV_CH_LOW_FREQUENCY_2: s->lfe_channel = ch; break;
case AV_CH_BACK_LEFT: azim[ch] = 150; break;
case AV_CH_BACK_RIGHT: azim[ch] = 210; break;
case AV_CH_BACK_CENTER: azim[ch] = 180; break;
case AV_CH_SIDE_LEFT: azim[ch] = 90; break;
case AV_CH_SIDE_RIGHT: azim[ch] = 270; break;
case AV_CH_FRONT_LEFT_OF_CENTER: azim[ch] = 15; break;
case AV_CH_FRONT_RIGHT_OF_CENTER: azim[ch] = 345; break;
case AV_CH_TOP_CENTER: azim[ch] = 0;
elev[ch] = 90; break;
case AV_CH_TOP_FRONT_LEFT: azim[ch] = 30;
elev[ch] = 45; break;
case AV_CH_TOP_FRONT_CENTER: azim[ch] = 0;
elev[ch] = 45; break;
case AV_CH_TOP_FRONT_RIGHT: azim[ch] = 330;
elev[ch] = 45; break;
case AV_CH_TOP_BACK_LEFT: azim[ch] = 150;
elev[ch] = 45; break;
case AV_CH_TOP_BACK_RIGHT: azim[ch] = 210;
elev[ch] = 45; break;
case AV_CH_TOP_BACK_CENTER: azim[ch] = 180;
elev[ch] = 45; break;
case AV_CH_WIDE_LEFT: azim[ch] = 90; break;
case AV_CH_WIDE_RIGHT: azim[ch] = 270; break;
case AV_CH_SURROUND_DIRECT_LEFT: azim[ch] = 90; break;
case AV_CH_SURROUND_DIRECT_RIGHT: azim[ch] = 270; break;
case AV_CH_STEREO_LEFT: azim[ch] = 90; break;
case AV_CH_STEREO_RIGHT: azim[ch] = 270; break;
case 0: break;
default:
return AVERROR(EINVAL);
}
if (s->vspkrpos[m].set) {
azim[ch] = s->vspkrpos[m].azim;
elev[ch] = s->vspkrpos[m].elev;
}
if (mask)
ch++;
}
memcpy(speaker_azim, azim, n_conv * sizeof(float));
memcpy(speaker_elev, elev, n_conv * sizeof(float));
return 0;
}
typedef struct ThreadData {
AVFrame *in, *out;
int *write;
int **delay;
float **ir;
int *n_clippings;
float **ringbuffer;
float **temp_src;
FFTComplex **temp_fft;
FFTComplex **temp_afft;
} ThreadData;
static int sofalizer_convolute(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs)
{
SOFAlizerContext *s = ctx->priv;
ThreadData *td = arg;
AVFrame *in = td->in, *out = td->out;
int offset = jobnr;
int *write = &td->write[jobnr];
const int *const delay = td->delay[jobnr];
const float *const ir = td->ir[jobnr];
int *n_clippings = &td->n_clippings[jobnr];
float *ringbuffer = td->ringbuffer[jobnr];
float *temp_src = td->temp_src[jobnr];
const int ir_samples = s->sofa.ir_samples; 
const int n_samples = s->sofa.n_samples;
const int planar = in->format == AV_SAMPLE_FMT_FLTP;
const int mult = 1 + !planar;
const float *src = (const float *)in->extended_data[0]; 
float *dst = (float *)out->extended_data[jobnr * planar]; 
const int in_channels = s->n_conv; 
const int buffer_length = s->buffer_length;
const uint32_t modulo = (uint32_t)buffer_length - 1;
float *buffer[16]; 
int wr = *write;
int read;
int i, l;
if (!planar)
dst += offset;
for (l = 0; l < in_channels; l++) {
buffer[l] = ringbuffer + l * buffer_length;
}
for (i = 0; i < in->nb_samples; i++) {
const float *temp_ir = ir; 
dst[0] = 0;
if (planar) {
for (l = 0; l < in_channels; l++) {
const float *srcp = (const float *)in->extended_data[l];
buffer[l][wr] = srcp[i];
}
} else {
for (l = 0; l < in_channels; l++) {
buffer[l][wr] = src[l];
}
}
for (l = 0; l < in_channels; l++) {
const float *const bptr = buffer[l];
if (l == s->lfe_channel) {
dst[0] += *(buffer[s->lfe_channel] + wr) * s->gain_lfe;
temp_ir += n_samples;
continue;
}
read = (wr - delay[l] - (ir_samples - 1) + buffer_length) & modulo;
if (read + ir_samples < buffer_length) {
memmove(temp_src, bptr + read, ir_samples * sizeof(*temp_src));
} else {
int len = FFMIN(n_samples - (read % ir_samples), buffer_length - read);
memmove(temp_src, bptr + read, len * sizeof(*temp_src));
memmove(temp_src + len, bptr, (n_samples - len) * sizeof(*temp_src));
}
dst[0] += s->fdsp->scalarproduct_float(temp_ir, temp_src, FFALIGN(ir_samples, 32));
temp_ir += n_samples;
}
if (fabsf(dst[0]) > 1)
n_clippings[0]++;
dst += mult;
src += in_channels;
wr = (wr + 1) & modulo; 
}
*write = wr; 
return 0;
}
static int sofalizer_fast_convolute(AVFilterContext *ctx, void *arg, int jobnr, int nb_jobs)
{
SOFAlizerContext *s = ctx->priv;
ThreadData *td = arg;
AVFrame *in = td->in, *out = td->out;
int offset = jobnr;
int *write = &td->write[jobnr];
FFTComplex *hrtf = s->data_hrtf[jobnr]; 
int *n_clippings = &td->n_clippings[jobnr];
float *ringbuffer = td->ringbuffer[jobnr];
const int ir_samples = s->sofa.ir_samples; 
const int planar = in->format == AV_SAMPLE_FMT_FLTP;
const int mult = 1 + !planar;
float *dst = (float *)out->extended_data[jobnr * planar]; 
const int in_channels = s->n_conv; 
const int buffer_length = s->buffer_length;
const uint32_t modulo = (uint32_t)buffer_length - 1;
FFTComplex *fft_in = s->temp_fft[jobnr]; 
FFTComplex *fft_acc = s->temp_afft[jobnr];
FFTContext *ifft = s->ifft[jobnr];
FFTContext *fft = s->fft[jobnr];
const int n_conv = s->n_conv;
const int n_fft = s->n_fft;
const float fft_scale = 1.0f / s->n_fft;
FFTComplex *hrtf_offset;
int wr = *write;
int n_read;
int i, j;
if (!planar)
dst += offset;
n_read = FFMIN(ir_samples, in->nb_samples);
for (j = 0; j < n_read; j++) {
dst[mult * j] = ringbuffer[wr];
ringbuffer[wr] = 0.0f; 
wr = (wr + 1) & modulo;
}
for (j = n_read; j < in->nb_samples; j++) {
dst[mult * j] = 0;
}
memset(fft_acc, 0, sizeof(FFTComplex) * n_fft);
for (i = 0; i < n_conv; i++) {
const float *src = (const float *)in->extended_data[i * planar]; 
if (i == s->lfe_channel) { 
if (in->format == AV_SAMPLE_FMT_FLT) {
for (j = 0; j < in->nb_samples; j++) {
dst[2 * j] += src[i + j * in_channels] * s->gain_lfe;
}
} else {
for (j = 0; j < in->nb_samples; j++) {
dst[j] += src[j] * s->gain_lfe;
}
}
continue;
}
offset = i * n_fft; 
hrtf_offset = hrtf + offset;
memset(fft_in, 0, sizeof(FFTComplex) * n_fft);
if (in->format == AV_SAMPLE_FMT_FLT) {
for (j = 0; j < in->nb_samples; j++) {
fft_in[j].re = src[j * in_channels + i];
}
} else {
for (j = 0; j < in->nb_samples; j++) {
fft_in[j].re = src[j];
}
}
av_fft_permute(fft, fft_in);
av_fft_calc(fft, fft_in);
for (j = 0; j < n_fft; j++) {
const FFTComplex *hcomplex = hrtf_offset + j;
const float re = fft_in[j].re;
const float im = fft_in[j].im;
fft_acc[j].re += re * hcomplex->re - im * hcomplex->im;
fft_acc[j].im += re * hcomplex->im + im * hcomplex->re;
}
}
av_fft_permute(ifft, fft_acc);
av_fft_calc(ifft, fft_acc);
for (j = 0; j < in->nb_samples; j++) {
dst[mult * j] += fft_acc[j].re * fft_scale;
}
for (j = 0; j < ir_samples - 1; j++) { 
int write_pos = (wr + j) & modulo;
*(ringbuffer + write_pos) += fft_acc[in->nb_samples + j].re * fft_scale;
}
for (i = 0; i < out->nb_samples; i++) {
if (fabsf(dst[i * mult]) > 1) { 
n_clippings[0]++;
}
}
*write = wr;
return 0;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *in)
{
AVFilterContext *ctx = inlink->dst;
SOFAlizerContext *s = ctx->priv;
AVFilterLink *outlink = ctx->outputs[0];
int n_clippings[2] = { 0 };
ThreadData td;
AVFrame *out;
out = ff_get_audio_buffer(outlink, in->nb_samples);
if (!out) {
av_frame_free(&in);
return AVERROR(ENOMEM);
}
av_frame_copy_props(out, in);
td.in = in; td.out = out; td.write = s->write;
td.delay = s->delay; td.ir = s->data_ir; td.n_clippings = n_clippings;
td.ringbuffer = s->ringbuffer; td.temp_src = s->temp_src;
td.temp_fft = s->temp_fft;
td.temp_afft = s->temp_afft;
if (s->type == TIME_DOMAIN) {
ctx->internal->execute(ctx, sofalizer_convolute, &td, NULL, 2);
} else if (s->type == FREQUENCY_DOMAIN) {
ctx->internal->execute(ctx, sofalizer_fast_convolute, &td, NULL, 2);
}
emms_c();
if (n_clippings[0] + n_clippings[1] > 0) {
av_log(ctx, AV_LOG_WARNING, "%d of %d samples clipped. Please reduce gain.\n",
n_clippings[0] + n_clippings[1], out->nb_samples * 2);
}
av_frame_free(&in);
return ff_filter_frame(outlink, out);
}
static int activate(AVFilterContext *ctx)
{
AVFilterLink *inlink = ctx->inputs[0];
AVFilterLink *outlink = ctx->outputs[0];
SOFAlizerContext *s = ctx->priv;
AVFrame *in;
int ret;
FF_FILTER_FORWARD_STATUS_BACK(outlink, inlink);
if (s->nb_samples)
ret = ff_inlink_consume_samples(inlink, s->nb_samples, s->nb_samples, &in);
else
ret = ff_inlink_consume_frame(inlink, &in);
if (ret < 0)
return ret;
if (ret > 0)
return filter_frame(inlink, in);
FF_FILTER_FORWARD_STATUS(inlink, outlink);
FF_FILTER_FORWARD_WANTED(outlink, inlink);
return FFERROR_NOT_READY;
}
static int query_formats(AVFilterContext *ctx)
{
struct SOFAlizerContext *s = ctx->priv;
AVFilterFormats *formats = NULL;
AVFilterChannelLayouts *layouts = NULL;
int ret, sample_rates[] = { 48000, -1 };
static const enum AVSampleFormat sample_fmts[] = {
AV_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE
};
formats = ff_make_format_list(sample_fmts);
if (!formats)
return AVERROR(ENOMEM);
ret = ff_set_common_formats(ctx, formats);
if (ret)
return ret;
layouts = ff_all_channel_layouts();
if (!layouts)
return AVERROR(ENOMEM);
ret = ff_channel_layouts_ref(layouts, &ctx->inputs[0]->out_channel_layouts);
if (ret)
return ret;
layouts = NULL;
ret = ff_add_channel_layout(&layouts, AV_CH_LAYOUT_STEREO);
if (ret)
return ret;
ret = ff_channel_layouts_ref(layouts, &ctx->outputs[0]->in_channel_layouts);
if (ret)
return ret;
sample_rates[0] = s->sample_rate;
formats = ff_make_format_list(sample_rates);
if (!formats)
return AVERROR(ENOMEM);
return ff_set_common_samplerates(ctx, formats);
}
static int getfilter_float(AVFilterContext *ctx, float x, float y, float z,
float *left, float *right,
float *delay_left, float *delay_right)
{
struct SOFAlizerContext *s = ctx->priv;
float c[3], delays[2];
float *fl, *fr;
int nearest;
int *neighbors;
float *res;
c[0] = x, c[1] = y, c[2] = z;
nearest = mysofa_lookup(s->sofa.lookup, c);
if (nearest < 0)
return AVERROR(EINVAL);
if (s->interpolate) {
neighbors = mysofa_neighborhood(s->sofa.neighborhood, nearest);
res = mysofa_interpolate(s->sofa.hrtf, c,
nearest, neighbors,
s->sofa.fir, delays);
} else {
if (s->sofa.hrtf->DataDelay.elements > s->sofa.hrtf->R) {
delays[0] = s->sofa.hrtf->DataDelay.values[nearest * s->sofa.hrtf->R];
delays[1] = s->sofa.hrtf->DataDelay.values[nearest * s->sofa.hrtf->R + 1];
} else {
delays[0] = s->sofa.hrtf->DataDelay.values[0];
delays[1] = s->sofa.hrtf->DataDelay.values[1];
}
res = s->sofa.hrtf->DataIR.values + nearest * s->sofa.hrtf->N * s->sofa.hrtf->R;
}
*delay_left = delays[0];
*delay_right = delays[1];
fl = res;
fr = res + s->sofa.hrtf->N;
memcpy(left, fl, sizeof(float) * s->sofa.hrtf->N);
memcpy(right, fr, sizeof(float) * s->sofa.hrtf->N);
return 0;
}
static int load_data(AVFilterContext *ctx, int azim, int elev, float radius, int sample_rate)
{
struct SOFAlizerContext *s = ctx->priv;
int n_samples;
int ir_samples;
int n_conv = s->n_conv; 
int n_fft;
float delay_l; 
float delay_r;
int nb_input_channels = ctx->inputs[0]->channels; 
float gain_lin = expf((s->gain - 3 * nb_input_channels) / 20 * M_LN10); 
FFTComplex *data_hrtf_l = NULL;
FFTComplex *data_hrtf_r = NULL;
FFTComplex *fft_in_l = NULL;
FFTComplex *fft_in_r = NULL;
float *data_ir_l = NULL;
float *data_ir_r = NULL;
int offset = 0; 
int i, j, azim_orig = azim, elev_orig = elev;
int ret = 0;
int n_current;
int n_max = 0;
av_log(ctx, AV_LOG_DEBUG, "IR length: %d.\n", s->sofa.hrtf->N);
s->sofa.ir_samples = s->sofa.hrtf->N;
s->sofa.n_samples = 1 << (32 - ff_clz(s->sofa.ir_samples));
n_samples = s->sofa.n_samples;
ir_samples = s->sofa.ir_samples;
if (s->type == TIME_DOMAIN) {
s->data_ir[0] = av_calloc(n_samples, sizeof(float) * s->n_conv);
s->data_ir[1] = av_calloc(n_samples, sizeof(float) * s->n_conv);
if (!s->data_ir[0] || !s->data_ir[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
}
s->delay[0] = av_calloc(s->n_conv, sizeof(int));
s->delay[1] = av_calloc(s->n_conv, sizeof(int));
if (!s->delay[0] || !s->delay[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
data_ir_l = av_calloc(n_conv * n_samples, sizeof(*data_ir_l));
data_ir_r = av_calloc(n_conv * n_samples, sizeof(*data_ir_r));
if (!data_ir_r || !data_ir_l) {
ret = AVERROR(ENOMEM);
goto fail;
}
if (s->type == TIME_DOMAIN) {
s->temp_src[0] = av_calloc(n_samples, sizeof(float));
s->temp_src[1] = av_calloc(n_samples, sizeof(float));
if (!s->temp_src[0] || !s->temp_src[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
}
s->speaker_azim = av_calloc(s->n_conv, sizeof(*s->speaker_azim));
s->speaker_elev = av_calloc(s->n_conv, sizeof(*s->speaker_elev));
if (!s->speaker_azim || !s->speaker_elev) {
ret = AVERROR(ENOMEM);
goto fail;
}
if ((ret = get_speaker_pos(ctx, s->speaker_azim, s->speaker_elev)) < 0) {
av_log(ctx, AV_LOG_ERROR, "Couldn't get speaker positions. Input channel configuration not supported.\n");
goto fail;
}
for (i = 0; i < s->n_conv; i++) {
float coordinates[3];
azim = (int)(s->speaker_azim[i] + azim_orig) % 360;
elev = (int)(s->speaker_elev[i] + elev_orig) % 90;
coordinates[0] = azim;
coordinates[1] = elev;
coordinates[2] = radius;
mysofa_s2c(coordinates);
ret = getfilter_float(ctx, coordinates[0], coordinates[1], coordinates[2],
data_ir_l + n_samples * i,
data_ir_r + n_samples * i,
&delay_l, &delay_r);
if (ret < 0)
goto fail;
s->delay[0][i] = delay_l * sample_rate;
s->delay[1][i] = delay_r * sample_rate;
s->sofa.max_delay = FFMAX3(s->sofa.max_delay, s->delay[0][i], s->delay[1][i]);
}
n_current = n_samples + s->sofa.max_delay;
n_max = FFMAX(n_max, n_current);
s->buffer_length = 1 << (32 - ff_clz(n_max));
s->n_fft = n_fft = 1 << (32 - ff_clz(n_max + s->framesize));
if (s->type == FREQUENCY_DOMAIN) {
av_fft_end(s->fft[0]);
av_fft_end(s->fft[1]);
s->fft[0] = av_fft_init(av_log2(s->n_fft), 0);
s->fft[1] = av_fft_init(av_log2(s->n_fft), 0);
av_fft_end(s->ifft[0]);
av_fft_end(s->ifft[1]);
s->ifft[0] = av_fft_init(av_log2(s->n_fft), 1);
s->ifft[1] = av_fft_init(av_log2(s->n_fft), 1);
if (!s->fft[0] || !s->fft[1] || !s->ifft[0] || !s->ifft[1]) {
av_log(ctx, AV_LOG_ERROR, "Unable to create FFT contexts of size %d.\n", s->n_fft);
ret = AVERROR(ENOMEM);
goto fail;
}
}
if (s->type == TIME_DOMAIN) {
s->ringbuffer[0] = av_calloc(s->buffer_length, sizeof(float) * nb_input_channels);
s->ringbuffer[1] = av_calloc(s->buffer_length, sizeof(float) * nb_input_channels);
} else if (s->type == FREQUENCY_DOMAIN) {
data_hrtf_l = av_malloc_array(n_fft, sizeof(*data_hrtf_l) * n_conv);
data_hrtf_r = av_malloc_array(n_fft, sizeof(*data_hrtf_r) * n_conv);
if (!data_hrtf_r || !data_hrtf_l) {
ret = AVERROR(ENOMEM);
goto fail;
}
s->ringbuffer[0] = av_calloc(s->buffer_length, sizeof(float));
s->ringbuffer[1] = av_calloc(s->buffer_length, sizeof(float));
s->temp_fft[0] = av_malloc_array(s->n_fft, sizeof(FFTComplex));
s->temp_fft[1] = av_malloc_array(s->n_fft, sizeof(FFTComplex));
s->temp_afft[0] = av_malloc_array(s->n_fft, sizeof(FFTComplex));
s->temp_afft[1] = av_malloc_array(s->n_fft, sizeof(FFTComplex));
if (!s->temp_fft[0] || !s->temp_fft[1] ||
!s->temp_afft[0] || !s->temp_afft[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
}
if (!s->ringbuffer[0] || !s->ringbuffer[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
if (s->type == FREQUENCY_DOMAIN) {
fft_in_l = av_calloc(n_fft, sizeof(*fft_in_l));
fft_in_r = av_calloc(n_fft, sizeof(*fft_in_r));
if (!fft_in_l || !fft_in_r) {
ret = AVERROR(ENOMEM);
goto fail;
}
}
for (i = 0; i < s->n_conv; i++) {
float *lir, *rir;
offset = i * n_samples; 
lir = data_ir_l + offset;
rir = data_ir_r + offset;
if (s->type == TIME_DOMAIN) {
for (j = 0; j < ir_samples; j++) {
s->data_ir[0][offset + j] = lir[ir_samples - 1 - j] * gain_lin;
s->data_ir[1][offset + j] = rir[ir_samples - 1 - j] * gain_lin;
}
} else if (s->type == FREQUENCY_DOMAIN) {
memset(fft_in_l, 0, n_fft * sizeof(*fft_in_l));
memset(fft_in_r, 0, n_fft * sizeof(*fft_in_r));
offset = i * n_fft; 
for (j = 0; j < ir_samples; j++) {
fft_in_l[s->delay[0][i] + j].re = lir[j] * gain_lin;
fft_in_r[s->delay[1][i] + j].re = rir[j] * gain_lin;
}
av_fft_permute(s->fft[0], fft_in_l);
av_fft_calc(s->fft[0], fft_in_l);
memcpy(data_hrtf_l + offset, fft_in_l, n_fft * sizeof(*fft_in_l));
av_fft_permute(s->fft[0], fft_in_r);
av_fft_calc(s->fft[0], fft_in_r);
memcpy(data_hrtf_r + offset, fft_in_r, n_fft * sizeof(*fft_in_r));
}
}
if (s->type == FREQUENCY_DOMAIN) {
s->data_hrtf[0] = av_malloc_array(n_fft * s->n_conv, sizeof(FFTComplex));
s->data_hrtf[1] = av_malloc_array(n_fft * s->n_conv, sizeof(FFTComplex));
if (!s->data_hrtf[0] || !s->data_hrtf[1]) {
ret = AVERROR(ENOMEM);
goto fail;
}
memcpy(s->data_hrtf[0], data_hrtf_l, 
sizeof(FFTComplex) * n_conv * n_fft); 
memcpy(s->data_hrtf[1], data_hrtf_r,
sizeof(FFTComplex) * n_conv * n_fft);
}
fail:
av_freep(&data_hrtf_l); 
av_freep(&data_hrtf_r);
av_freep(&data_ir_l); 
av_freep(&data_ir_r);
av_freep(&fft_in_l); 
av_freep(&fft_in_r);
return ret;
}
static av_cold int init(AVFilterContext *ctx)
{
SOFAlizerContext *s = ctx->priv;
int ret;
if (!s->filename) {
av_log(ctx, AV_LOG_ERROR, "Valid SOFA filename must be set.\n");
return AVERROR(EINVAL);
}
ret = preload_sofa(ctx, s->filename, &s->sample_rate);
if (ret) {
av_log(ctx, AV_LOG_ERROR, "Error while loading SOFA file: '%s'\n", s->filename);
} else { 
av_log(ctx, AV_LOG_DEBUG, "File '%s' loaded.\n", s->filename);
}
if (ret) {
av_log(ctx, AV_LOG_ERROR, "No valid SOFA file could be loaded. Please specify valid SOFA file.\n");
return ret;
}
s->fdsp = avpriv_float_dsp_alloc(0);
if (!s->fdsp)
return AVERROR(ENOMEM);
return 0;
}
static int config_input(AVFilterLink *inlink)
{
AVFilterContext *ctx = inlink->dst;
SOFAlizerContext *s = ctx->priv;
int ret;
if (s->type == FREQUENCY_DOMAIN)
s->nb_samples = s->framesize;
s->gain_lfe = expf((s->gain - 3 * inlink->channels + s->lfe_gain) / 20 * M_LN10);
s->n_conv = inlink->channels;
if ((ret = load_data(ctx, s->rotation, s->elevation, s->radius, inlink->sample_rate)) < 0)
return ret;
av_log(ctx, AV_LOG_DEBUG, "Samplerate: %d Channels to convolute: %d, Length of ringbuffer: %d x %d\n",
inlink->sample_rate, s->n_conv, inlink->channels, s->buffer_length);
return 0;
}
static av_cold void uninit(AVFilterContext *ctx)
{
SOFAlizerContext *s = ctx->priv;
close_sofa(&s->sofa);
av_fft_end(s->ifft[0]);
av_fft_end(s->ifft[1]);
av_fft_end(s->fft[0]);
av_fft_end(s->fft[1]);
s->ifft[0] = NULL;
s->ifft[1] = NULL;
s->fft[0] = NULL;
s->fft[1] = NULL;
av_freep(&s->delay[0]);
av_freep(&s->delay[1]);
av_freep(&s->data_ir[0]);
av_freep(&s->data_ir[1]);
av_freep(&s->ringbuffer[0]);
av_freep(&s->ringbuffer[1]);
av_freep(&s->speaker_azim);
av_freep(&s->speaker_elev);
av_freep(&s->temp_src[0]);
av_freep(&s->temp_src[1]);
av_freep(&s->temp_afft[0]);
av_freep(&s->temp_afft[1]);
av_freep(&s->temp_fft[0]);
av_freep(&s->temp_fft[1]);
av_freep(&s->data_hrtf[0]);
av_freep(&s->data_hrtf[1]);
av_freep(&s->fdsp);
}
#define OFFSET(x) offsetof(SOFAlizerContext, x)
#define FLAGS AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_FILTERING_PARAM
static const AVOption sofalizer_options[] = {
{ "sofa", "sofa filename", OFFSET(filename), AV_OPT_TYPE_STRING, {.str=NULL}, .flags = FLAGS },
{ "gain", "set gain in dB", OFFSET(gain), AV_OPT_TYPE_FLOAT, {.dbl=0}, -20, 40, .flags = FLAGS },
{ "rotation", "set rotation" , OFFSET(rotation), AV_OPT_TYPE_FLOAT, {.dbl=0}, -360, 360, .flags = FLAGS },
{ "elevation", "set elevation", OFFSET(elevation), AV_OPT_TYPE_FLOAT, {.dbl=0}, -90, 90, .flags = FLAGS },
{ "radius", "set radius", OFFSET(radius), AV_OPT_TYPE_FLOAT, {.dbl=1}, 0, 5, .flags = FLAGS },
{ "type", "set processing", OFFSET(type), AV_OPT_TYPE_INT, {.i64=1}, 0, 1, .flags = FLAGS, "type" },
{ "time", "time domain", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, .flags = FLAGS, "type" },
{ "freq", "frequency domain", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, .flags = FLAGS, "type" },
{ "speakers", "set speaker custom positions", OFFSET(speakers_pos), AV_OPT_TYPE_STRING, {.str=0}, 0, 0, .flags = FLAGS },
{ "lfegain", "set lfe gain", OFFSET(lfe_gain), AV_OPT_TYPE_FLOAT, {.dbl=0}, -20,40, .flags = FLAGS },
{ "framesize", "set frame size", OFFSET(framesize), AV_OPT_TYPE_INT, {.i64=1024},1024,96000, .flags = FLAGS },
{ "normalize", "normalize IRs", OFFSET(normalize), AV_OPT_TYPE_BOOL, {.i64=1}, 0, 1, .flags = FLAGS },
{ "interpolate","interpolate IRs from neighbors", OFFSET(interpolate),AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1, .flags = FLAGS },
{ "minphase", "minphase IRs", OFFSET(minphase), AV_OPT_TYPE_BOOL, {.i64=0}, 0, 1, .flags = FLAGS },
{ "anglestep", "set neighbor search angle step", OFFSET(anglestep), AV_OPT_TYPE_FLOAT, {.dbl=.5}, 0.01, 10, .flags = FLAGS },
{ "radstep", "set neighbor search radius step", OFFSET(radstep), AV_OPT_TYPE_FLOAT, {.dbl=.01}, 0.01, 1, .flags = FLAGS },
{ NULL }
};
AVFILTER_DEFINE_CLASS(sofalizer);
static const AVFilterPad inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.config_props = config_input,
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
AVFilter ff_af_sofalizer = {
.name = "sofalizer",
.description = NULL_IF_CONFIG_SMALL("SOFAlizer (Spatially Oriented Format for Acoustics)."),
.priv_size = sizeof(SOFAlizerContext),
.priv_class = &sofalizer_class,
.init = init,
.activate = activate,
.uninit = uninit,
.query_formats = query_formats,
.inputs = inputs,
.outputs = outputs,
.flags = AVFILTER_FLAG_SLICE_THREADS,
};
