#include <string.h>
#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"
#include "libavutil/xga_font_data.h"
#include "cga_data.h"
static int tmv_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *src = avpkt->data;
uint8_t *dst;
unsigned char_cols = avctx->width >> 3;
unsigned char_rows = avctx->height >> 3;
unsigned x, y, fg, bg, c;
int ret;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
if (avpkt->size < 2*char_rows*char_cols) {
av_log(avctx, AV_LOG_ERROR,
"Input buffer too small, truncated sample?\n");
*got_frame = 0;
return AVERROR_INVALIDDATA;
}
frame->pict_type = AV_PICTURE_TYPE_I;
frame->key_frame = 1;
dst = frame->data[0];
frame->palette_has_changed = 1;
memcpy(frame->data[1], ff_cga_palette, 16 * 4);
memset(frame->data[1] + 16 * 4, 0, AVPALETTE_SIZE - 16 * 4);
for (y = 0; y < char_rows; y++) {
for (x = 0; x < char_cols; x++) {
c = *src++;
bg = *src >> 4;
fg = *src++ & 0xF;
ff_draw_pc_font(dst + x * 8, frame->linesize[0],
avpriv_cga_font, 8, c, fg, bg);
}
dst += frame->linesize[0] * 8;
}
*got_frame = 1;
return avpkt->size;
}
static av_cold int tmv_decode_init(AVCodecContext *avctx)
{
avctx->pix_fmt = AV_PIX_FMT_PAL8;
return 0;
}
AVCodec ff_tmv_decoder = {
.name = "tmv",
.long_name = NULL_IF_CONFIG_SMALL("8088flex TMV"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_TMV,
.init = tmv_decode_init,
.decode = tmv_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
