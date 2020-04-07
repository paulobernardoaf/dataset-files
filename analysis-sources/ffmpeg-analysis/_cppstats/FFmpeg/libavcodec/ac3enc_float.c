#define CONFIG_AC3ENC_FLOAT 1
#include "internal.h"
#include "audiodsp.h"
#include "ac3enc.h"
#include "eac3enc.h"
#include "kbdwin.h"
#define AC3ENC_TYPE AC3ENC_TYPE_AC3
#include "ac3enc_opts_template.c"
static const AVClass ac3enc_class = {
.class_name = "AC-3 Encoder",
.item_name = av_default_item_name,
.option = ac3_options,
.version = LIBAVUTIL_VERSION_INT,
};
static void scale_coefficients(AC3EncodeContext *s)
{
int chan_size = AC3_MAX_COEFS * s->num_blocks;
int cpl = s->cpl_on;
s->ac3dsp.float_to_fixed24(s->fixed_coef_buffer + (chan_size * !cpl),
s->mdct_coef_buffer + (chan_size * !cpl),
chan_size * (s->channels + cpl));
}
static void clip_coefficients(AudioDSPContext *adsp, float *coef,
unsigned int len)
{
adsp->vector_clipf(coef, coef, len, COEF_MIN, COEF_MAX);
}
static CoefType calc_cpl_coord(CoefSumType energy_ch, CoefSumType energy_cpl)
{
float coord = 0.125;
if (energy_cpl > 0)
coord *= sqrtf(energy_ch / energy_cpl);
return FFMIN(coord, COEF_MAX);
}
static void sum_square_butterfly(AC3EncodeContext *s, float sum[4],
const float *coef0, const float *coef1,
int len)
{
s->ac3dsp.sum_square_butterfly_float(sum, coef0, coef1, len);
}
#include "ac3enc_template.c"
av_cold void ff_ac3_float_mdct_end(AC3EncodeContext *s)
{
ff_mdct_end(&s->mdct);
av_freep(&s->mdct_window);
}
av_cold int ff_ac3_float_mdct_init(AC3EncodeContext *s)
{
float *window;
int i, n, n2;
n = 1 << 9;
n2 = n >> 1;
window = av_malloc_array(n, sizeof(*window));
if (!window) {
av_log(s->avctx, AV_LOG_ERROR, "Cannot allocate memory.\n");
return AVERROR(ENOMEM);
}
ff_kbd_window_init(window, 5.0, n2);
for (i = 0; i < n2; i++)
window[n-1-i] = window[i];
s->mdct_window = window;
return ff_mdct_init(&s->mdct, 9, 0, -2.0 / n);
}
av_cold int ff_ac3_float_encode_init(AVCodecContext *avctx)
{
AC3EncodeContext *s = avctx->priv_data;
s->fdsp = avpriv_float_dsp_alloc(avctx->flags & AV_CODEC_FLAG_BITEXACT);
if (!s->fdsp)
return AVERROR(ENOMEM);
return ff_ac3_encode_init(avctx);
}
AVCodec ff_ac3_encoder = {
.name = "ac3",
.long_name = NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_AC3,
.priv_data_size = sizeof(AC3EncodeContext),
.init = ff_ac3_float_encode_init,
.encode2 = ff_ac3_float_encode_frame,
.close = ff_ac3_encode_close,
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
.priv_class = &ac3enc_class,
.channel_layouts = ff_ac3_channel_layouts,
.defaults = ac3_defaults,
};
