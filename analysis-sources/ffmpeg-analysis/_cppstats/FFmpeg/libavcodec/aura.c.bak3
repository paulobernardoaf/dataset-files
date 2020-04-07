
























#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"

static av_cold int aura_decode_init(AVCodecContext *avctx)
{

if (avctx->width & 0x3)
return AVERROR(EINVAL);
avctx->pix_fmt = AV_PIX_FMT_YUV422P;

return 0;
}

static int aura_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *pkt)
{
AVFrame *frame = data;
uint8_t *Y, *U, *V;
uint8_t val;
int x, y, ret;
const uint8_t *buf = pkt->data;


const int8_t *delta_table = (const int8_t*)buf + 16;

if (pkt->size != 48 + avctx->height * avctx->width) {
av_log(avctx, AV_LOG_ERROR, "got a buffer with %d bytes when %d were expected\n",
pkt->size, 48 + avctx->height * avctx->width);
return AVERROR_INVALIDDATA;
}


buf += 48;

if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

Y = frame->data[0];
U = frame->data[1];
V = frame->data[2];


for (y = 0; y < avctx->height; y++) {

val = *buf++;
U[0] = val & 0xF0;
Y[0] = val << 4;
val = *buf++;
V[0] = val & 0xF0;
Y[1] = Y[0] + delta_table[val & 0xF];
Y += 2; U++; V++;


for (x = 1; x < (avctx->width >> 1); x++) {
val = *buf++;
U[0] = U[-1] + delta_table[val >> 4];
Y[0] = Y[-1] + delta_table[val & 0xF];
val = *buf++;
V[0] = V[-1] + delta_table[val >> 4];
Y[1] = Y[ 0] + delta_table[val & 0xF];
Y += 2; U++; V++;
}
Y += frame->linesize[0] - avctx->width;
U += frame->linesize[1] - (avctx->width >> 1);
V += frame->linesize[2] - (avctx->width >> 1);
}

*got_frame = 1;

return pkt->size;
}

AVCodec ff_aura2_decoder = {
.name = "aura2",
.long_name = NULL_IF_CONFIG_SMALL("Auravision Aura 2"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_AURA2,
.init = aura_decode_init,
.decode = aura_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
