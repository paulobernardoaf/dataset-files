#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "get_bits.h"
#include "fft.h"
#include "internal.h"
#include "sinewin.h"
#include "atrac.h"
#include "atrac1data.h"
#define AT1_MAX_BFU 52 
#define AT1_SU_SIZE 212 
#define AT1_SU_SAMPLES 512 
#define AT1_FRAME_SIZE AT1_SU_SIZE * 2
#define AT1_SU_MAX_BITS AT1_SU_SIZE * 8
#define AT1_MAX_CHANNELS 2
#define AT1_QMF_BANDS 3
#define IDX_LOW_BAND 0
#define IDX_MID_BAND 1
#define IDX_HIGH_BAND 2
typedef struct AT1SUCtx {
int log2_block_count[AT1_QMF_BANDS]; 
int num_bfus; 
float* spectrum[2];
DECLARE_ALIGNED(32, float, spec1)[AT1_SU_SAMPLES]; 
DECLARE_ALIGNED(32, float, spec2)[AT1_SU_SAMPLES]; 
DECLARE_ALIGNED(32, float, fst_qmf_delay)[46]; 
DECLARE_ALIGNED(32, float, snd_qmf_delay)[46]; 
DECLARE_ALIGNED(32, float, last_qmf_delay)[256+39]; 
} AT1SUCtx;
typedef struct AT1Ctx {
AT1SUCtx SUs[AT1_MAX_CHANNELS]; 
DECLARE_ALIGNED(32, float, spec)[AT1_SU_SAMPLES]; 
DECLARE_ALIGNED(32, float, low)[256];
DECLARE_ALIGNED(32, float, mid)[256];
DECLARE_ALIGNED(32, float, high)[512];
float* bands[3];
FFTContext mdct_ctx[3];
AVFloatDSPContext *fdsp;
} AT1Ctx;
static const uint16_t samples_per_band[3] = {128, 128, 256};
static const uint8_t mdct_long_nbits[3] = {7, 7, 8};
static void at1_imdct(AT1Ctx *q, float *spec, float *out, int nbits,
int rev_spec)
{
FFTContext* mdct_context = &q->mdct_ctx[nbits - 5 - (nbits > 6)];
int transf_size = 1 << nbits;
if (rev_spec) {
int i;
for (i = 0; i < transf_size / 2; i++)
FFSWAP(float, spec[i], spec[transf_size - 1 - i]);
}
mdct_context->imdct_half(mdct_context, out, spec);
}
static int at1_imdct_block(AT1SUCtx* su, AT1Ctx *q)
{
int band_num, band_samples, log2_block_count, nbits, num_blocks, block_size;
unsigned int start_pos, ref_pos = 0, pos = 0;
for (band_num = 0; band_num < AT1_QMF_BANDS; band_num++) {
float *prev_buf;
int j;
band_samples = samples_per_band[band_num];
log2_block_count = su->log2_block_count[band_num];
num_blocks = 1 << log2_block_count;
if (num_blocks == 1) {
block_size = band_samples >> log2_block_count;
nbits = mdct_long_nbits[band_num] - log2_block_count;
if (nbits != 5 && nbits != 7 && nbits != 8)
return AVERROR_INVALIDDATA;
} else {
block_size = 32;
nbits = 5;
}
start_pos = 0;
prev_buf = &su->spectrum[1][ref_pos + band_samples - 16];
for (j=0; j < num_blocks; j++) {
at1_imdct(q, &q->spec[pos], &su->spectrum[0][ref_pos + start_pos], nbits, band_num);
q->fdsp->vector_fmul_window(&q->bands[band_num][start_pos], prev_buf,
&su->spectrum[0][ref_pos + start_pos], ff_sine_32, 16);
prev_buf = &su->spectrum[0][ref_pos+start_pos + 16];
start_pos += block_size;
pos += block_size;
}
if (num_blocks == 1)
memcpy(q->bands[band_num] + 32, &su->spectrum[0][ref_pos + 16], 240 * sizeof(float));
ref_pos += band_samples;
}
FFSWAP(float*, su->spectrum[0], su->spectrum[1]);
return 0;
}
static int at1_parse_bsm(GetBitContext* gb, int log2_block_cnt[AT1_QMF_BANDS])
{
int log2_block_count_tmp, i;
for (i = 0; i < 2; i++) {
log2_block_count_tmp = get_bits(gb, 2);
if (log2_block_count_tmp & 1)
return AVERROR_INVALIDDATA;
log2_block_cnt[i] = 2 - log2_block_count_tmp;
}
log2_block_count_tmp = get_bits(gb, 2);
if (log2_block_count_tmp != 0 && log2_block_count_tmp != 3)
return AVERROR_INVALIDDATA;
log2_block_cnt[IDX_HIGH_BAND] = 3 - log2_block_count_tmp;
skip_bits(gb, 2);
return 0;
}
static int at1_unpack_dequant(GetBitContext* gb, AT1SUCtx* su,
float spec[AT1_SU_SAMPLES])
{
int bits_used, band_num, bfu_num, i;
uint8_t idwls[AT1_MAX_BFU]; 
uint8_t idsfs[AT1_MAX_BFU]; 
su->num_bfus = bfu_amount_tab1[get_bits(gb, 3)];
bits_used = su->num_bfus * 10 + 32 +
bfu_amount_tab2[get_bits(gb, 2)] +
(bfu_amount_tab3[get_bits(gb, 3)] << 1);
for (i = 0; i < su->num_bfus; i++)
idwls[i] = get_bits(gb, 4);
for (i = 0; i < su->num_bfus; i++)
idsfs[i] = get_bits(gb, 6);
for (i = su->num_bfus; i < AT1_MAX_BFU; i++)
idwls[i] = idsfs[i] = 0;
for (band_num = 0; band_num < AT1_QMF_BANDS; band_num++) {
for (bfu_num = bfu_bands_t[band_num]; bfu_num < bfu_bands_t[band_num+1]; bfu_num++) {
int pos;
int num_specs = specs_per_bfu[bfu_num];
int word_len = !!idwls[bfu_num] + idwls[bfu_num];
float scale_factor = ff_atrac_sf_table[idsfs[bfu_num]];
bits_used += word_len * num_specs; 
if (bits_used > AT1_SU_MAX_BITS)
return AVERROR_INVALIDDATA;
pos = su->log2_block_count[band_num] ? bfu_start_short[bfu_num] : bfu_start_long[bfu_num];
if (word_len) {
float max_quant = 1.0 / (float)((1 << (word_len - 1)) - 1);
for (i = 0; i < num_specs; i++) {
spec[pos+i] = get_sbits(gb, word_len) * scale_factor * max_quant;
}
} else { 
memset(&spec[pos], 0, num_specs * sizeof(float));
}
}
}
return 0;
}
static void at1_subband_synthesis(AT1Ctx *q, AT1SUCtx* su, float *pOut)
{
float temp[256];
float iqmf_temp[512 + 46];
ff_atrac_iqmf(q->bands[0], q->bands[1], 128, temp, su->fst_qmf_delay, iqmf_temp);
memcpy( su->last_qmf_delay, &su->last_qmf_delay[256], sizeof(float) * 39);
memcpy(&su->last_qmf_delay[39], q->bands[2], sizeof(float) * 256);
ff_atrac_iqmf(temp, su->last_qmf_delay, 256, pOut, su->snd_qmf_delay, iqmf_temp);
}
static int atrac1_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
AT1Ctx *q = avctx->priv_data;
int ch, ret;
GetBitContext gb;
if (buf_size < 212 * avctx->channels) {
av_log(avctx, AV_LOG_ERROR, "Not enough data to decode!\n");
return AVERROR_INVALIDDATA;
}
frame->nb_samples = AT1_SU_SAMPLES;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
for (ch = 0; ch < avctx->channels; ch++) {
AT1SUCtx* su = &q->SUs[ch];
init_get_bits(&gb, &buf[212 * ch], 212 * 8);
ret = at1_parse_bsm(&gb, su->log2_block_count);
if (ret < 0)
return ret;
ret = at1_unpack_dequant(&gb, su, q->spec);
if (ret < 0)
return ret;
ret = at1_imdct_block(su, q);
if (ret < 0)
return ret;
at1_subband_synthesis(q, su, (float *)frame->extended_data[ch]);
}
*got_frame_ptr = 1;
return avctx->block_align;
}
static av_cold int atrac1_decode_end(AVCodecContext * avctx)
{
AT1Ctx *q = avctx->priv_data;
ff_mdct_end(&q->mdct_ctx[0]);
ff_mdct_end(&q->mdct_ctx[1]);
ff_mdct_end(&q->mdct_ctx[2]);
av_freep(&q->fdsp);
return 0;
}
static av_cold int atrac1_decode_init(AVCodecContext *avctx)
{
AT1Ctx *q = avctx->priv_data;
int ret;
avctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
if (avctx->channels < 1 || avctx->channels > AT1_MAX_CHANNELS) {
av_log(avctx, AV_LOG_ERROR, "Unsupported number of channels: %d\n",
avctx->channels);
return AVERROR(EINVAL);
}
if (avctx->block_align <= 0) {
av_log(avctx, AV_LOG_ERROR, "Unsupported block align.");
return AVERROR_PATCHWELCOME;
}
if ((ret = ff_mdct_init(&q->mdct_ctx[0], 6, 1, -1.0/ (1 << 15))) ||
(ret = ff_mdct_init(&q->mdct_ctx[1], 8, 1, -1.0/ (1 << 15))) ||
(ret = ff_mdct_init(&q->mdct_ctx[2], 9, 1, -1.0/ (1 << 15)))) {
av_log(avctx, AV_LOG_ERROR, "Error initializing MDCT\n");
atrac1_decode_end(avctx);
return ret;
}
ff_init_ff_sine_windows(5);
ff_atrac_generate_tables();
q->fdsp = avpriv_float_dsp_alloc(avctx->flags & AV_CODEC_FLAG_BITEXACT);
q->bands[0] = q->low;
q->bands[1] = q->mid;
q->bands[2] = q->high;
q->SUs[0].spectrum[0] = q->SUs[0].spec1;
q->SUs[0].spectrum[1] = q->SUs[0].spec2;
q->SUs[1].spectrum[0] = q->SUs[1].spec1;
q->SUs[1].spectrum[1] = q->SUs[1].spec2;
return 0;
}
AVCodec ff_atrac1_decoder = {
.name = "atrac1",
.long_name = NULL_IF_CONFIG_SMALL("ATRAC1 (Adaptive TRansform Acoustic Coding)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_ATRAC1,
.priv_data_size = sizeof(AT1Ctx),
.init = atrac1_decode_init,
.close = atrac1_decode_end,
.decode = atrac1_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.sample_fmts = (const enum AVSampleFormat[]) { AV_SAMPLE_FMT_FLTP,
AV_SAMPLE_FMT_NONE },
};
