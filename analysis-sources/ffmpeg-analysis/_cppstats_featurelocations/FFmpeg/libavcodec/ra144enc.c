


























#include <float.h>

#include "avcodec.h"
#include "audio_frame_queue.h"
#include "celp_filters.h"
#include "internal.h"
#include "mathops.h"
#include "put_bits.h"
#include "ra144.h"

static av_cold int ra144_encode_close(AVCodecContext *avctx)
{
RA144Context *ractx = avctx->priv_data;
ff_lpc_end(&ractx->lpc_ctx);
ff_af_queue_close(&ractx->afq);
return 0;
}


static av_cold int ra144_encode_init(AVCodecContext * avctx)
{
RA144Context *ractx;
int ret;

if (avctx->channels != 1) {
av_log(avctx, AV_LOG_ERROR, "invalid number of channels: %d\n",
avctx->channels);
return -1;
}
avctx->frame_size = NBLOCKS * BLOCKSIZE;
avctx->initial_padding = avctx->frame_size;
avctx->bit_rate = 8000;
ractx = avctx->priv_data;
ractx->lpc_coef[0] = ractx->lpc_tables[0];
ractx->lpc_coef[1] = ractx->lpc_tables[1];
ractx->avctx = avctx;
ff_audiodsp_init(&ractx->adsp);
ret = ff_lpc_init(&ractx->lpc_ctx, avctx->frame_size, LPC_ORDER,
FF_LPC_TYPE_LEVINSON);
if (ret < 0)
goto error;

ff_af_queue_init(avctx, &ractx->afq);

return 0;
error:
ra144_encode_close(avctx);
return ret;
}












static int quantize(int value, const int16_t *table, unsigned int size)
{
unsigned int low = 0, high = size - 1;

while (1) {
int index = (low + high) >> 1;
int error = table[index] - value;

if (index == low)
return table[high] + error > value ? low : high;
if (error > 0) {
high = index;
} else {
low = index;
}
}
}








static void orthogonalize(float *v, const float *u)
{
int i;
float num = 0, den = 0;

for (i = 0; i < BLOCKSIZE; i++) {
num += v[i] * u[i];
den += u[i] * u[i];
}
num /= den;
for (i = 0; i < BLOCKSIZE; i++)
v[i] -= num * u[i];
}















static void get_match_score(float *work, const float *coefs, float *vect,
const float *ortho1, const float *ortho2,
const float *data, float *score, float *gain)
{
float c, g;
int i;

ff_celp_lp_synthesis_filterf(work, coefs, vect, BLOCKSIZE, LPC_ORDER);
if (ortho1)
orthogonalize(work, ortho1);
if (ortho2)
orthogonalize(work, ortho2);
c = g = 0;
for (i = 0; i < BLOCKSIZE; i++) {
g += work[i] * work[i];
c += data[i] * work[i];
}
if (c <= 0) {
*score = 0;
return;
}
*gain = c / g;
*score = *gain * c;
}









static void create_adapt_vect(float *vect, const int16_t *cb, int lag)
{
int i;

cb += BUFFERSIZE - lag;
for (i = 0; i < FFMIN(BLOCKSIZE, lag); i++)
vect[i] = cb[i];
if (lag < BLOCKSIZE)
for (i = 0; i < BLOCKSIZE - lag; i++)
vect[lag + i] = cb[i];
}












static int adaptive_cb_search(const int16_t *adapt_cb, float *work,
const float *coefs, float *data)
{
int i, av_uninit(best_vect);
float score, gain, best_score, av_uninit(best_gain);
float exc[BLOCKSIZE];

gain = best_score = 0;
for (i = BLOCKSIZE / 2; i <= BUFFERSIZE; i++) {
create_adapt_vect(exc, adapt_cb, i);
get_match_score(work, coefs, exc, NULL, NULL, data, &score, &gain);
if (score > best_score) {
best_score = score;
best_vect = i;
best_gain = gain;
}
}
if (!best_score)
return 0;





create_adapt_vect(exc, adapt_cb, best_vect);
ff_celp_lp_synthesis_filterf(work, coefs, exc, BLOCKSIZE, LPC_ORDER);
for (i = 0; i < BLOCKSIZE; i++)
data[i] -= best_gain * work[i];
return best_vect - BLOCKSIZE / 2 + 1;
}


















