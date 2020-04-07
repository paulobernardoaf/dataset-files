#include "libavutil/channel_layout.h"
#include "avfilter.h"
#include "audio.h"
#include "formats.h"
#define NUMTAPS 64
static const int8_t filt[NUMTAPS] = {
4, -6, 
4, -11, 
-1, -5, 
3, 3, 
-2, 5, 
-5, 0,
9, 1,
6, 3, 
-4, -1, 
-5, -3, 
-2, -5, 
-7, 1, 
6, -7, 
30, -29, 
12, -3, 
-11, 4, 
-3, 7, 
-20, 23, 
2, 0, 
1, -6, 
-14, -5, 
15, -18, 
6, 7, 
15, -10, 
-14, 22, 
-7, -2, 
-4, 9, 
6, -12, 
6, -6, 
0, -11,
0, -5,
4, 0};
typedef struct EarwaxContext {
int16_t taps[NUMTAPS * 2];
} EarwaxContext;
static int query_formats(AVFilterContext *ctx)
{
static const int sample_rates[] = { 44100, -1 };
int ret;
AVFilterFormats *formats = NULL;
AVFilterChannelLayouts *layout = NULL;
if ((ret = ff_add_format (&formats, AV_SAMPLE_FMT_S16 )) < 0 ||
(ret = ff_set_common_formats (ctx , formats )) < 0 ||
(ret = ff_add_channel_layout (&layout , AV_CH_LAYOUT_STEREO )) < 0 ||
(ret = ff_set_common_channel_layouts (ctx , layout )) < 0 ||
(ret = ff_set_common_samplerates (ctx , ff_make_format_list(sample_rates) )) < 0)
return ret;
return 0;
}
static inline int16_t *scalarproduct(const int16_t *in, const int16_t *endin, int16_t *out)
{
int32_t sample;
int16_t j;
while (in < endin) {
sample = 0;
for (j = 0; j < NUMTAPS; j++)
sample += in[j] * filt[j];
*out = av_clip_int16(sample >> 6);
out++;
in++;
}
return out;
}
static int filter_frame(AVFilterLink *inlink, AVFrame *insamples)
{
AVFilterLink *outlink = inlink->dst->outputs[0];
int16_t *taps, *endin, *in, *out;
AVFrame *outsamples = ff_get_audio_buffer(outlink, insamples->nb_samples);
int len;
if (!outsamples) {
av_frame_free(&insamples);
return AVERROR(ENOMEM);
}
av_frame_copy_props(outsamples, insamples);
taps = ((EarwaxContext *)inlink->dst->priv)->taps;
out = (int16_t *)outsamples->data[0];
in = (int16_t *)insamples ->data[0];
len = FFMIN(NUMTAPS, 2*insamples->nb_samples);
memcpy(taps+NUMTAPS, in, len * sizeof(*taps));
out = scalarproduct(taps, taps + len, out);
if (2*insamples->nb_samples >= NUMTAPS ){
endin = in + insamples->nb_samples * 2 - NUMTAPS;
scalarproduct(in, endin, out);
memcpy(taps, endin, NUMTAPS * sizeof(*taps));
} else
memmove(taps, taps + 2*insamples->nb_samples, NUMTAPS * sizeof(*taps));
av_frame_free(&insamples);
return ff_filter_frame(outlink, outsamples);
}
static const AVFilterPad earwax_inputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
.filter_frame = filter_frame,
},
{ NULL }
};
static const AVFilterPad earwax_outputs[] = {
{
.name = "default",
.type = AVMEDIA_TYPE_AUDIO,
},
{ NULL }
};
AVFilter ff_af_earwax = {
.name = "earwax",
.description = NULL_IF_CONFIG_SMALL("Widen the stereo image."),
.query_formats = query_formats,
.priv_size = sizeof(EarwaxContext),
.inputs = earwax_inputs,
.outputs = earwax_outputs,
};
