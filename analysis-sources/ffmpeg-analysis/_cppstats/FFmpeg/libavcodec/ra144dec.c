#include "libavutil/channel_layout.h"
#include "avcodec.h"
#include "get_bits.h"
#include "internal.h"
#include "ra144.h"
static av_cold int ra144_decode_init(AVCodecContext * avctx)
{
RA144Context *ractx = avctx->priv_data;
ractx->avctx = avctx;
ff_audiodsp_init(&ractx->adsp);
ractx->lpc_coef[0] = ractx->lpc_tables[0];
ractx->lpc_coef[1] = ractx->lpc_tables[1];
avctx->channels = 1;
avctx->channel_layout = AV_CH_LAYOUT_MONO;
avctx->sample_fmt = AV_SAMPLE_FMT_S16;
return 0;
}
static void do_output_subblock(RA144Context *ractx, const int16_t *lpc_coefs,
int gval, GetBitContext *gb)
{
int cba_idx = get_bits(gb, 7); 
int gain = get_bits(gb, 8);
int cb1_idx = get_bits(gb, 7);
int cb2_idx = get_bits(gb, 7);
ff_subblock_synthesis(ractx, lpc_coefs, cba_idx, cb1_idx, cb2_idx, gval,
gain);
}
static int ra144_decode_frame(AVCodecContext * avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
static const uint8_t sizes[LPC_ORDER] = {6, 5, 5, 4, 4, 3, 3, 3, 3, 2};
unsigned int refl_rms[NBLOCKS]; 
int16_t block_coefs[NBLOCKS][LPC_ORDER]; 
unsigned int lpc_refl[LPC_ORDER]; 
int i, j;
int ret;
int16_t *samples;
unsigned int energy;
RA144Context *ractx = avctx->priv_data;
GetBitContext gb;
if (buf_size < FRAME_SIZE) {
av_log(avctx, AV_LOG_ERROR,
"Frame too small (%d bytes). Truncated file?\n", buf_size);
*got_frame_ptr = 0;
return AVERROR_INVALIDDATA;
}
frame->nb_samples = NBLOCKS * BLOCKSIZE;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
samples = (int16_t *)frame->data[0];
init_get_bits8(&gb, buf, FRAME_SIZE);
for (i = 0; i < LPC_ORDER; i++)
lpc_refl[i] = ff_lpc_refl_cb[i][get_bits(&gb, sizes[i])];
ff_eval_coefs(ractx->lpc_coef[0], lpc_refl);
ractx->lpc_refl_rms[0] = ff_rms(lpc_refl);
energy = ff_energy_tab[get_bits(&gb, 5)];
refl_rms[0] = ff_interp(ractx, block_coefs[0], 1, 1, ractx->old_energy);
refl_rms[1] = ff_interp(ractx, block_coefs[1], 2,
energy <= ractx->old_energy,
ff_t_sqrt(energy*ractx->old_energy) >> 12);
refl_rms[2] = ff_interp(ractx, block_coefs[2], 3, 0, energy);
refl_rms[3] = ff_rescale_rms(ractx->lpc_refl_rms[0], energy);
ff_int_to_int16(block_coefs[3], ractx->lpc_coef[0]);
for (i=0; i < NBLOCKS; i++) {
do_output_subblock(ractx, block_coefs[i], refl_rms[i], &gb);
for (j=0; j < BLOCKSIZE; j++)
*samples++ = av_clip_int16(ractx->curr_sblock[j + 10] * (1 << 2));
}
ractx->old_energy = energy;
ractx->lpc_refl_rms[1] = ractx->lpc_refl_rms[0];
FFSWAP(unsigned int *, ractx->lpc_coef[0], ractx->lpc_coef[1]);
*got_frame_ptr = 1;
return FRAME_SIZE;
}
AVCodec ff_ra_144_decoder = {
.name = "real_144",
.long_name = NULL_IF_CONFIG_SMALL("RealAudio 1.0 (14.4K)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_RA_144,
.priv_data_size = sizeof(RA144Context),
.init = ra144_decode_init,
.decode = ra144_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