static void find_best_vect(float *work, const float *coefs,
const int8_t cb[][BLOCKSIZE], const float *ortho1,
const float *ortho2, float *data, int *idx,
float *gain)
{
int i, j;
float g, score, best_score;
float vect[BLOCKSIZE];

*idx = *gain = best_score = 0;
for (i = 0; i < FIXED_CB_SIZE; i++) {
for (j = 0; j < BLOCKSIZE; j++)
vect[j] = cb[i][j];
get_match_score(work, coefs, vect, ortho1, ortho2, data, &score, &g);
if (score > best_score) {
best_score = score;
*idx = i;
*gain = g;
}
}
}














static void fixed_cb_search(float *work, const float *coefs, float *data,
int cba_idx, int *cb1_idx, int *cb2_idx)
{
int i, ortho_cb1;
float gain;
float cba_vect[BLOCKSIZE], cb1_vect[BLOCKSIZE];
float vect[BLOCKSIZE];





if (cba_idx)
memcpy(cba_vect, work, sizeof(cba_vect));

find_best_vect(work, coefs, ff_cb1_vects, cba_idx ? cba_vect : NULL, NULL,
data, cb1_idx, &gain);





if (gain) {
for (i = 0; i < BLOCKSIZE; i++)
vect[i] = ff_cb1_vects[*cb1_idx][i];
ff_celp_lp_synthesis_filterf(work, coefs, vect, BLOCKSIZE, LPC_ORDER);
if (cba_idx)
orthogonalize(work, cba_vect);
for (i = 0; i < BLOCKSIZE; i++)
data[i] -= gain * work[i];
memcpy(cb1_vect, work, sizeof(cb1_vect));
ortho_cb1 = 1;
} else
ortho_cb1 = 0;

find_best_vect(work, coefs, ff_cb2_vects, cba_idx ? cba_vect : NULL,
ortho_cb1 ? cb1_vect : NULL, data, cb2_idx, &gain);
}











