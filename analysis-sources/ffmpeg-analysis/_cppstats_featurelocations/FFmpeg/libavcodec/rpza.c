



































#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

typedef struct RpzaContext {

AVCodecContext *avctx;
AVFrame *frame;

GetByteContext gb;
} RpzaContext;

#define CHECK_BLOCK() if (total_blocks < 1) { av_log(s->avctx, AV_LOG_ERROR, "Block counter just went negative (this should not happen)\n"); return AVERROR_INVALIDDATA; } 






#define ADVANCE_BLOCK() { pixel_ptr += 4; if (pixel_ptr >= width) { pixel_ptr = 0; row_ptr += stride * 4; } total_blocks--; }










static int rpza_decode_stream(RpzaContext *s)
{
int width = s->avctx->width;
int stride, row_inc, ret;
int chunk_size;
uint16_t colorA = 0, colorB;
uint16_t color4[4];
uint16_t ta, tb;
uint16_t *pixels;

int row_ptr = 0;
int pixel_ptr = 0;
int block_ptr;
int pixel_x, pixel_y;
int total_blocks;


if (bytestream2_peek_byte(&s->gb) != 0xe1)
av_log(s->avctx, AV_LOG_ERROR, "First chunk byte is 0x%02x instead of 0xe1\n",
bytestream2_peek_byte(&s->gb));


chunk_size = bytestream2_get_be32(&s->gb) & 0x00FFFFFF;


if (chunk_size != bytestream2_get_bytes_left(&s->gb) + 4)
av_log(s->avctx, AV_LOG_WARNING,
"MOV chunk size %d != encoded chunk size %d\n",
chunk_size,
bytestream2_get_bytes_left(&s->gb) + 4
);


total_blocks = ((s->avctx->width + 3) / 4) * ((s->avctx->height + 3) / 4);

if (total_blocks / 32 > bytestream2_get_bytes_left(&s->gb))
return AVERROR_INVALIDDATA;

if ((ret = ff_reget_buffer(s->avctx, s->frame, 0)) < 0)
return ret;
pixels = (uint16_t *)s->frame->data[0];
stride = s->frame->linesize[0] / 2;
row_inc = stride - 4;


while (bytestream2_get_bytes_left(&s->gb)) {
uint8_t opcode = bytestream2_get_byte(&s->gb); 

int n_blocks = (opcode & 0x1f) + 1; 


if ((opcode & 0x80) == 0) {
colorA = (opcode << 8) | bytestream2_get_byte(&s->gb);
opcode = 0;
if ((bytestream2_peek_byte(&s->gb) & 0x80) != 0) {



opcode = 0x20;
n_blocks = 1;
}
}

n_blocks = FFMIN(n_blocks, total_blocks);

switch (opcode & 0xe0) {


case 0x80:
while (n_blocks--) {
CHECK_BLOCK();
ADVANCE_BLOCK();
}
break;


case 0xa0:
colorA = bytestream2_get_be16(&s->gb);
while (n_blocks--) {
CHECK_BLOCK();
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++){
pixels[block_ptr] = colorA;
block_ptr++;
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0xc0:
colorA = bytestream2_get_be16(&s->gb);
case 0x20:
colorB = bytestream2_get_be16(&s->gb);


color4[0] = colorB;
color4[1] = 0;
color4[2] = 0;
color4[3] = colorA;


ta = (colorA >> 10) & 0x1F;
tb = (colorB >> 10) & 0x1F;
color4[1] |= ((11 * ta + 21 * tb) >> 5) << 10;
color4[2] |= ((21 * ta + 11 * tb) >> 5) << 10;


ta = (colorA >> 5) & 0x1F;
tb = (colorB >> 5) & 0x1F;
color4[1] |= ((11 * ta + 21 * tb) >> 5) << 5;
color4[2] |= ((21 * ta + 11 * tb) >> 5) << 5;


ta = colorA & 0x1F;
tb = colorB & 0x1F;
color4[1] |= ((11 * ta + 21 * tb) >> 5);
color4[2] |= ((21 * ta + 11 * tb) >> 5);

if (bytestream2_get_bytes_left(&s->gb) < n_blocks * 4)
return AVERROR_INVALIDDATA;
while (n_blocks--) {
CHECK_BLOCK();
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
uint8_t index = bytestream2_get_byteu(&s->gb);
for (pixel_x = 0; pixel_x < 4; pixel_x++){
uint8_t idx = (index >> (2 * (3 - pixel_x))) & 0x03;
pixels[block_ptr] = color4[idx];
block_ptr++;
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0x00:
if (bytestream2_get_bytes_left(&s->gb) < 30)
return AVERROR_INVALIDDATA;
CHECK_BLOCK();
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++){

if ((pixel_y != 0) || (pixel_x != 0))
colorA = bytestream2_get_be16u(&s->gb);
pixels[block_ptr] = colorA;
block_ptr++;
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
break;


default:
av_log(s->avctx, AV_LOG_ERROR, "Unknown opcode %d in rpza chunk."
" Skip remaining %d bytes of chunk data.\n", opcode,
bytestream2_get_bytes_left(&s->gb));
return AVERROR_INVALIDDATA;
} 
}

return 0;
}

static av_cold int rpza_decode_init(AVCodecContext *avctx)
{
RpzaContext *s = avctx->priv_data;

s->avctx = avctx;
avctx->pix_fmt = AV_PIX_FMT_RGB555;

s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);

return 0;
}

static int rpza_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
RpzaContext *s = avctx->priv_data;
int ret;

bytestream2_init(&s->gb, avpkt->data, avpkt->size);

ret = rpza_decode_stream(s);
if (ret < 0)
return ret;

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;


return avpkt->size;
}

static av_cold int rpza_decode_end(AVCodecContext *avctx)
{
RpzaContext *s = avctx->priv_data;

av_frame_free(&s->frame);

return 0;
}

AVCodec ff_rpza_decoder = {
.name = "rpza",
.long_name = NULL_IF_CONFIG_SMALL("QuickTime video (RPZA)"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_RPZA,
.priv_data_size = sizeof(RpzaContext),
.init = rpza_decode_init,
.close = rpza_decode_end,
.decode = rpza_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
