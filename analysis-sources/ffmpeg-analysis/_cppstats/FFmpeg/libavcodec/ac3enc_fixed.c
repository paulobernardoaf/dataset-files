#define FFT_FLOAT 0
#undef CONFIG_AC3ENC_FLOAT
#include "internal.h"
#include "audiodsp.h"
#include "ac3enc.h"
#include "eac3enc.h"
#define AC3ENC_TYPE AC3ENC_TYPE_AC3_FIXED
#include "ac3enc_opts_template.c"
static const AVClass ac3enc_class = {
.class_name = "Fixed-Point AC-3 Encoder",
.item_name = av_default_item_name,
.option = ac3_options,
.version = LIBAVUTIL_VERSION_INT,
};
static int normalize_samples(AC3EncodeContext *s)
{
int v = s->ac3dsp.ac3_max_msb_abs_int16(s->windowed_samples, AC3_WINDOW_SIZE);
v = 14 - av_log2(v);
if (v > 0)
s->ac3dsp.ac3_lshift_int16(s->windowed_samples, AC3_WINDOW_SIZE, v);
return v + 6;
}
static void scale_coefficients(AC3EncodeContext *s)
{
int blk, ch;
for (blk = 0; blk < s->num_blocks; blk++) {
AC3Block *block = &s->blocks[blk];
for (ch = 1; ch <= s->channels; ch++) {
s->ac3dsp.ac3_rshift_int32(block->mdct_coef[ch], AC3_MAX_COEFS,
block->coeff_shift[ch]);
}
}
}
static void sum_square_butterfly(AC3EncodeContext *s, int64_t sum[4],
const int32_t *coef0, const int32_t *coef1,
int len)
{
s->ac3dsp.sum_square_butterfly_int32(sum, coef0, coef1, len);
}
static void clip_coefficients(AudioDSPContext *adsp, int32_t *coef,
unsigned int len)
{
adsp->vector_clip_int32(coef, coef, COEF_MIN, COEF_MAX, len);
}
static CoefType calc_cpl_coord(CoefSumType energy_ch, CoefSumType energy_cpl)
{
if (energy_cpl <= COEF_MAX) {
return 1048576;
} else {
uint64_t coord = energy_ch / (energy_cpl >> 24);
uint32_t coord32 = FFMIN(coord, 1073741824);
coord32 = ff_sqrt(coord32) << 9;
return FFMIN(coord32, COEF_MAX);
}
}
#include "ac3enc_template.c"
av_cold void ff_ac3_fixed_mdct_end(AC3EncodeContext *s)
{
ff_mdct_end(&s->mdct);
}
av_cold int ff_ac3_fixed_mdct_init(AC3EncodeContext *s)
{
int ret = ff_mdct_init(&s->mdct, 9, 0, -1.0);
s->mdct_window = ff_ac3_window;
return ret;
}
static av_cold int ac3_fixed_encode_init(AVCodecContext *avctx)
{
AC3EncodeContext *s = avctx->priv_data;
s->fixed_point = 1;
return ff_ac3_encode_init(avctx);
}
AVCodec ff_ac3_fixed_encoder = {
.name = "ac3_fixed",
.long_name = NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_AC3,
.priv_data_size = sizeof(AC3EncodeContext),
.init = ac3_fixed_encode_init,
.encode2 = ff_ac3_fixed_encode_frame,
.close = ff_ac3_encode_close,
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16P,
AV_SAMPLE_FMT_NONE },
.priv_class = &ac3enc_class,
.channel_layouts = ff_ac3_channel_layouts,
.defaults = ac3_defaults,
};
