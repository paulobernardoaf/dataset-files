




















#include "libavutil/channel_layout.h"
#include "libavutil/float_dsp.h"
#include "libavutil/internal.h"

#define BITSTREAM_READER_LE
#include "avcodec.h"
#include "celp_filters.h"
#include "get_bits.h"
#include "internal.h"
#include "lpc.h"
#include "ra288.h"

#define MAX_BACKWARD_FILTER_ORDER 36
#define MAX_BACKWARD_FILTER_LEN 40
#define MAX_BACKWARD_FILTER_NONREC 35

#define RA288_BLOCK_SIZE 5
#define RA288_BLOCKS_PER_FRAME 32

typedef struct RA288Context {
AVFloatDSPContext *fdsp;
DECLARE_ALIGNED(32, float, sp_lpc)[FFALIGN(36, 16)]; 
DECLARE_ALIGNED(32, float, gain_lpc)[FFALIGN(10, 16)]; 




float sp_hist[111];


float sp_rec[37];




float gain_hist[38];


float gain_rec[11];
} RA288Context;

static av_cold int ra288_decode_close(AVCodecContext *avctx)
{
RA288Context *ractx = avctx->priv_data;

av_freep(&ractx->fdsp);

return 0;
}

static av_cold int ra288_decode_init(AVCodecContext *avctx)
{
RA288Context *ractx = avctx->priv_data;

avctx->channels = 1;
avctx->channel_layout = AV_CH_LAYOUT_MONO;
avctx->sample_fmt = AV_SAMPLE_FMT_FLT;

if (avctx->block_align != 38) {
av_log(avctx, AV_LOG_ERROR, "unsupported block align\n");
return AVERROR_PATCHWELCOME;
}

ractx->fdsp = avpriv_float_dsp_alloc(avctx->flags & AV_CODEC_FLAG_BITEXACT);
if (!ractx->fdsp)
return AVERROR(ENOMEM);

return 0;
}

static void convolve(float *tgt, const float *src, int len, int n)
{
for (; n >= 0; n--)
tgt[n] = avpriv_scalarproduct_float_c(src, src - n, len);

}

static void decode(RA288Context *ractx, float gain, int cb_coef)
{
int i;
double sumsum;
float sum, buffer[5];
float *block = ractx->sp_hist + 70 + 36; 
float *gain_block = ractx->gain_hist + 28;

memmove(ractx->sp_hist + 70, ractx->sp_hist + 75, 36*sizeof(*block));


sum = 32.0;
for (i=0; i < 10; i++)
sum -= gain_block[9-i] * ractx->gain_lpc[i];


sum = av_clipf(sum, 0, 60);



sumsum = exp(sum * 0.1151292546497) * gain * (1.0/(1<<23));

for (i=0; i < 5; i++)
buffer[i] = codetable[cb_coef][i] * sumsum;

sum = avpriv_scalarproduct_float_c(buffer, buffer, 5);

sum = FFMAX(sum, 5.0 / (1<<24));


memmove(gain_block, gain_block + 1, 9 * sizeof(*gain_block));

gain_block[9] = 10 * log10(sum) + (10*log10(((1<<24)/5.)) - 32);

ff_celp_lp_synthesis_filterf(block, ractx->sp_lpc, buffer, 5, 36);
}













static void do_hybrid_window(RA288Context *ractx,
int order, int n, int non_rec, float *out,
float *hist, float *out2, const float *window)
{
int i;
float buffer1[MAX_BACKWARD_FILTER_ORDER + 1];
float buffer2[MAX_BACKWARD_FILTER_ORDER + 1];
LOCAL_ALIGNED(32, float, work, [FFALIGN(MAX_BACKWARD_FILTER_ORDER +
MAX_BACKWARD_FILTER_LEN +
MAX_BACKWARD_FILTER_NONREC, 16)]);

av_assert2(order>=0);

ractx->fdsp->vector_fmul(work, window, hist, FFALIGN(order + n + non_rec, 16));

convolve(buffer1, work + order , n , order);
convolve(buffer2, work + order + n, non_rec, order);

for (i=0; i <= order; i++) {
out2[i] = out2[i] * 0.5625 + buffer1[i];
out [i] = out2[i] + buffer2[i];
}


*out *= 257.0 / 256.0;
}




static void backward_filter(RA288Context *ractx,
float *hist, float *rec, const float *window,
float *lpc, const float *tab,
int order, int n, int non_rec, int move_size)
{
float temp[MAX_BACKWARD_FILTER_ORDER+1];

do_hybrid_window(ractx, order, n, non_rec, temp, hist, rec, window);

if (!compute_lpc_coefs(temp, order, lpc, 0, 1, 1))
ractx->fdsp->vector_fmul(lpc, lpc, tab, FFALIGN(order, 16));

memmove(hist, hist + n, move_size*sizeof(*hist));
}

static int ra288_decode_frame(AVCodecContext * avctx, void *data,
int *got_frame_ptr, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
float *out;
int i, ret;
RA288Context *ractx = avctx->priv_data;
GetBitContext gb;

if (buf_size < avctx->block_align) {
av_log(avctx, AV_LOG_ERROR,
"Error! Input buffer is too small [%d<%d]\n",
buf_size, avctx->block_align);
return AVERROR_INVALIDDATA;
}

ret = init_get_bits8(&gb, buf, avctx->block_align);
if (ret < 0)
return ret;


frame->nb_samples = RA288_BLOCK_SIZE * RA288_BLOCKS_PER_FRAME;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
out = (float *)frame->data[0];

for (i=0; i < RA288_BLOCKS_PER_FRAME; i++) {
float gain = amptable[get_bits(&gb, 3)];
int cb_coef = get_bits(&gb, 6 + (i&1));

decode(ractx, gain, cb_coef);

memcpy(out, &ractx->sp_hist[70 + 36], RA288_BLOCK_SIZE * sizeof(*out));
out += RA288_BLOCK_SIZE;

if ((i & 7) == 3) {
backward_filter(ractx, ractx->sp_hist, ractx->sp_rec, syn_window,
ractx->sp_lpc, syn_bw_tab, 36, 40, 35, 70);

backward_filter(ractx, ractx->gain_hist, ractx->gain_rec, gain_window,
ractx->gain_lpc, gain_bw_tab, 10, 8, 20, 28);
}
}

*got_frame_ptr = 1;

return avctx->block_align;
}

AVCodec ff_ra_288_decoder = {
.name = "real_288",
.long_name = NULL_IF_CONFIG_SMALL("RealAudio 2.0 (28.8K)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_RA_288,
.priv_data_size = sizeof(RA288Context),
.init = ra288_decode_init,
.decode = ra288_decode_frame,
.close = ra288_decode_close,
.capabilities = AV_CODEC_CAP_DR1,
};