static void ra144_encode_subblock(RA144Context *ractx,
const int16_t *sblock_data,
const int16_t *lpc_coefs, unsigned int rms,
PutBitContext *pb)
{
float data[BLOCKSIZE] = { 0 }, work[LPC_ORDER + BLOCKSIZE];
float coefs[LPC_ORDER];
float zero[BLOCKSIZE], cba[BLOCKSIZE], cb1[BLOCKSIZE], cb2[BLOCKSIZE];
int cba_idx, cb1_idx, cb2_idx, gain;
int i, n;
unsigned m[3];
float g[3];
float error, best_error;

for (i = 0; i < LPC_ORDER; i++) {
work[i] = ractx->curr_sblock[BLOCKSIZE + i];
coefs[i] = lpc_coefs[i] * (1/4096.0);
}





ff_celp_lp_synthesis_filterf(work + LPC_ORDER, coefs, data, BLOCKSIZE,
LPC_ORDER);
for (i = 0; i < BLOCKSIZE; i++) {
zero[i] = work[LPC_ORDER + i];
data[i] = sblock_data[i] - zero[i];
}






memset(work, 0, LPC_ORDER * sizeof(*work));

cba_idx = adaptive_cb_search(ractx->adapt_cb, work + LPC_ORDER, coefs,
data);
if (cba_idx) {




memcpy(cba, work + LPC_ORDER, sizeof(cba));

ff_copy_and_dup(ractx->buffer_a, ractx->adapt_cb, cba_idx + BLOCKSIZE / 2 - 1);
m[0] = (ff_irms(&ractx->adsp, ractx->buffer_a) * rms) >> 12;
}
fixed_cb_search(work + LPC_ORDER, coefs, data, cba_idx, &cb1_idx, &cb2_idx);
for (i = 0; i < BLOCKSIZE; i++) {
cb1[i] = ff_cb1_vects[cb1_idx][i];
cb2[i] = ff_cb2_vects[cb2_idx][i];
}
ff_celp_lp_synthesis_filterf(work + LPC_ORDER, coefs, cb1, BLOCKSIZE,
LPC_ORDER);
memcpy(cb1, work + LPC_ORDER, sizeof(cb1));
m[1] = (ff_cb1_base[cb1_idx] * rms) >> 8;
ff_celp_lp_synthesis_filterf(work + LPC_ORDER, coefs, cb2, BLOCKSIZE,
LPC_ORDER);
memcpy(cb2, work + LPC_ORDER, sizeof(cb2));
m[2] = (ff_cb2_base[cb2_idx] * rms) >> 8;
best_error = FLT_MAX;
gain = 0;
for (n = 0; n < 256; n++) {
g[1] = ((ff_gain_val_tab[n][1] * m[1]) >> ff_gain_exp_tab[n]) *
(1/4096.0);
g[2] = ((ff_gain_val_tab[n][2] * m[2]) >> ff_gain_exp_tab[n]) *
(1/4096.0);
error = 0;
if (cba_idx) {
g[0] = ((ff_gain_val_tab[n][0] * m[0]) >> ff_gain_exp_tab[n]) *
(1/4096.0);
for (i = 0; i < BLOCKSIZE; i++) {
data[i] = zero[i] + g[0] * cba[i] + g[1] * cb1[i] +
g[2] * cb2[i];
error += (data[i] - sblock_data[i]) *
(data[i] - sblock_data[i]);
}
} else {
for (i = 0; i < BLOCKSIZE; i++) {
data[i] = zero[i] + g[1] * cb1[i] + g[2] * cb2[i];
error += (data[i] - sblock_data[i]) *
(data[i] - sblock_data[i]);
}
}
if (error < best_error) {
best_error = error;
gain = n;
}
}
put_bits(pb, 7, cba_idx);
put_bits(pb, 8, gain);
put_bits(pb, 7, cb1_idx);
put_bits(pb, 7, cb2_idx);
ff_subblock_synthesis(ractx, lpc_coefs, cba_idx, cb1_idx, cb2_idx, rms,
gain);
}


