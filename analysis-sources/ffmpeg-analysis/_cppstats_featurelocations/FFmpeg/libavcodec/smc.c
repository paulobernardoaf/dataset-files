





























#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

#define CPAIR 2
#define CQUAD 4
#define COCTET 8

#define COLORS_PER_TABLE 256

typedef struct SmcContext {

AVCodecContext *avctx;
AVFrame *frame;

GetByteContext gb;


unsigned char color_pairs[COLORS_PER_TABLE * CPAIR];
unsigned char color_quads[COLORS_PER_TABLE * CQUAD];
unsigned char color_octets[COLORS_PER_TABLE * COCTET];

uint32_t pal[256];
} SmcContext;

#define GET_BLOCK_COUNT() (opcode & 0x10) ? (1 + bytestream2_get_byte(&s->gb)) : 1 + (opcode & 0x0F);


#define ADVANCE_BLOCK() { pixel_ptr += 4; if (pixel_ptr >= width) { pixel_ptr = 0; row_ptr += stride * 4; } total_blocks--; if (total_blocks < !!n_blocks) { av_log(s->avctx, AV_LOG_INFO, "warning: block counter just went negative (this should not happen)\n"); return; } }















static void smc_decode_stream(SmcContext *s)
{
int width = s->avctx->width;
int height = s->avctx->height;
int stride = s->frame->linesize[0];
int i;
int chunk_size;
int buf_size = bytestream2_size(&s->gb);
unsigned char opcode;
int n_blocks;
unsigned int color_flags;
unsigned int color_flags_a;
unsigned int color_flags_b;
unsigned int flag_mask;

unsigned char * const pixels = s->frame->data[0];

int image_size = height * s->frame->linesize[0];
int row_ptr = 0;
int pixel_ptr = 0;
int pixel_x, pixel_y;
int row_inc = stride - 4;
int block_ptr;
int prev_block_ptr;
int prev_block_ptr1, prev_block_ptr2;
int prev_block_flag;
int total_blocks;
int color_table_index; 
int pixel;

int color_pair_index = 0;
int color_quad_index = 0;
int color_octet_index = 0;


memcpy(s->frame->data[1], s->pal, AVPALETTE_SIZE);

bytestream2_skip(&s->gb, 1);
chunk_size = bytestream2_get_be24(&s->gb);
if (chunk_size != buf_size)
av_log(s->avctx, AV_LOG_INFO, "warning: MOV chunk size != encoded chunk size (%d != %d); using MOV chunk size\n",
chunk_size, buf_size);

chunk_size = buf_size;
total_blocks = ((s->avctx->width + 3) / 4) * ((s->avctx->height + 3) / 4);


while (total_blocks) {


if (row_ptr >= image_size) {
av_log(s->avctx, AV_LOG_INFO, "SMC decoder just went out of bounds (row ptr = %d, height = %d)\n",
row_ptr, image_size);
return;
}
if (bytestream2_get_bytes_left(&s->gb) < 1) {
av_log(s->avctx, AV_LOG_ERROR, "input too small\n");
return;
}

opcode = bytestream2_get_byte(&s->gb);
switch (opcode & 0xF0) {

case 0x00:
case 0x10:
n_blocks = GET_BLOCK_COUNT();
while (n_blocks--) {
ADVANCE_BLOCK();
}
break;


case 0x20:
case 0x30:
n_blocks = GET_BLOCK_COUNT();


if ((row_ptr == 0) && (pixel_ptr == 0)) {
av_log(s->avctx, AV_LOG_INFO, "encountered repeat block opcode (%02X) but no blocks rendered yet\n",
opcode & 0xF0);
return;
}


if (pixel_ptr == 0)
prev_block_ptr1 =
(row_ptr - s->avctx->width * 4) + s->avctx->width - 4;
else
prev_block_ptr1 = row_ptr + pixel_ptr - 4;

while (n_blocks--) {
block_ptr = row_ptr + pixel_ptr;
prev_block_ptr = prev_block_ptr1;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixels[block_ptr++] = pixels[prev_block_ptr++];
}
block_ptr += row_inc;
prev_block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0x40:
case 0x50:
n_blocks = GET_BLOCK_COUNT();
n_blocks *= 2;


if ((row_ptr == 0) && (pixel_ptr < 2 * 4)) {
av_log(s->avctx, AV_LOG_INFO, "encountered repeat block opcode (%02X) but not enough blocks rendered yet\n",
opcode & 0xF0);
return;
}


if (pixel_ptr == 0)
prev_block_ptr1 = (row_ptr - s->avctx->width * 4) +
s->avctx->width - 4 * 2;
else if (pixel_ptr == 4)
prev_block_ptr1 = (row_ptr - s->avctx->width * 4) + row_inc;
else
prev_block_ptr1 = row_ptr + pixel_ptr - 4 * 2;

if (pixel_ptr == 0)
prev_block_ptr2 = (row_ptr - s->avctx->width * 4) + row_inc;
else
prev_block_ptr2 = row_ptr + pixel_ptr - 4;

prev_block_flag = 0;
while (n_blocks--) {
block_ptr = row_ptr + pixel_ptr;
if (prev_block_flag)
prev_block_ptr = prev_block_ptr2;
else
prev_block_ptr = prev_block_ptr1;
prev_block_flag = !prev_block_flag;

for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixels[block_ptr++] = pixels[prev_block_ptr++];
}
block_ptr += row_inc;
prev_block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0x60:
case 0x70:
n_blocks = GET_BLOCK_COUNT();
pixel = bytestream2_get_byte(&s->gb);

while (n_blocks--) {
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixels[block_ptr++] = pixel;
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0x80:
case 0x90:
n_blocks = (opcode & 0x0F) + 1;


if ((opcode & 0xF0) == 0x80) {


for (i = 0; i < CPAIR; i++) {
pixel = bytestream2_get_byte(&s->gb);
color_table_index = CPAIR * color_pair_index + i;
s->color_pairs[color_table_index] = pixel;
}

color_table_index = CPAIR * color_pair_index;
color_pair_index++;

if (color_pair_index == COLORS_PER_TABLE)
color_pair_index = 0;
} else
color_table_index = CPAIR * bytestream2_get_byte(&s->gb);

while (n_blocks--) {
color_flags = bytestream2_get_be16(&s->gb);
flag_mask = 0x8000;
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
if (color_flags & flag_mask)
pixel = color_table_index + 1;
else
pixel = color_table_index;
flag_mask >>= 1;
pixels[block_ptr++] = s->color_pairs[pixel];
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0xA0:
case 0xB0:
n_blocks = (opcode & 0x0F) + 1;


if ((opcode & 0xF0) == 0xA0) {


for (i = 0; i < CQUAD; i++) {
pixel = bytestream2_get_byte(&s->gb);
color_table_index = CQUAD * color_quad_index + i;
s->color_quads[color_table_index] = pixel;
}

color_table_index = CQUAD * color_quad_index;
color_quad_index++;

if (color_quad_index == COLORS_PER_TABLE)
color_quad_index = 0;
} else
color_table_index = CQUAD * bytestream2_get_byte(&s->gb);

while (n_blocks--) {
color_flags = bytestream2_get_be32(&s->gb);

flag_mask = 30;
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixel = color_table_index +
((color_flags >> flag_mask) & 0x03);
flag_mask -= 2;
pixels[block_ptr++] = s->color_quads[pixel];
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0xC0:
case 0xD0:
n_blocks = (opcode & 0x0F) + 1;


if ((opcode & 0xF0) == 0xC0) {


for (i = 0; i < COCTET; i++) {
pixel = bytestream2_get_byte(&s->gb);
color_table_index = COCTET * color_octet_index + i;
s->color_octets[color_table_index] = pixel;
}

color_table_index = COCTET * color_octet_index;
color_octet_index++;

if (color_octet_index == COLORS_PER_TABLE)
color_octet_index = 0;
} else
color_table_index = COCTET * bytestream2_get_byte(&s->gb);

while (n_blocks--) {







int val1 = bytestream2_get_be16(&s->gb);
int val2 = bytestream2_get_be16(&s->gb);
int val3 = bytestream2_get_be16(&s->gb);
color_flags_a = ((val1 & 0xFFF0) << 8) | (val2 >> 4);
color_flags_b = ((val3 & 0xFFF0) << 8) |
((val1 & 0x0F) << 8) | ((val2 & 0x0F) << 4) | (val3 & 0x0F);

color_flags = color_flags_a;

flag_mask = 21;
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {

if (pixel_y == 2) {
color_flags = color_flags_b;
flag_mask = 21;
}
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixel = color_table_index +
((color_flags >> flag_mask) & 0x07);
flag_mask -= 3;
pixels[block_ptr++] = s->color_octets[pixel];
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;


case 0xE0:
n_blocks = (opcode & 0x0F) + 1;

while (n_blocks--) {
block_ptr = row_ptr + pixel_ptr;
for (pixel_y = 0; pixel_y < 4; pixel_y++) {
for (pixel_x = 0; pixel_x < 4; pixel_x++) {
pixels[block_ptr++] = bytestream2_get_byte(&s->gb);
}
block_ptr += row_inc;
}
ADVANCE_BLOCK();
}
break;

case 0xF0:
avpriv_request_sample(s->avctx, "0xF0 opcode");
break;
}
}

return;
}

static av_cold int smc_decode_init(AVCodecContext *avctx)
{
SmcContext *s = avctx->priv_data;

s->avctx = avctx;
avctx->pix_fmt = AV_PIX_FMT_PAL8;

s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);

return 0;
}

static int smc_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
SmcContext *s = avctx->priv_data;
int pal_size;
const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, &pal_size);
int ret;
int total_blocks = ((s->avctx->width + 3) / 4) * ((s->avctx->height + 3) / 4);

if (total_blocks / 1024 > avpkt->size)
return AVERROR_INVALIDDATA;

bytestream2_init(&s->gb, buf, buf_size);

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

if (pal && pal_size == AVPALETTE_SIZE) {
s->frame->palette_has_changed = 1;
memcpy(s->pal, pal, AVPALETTE_SIZE);
} else if (pal) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", pal_size);
}

smc_decode_stream(s);

*got_frame = 1;
if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;


return buf_size;
}

static av_cold int smc_decode_end(AVCodecContext *avctx)
{
SmcContext *s = avctx->priv_data;

av_frame_free(&s->frame);

return 0;
}

AVCodec ff_smc_decoder = {
.name = "smc",
.long_name = NULL_IF_CONFIG_SMALL("QuickTime Graphics (SMC)"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_SMC,
.priv_data_size = sizeof(SmcContext),
.init = smc_decode_init,
.close = smc_decode_end,
.decode = smc_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