static int ra144_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr)
{
static const uint8_t sizes[LPC_ORDER] = {64, 32, 32, 16, 16, 8, 8, 8, 8, 4};
static const uint8_t bit_sizes[LPC_ORDER] = {6, 5, 5, 4, 4, 3, 3, 3, 3, 2};
RA144Context *ractx = avctx->priv_data;
PutBitContext pb;
int32_t lpc_data[NBLOCKS * BLOCKSIZE];
int32_t lpc_coefs[LPC_ORDER][MAX_LPC_ORDER];
int shift[LPC_ORDER];
int16_t block_coefs[NBLOCKS][LPC_ORDER];
int lpc_refl[LPC_ORDER]; 
unsigned int refl_rms[NBLOCKS]; 
const int16_t *samples = frame ? (const int16_t *)frame->data[0] : NULL;
int energy = 0;
int i, idx, ret;

if (ractx->last_frame)
return 0;

if ((ret = ff_alloc_packet2(avctx, avpkt, FRAME_SIZE, 0)) < 0)
return ret;








for (i = 0; i < (2 * BLOCKSIZE + BLOCKSIZE / 2); i++) {
lpc_data[i] = ractx->curr_block[BLOCKSIZE + BLOCKSIZE / 2 + i];
energy += (lpc_data[i] * lpc_data[i]) >> 4;
}
if (frame) {
int j;
for (j = 0; j < frame->nb_samples && i < NBLOCKS * BLOCKSIZE; i++, j++) {
lpc_data[i] = samples[j] >> 2;
energy += (lpc_data[i] * lpc_data[i]) >> 4;
}
}
if (i < NBLOCKS * BLOCKSIZE)
memset(&lpc_data[i], 0, (NBLOCKS * BLOCKSIZE - i) * sizeof(*lpc_data));
energy = ff_energy_tab[quantize(ff_t_sqrt(energy >> 5) >> 10, ff_energy_tab,
32)];

ff_lpc_calc_coefs(&ractx->lpc_ctx, lpc_data, NBLOCKS * BLOCKSIZE, LPC_ORDER,
LPC_ORDER, 16, lpc_coefs, shift, FF_LPC_TYPE_LEVINSON,
0, ORDER_METHOD_EST, 0, 12, 0);
for (i = 0; i < LPC_ORDER; i++)
block_coefs[NBLOCKS - 1][i] = -lpc_coefs[LPC_ORDER - 1][i]
* (1 << (12 - shift[LPC_ORDER - 1]));






if (ff_eval_refl(lpc_refl, block_coefs[NBLOCKS - 1], avctx)) {



ff_int_to_int16(block_coefs[NBLOCKS - 1], ractx->lpc_coef[1]);
if (ff_eval_refl(lpc_refl, block_coefs[NBLOCKS - 1], avctx)) {

memset(lpc_refl, 0, sizeof(lpc_refl));
}
}
init_put_bits(&pb, avpkt->data, avpkt->size);
for (i = 0; i < LPC_ORDER; i++) {
idx = quantize(lpc_refl[i], ff_lpc_refl_cb[i], sizes[i]);
put_bits(&pb, bit_sizes[i], idx);
lpc_refl[i] = ff_lpc_refl_cb[i][idx];
}
ractx->lpc_refl_rms[0] = ff_rms(lpc_refl);
ff_eval_coefs(ractx->lpc_coef[0], lpc_refl);
refl_rms[0] = ff_interp(ractx, block_coefs[0], 1, 1, ractx->old_energy);
refl_rms[1] = ff_interp(ractx, block_coefs[1], 2,
energy <= ractx->old_energy,
ff_t_sqrt(energy * ractx->old_energy) >> 12);
refl_rms[2] = ff_interp(ractx, block_coefs[2], 3, 0, energy);
refl_rms[3] = ff_rescale_rms(ractx->lpc_refl_rms[0], energy);
ff_int_to_int16(block_coefs[NBLOCKS - 1], ractx->lpc_coef[0]);
put_bits(&pb, 5, quantize(energy, ff_energy_tab, 32));
for (i = 0; i < NBLOCKS; i++)
ra144_encode_subblock(ractx, ractx->curr_block + i * BLOCKSIZE,
block_coefs[i], refl_rms[i], &pb);
flush_put_bits(&pb);
ractx->old_energy = energy;
ractx->lpc_refl_rms[1] = ractx->lpc_refl_rms[0];
FFSWAP(unsigned int *, ractx->lpc_coef[0], ractx->lpc_coef[1]);


i = 0;
if (frame) {
for (; i < frame->nb_samples; i++)
ractx->curr_block[i] = samples[i] >> 2;

if ((ret = ff_af_queue_add(&ractx->afq, frame)) < 0)
return ret;
} else
ractx->last_frame = 1;
memset(&ractx->curr_block[i], 0,
(NBLOCKS * BLOCKSIZE - i) * sizeof(*ractx->curr_block));


ff_af_queue_remove(&ractx->afq, avctx->frame_size, &avpkt->pts,
&avpkt->duration);

avpkt->size = FRAME_SIZE;
*got_packet_ptr = 1;
return 0;
}


AVCodec ff_ra_144_encoder = {
.name = "real_144",
.long_name = NULL_IF_CONFIG_SMALL("RealAudio 1.0 (14.4K)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_RA_144,
.priv_data_size = sizeof(RA144Context),
.init = ra144_encode_init,
.encode2 = ra144_encode_frame,
.close = ra144_encode_close,
.capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_SMALL_LAST_FRAME,
.sample_fmts = (const enum AVSampleFormat[]){ AV_SAMPLE_FMT_S16,
AV_SAMPLE_FMT_NONE },
.supported_samplerates = (const int[]){ 8000, 0 },
.channel_layouts = (const uint64_t[]) { AV_CH_LAYOUT_MONO, 0 },
};
