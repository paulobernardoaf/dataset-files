

























#include <stdio.h>
#include <stdlib.h>

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "libavutil/common.h"

#define KMVC_KEYFRAME 0x80
#define KMVC_PALETTE 0x40
#define KMVC_METHOD 0x0F
#define MAX_PALSIZE 256




typedef struct KmvcContext {
AVCodecContext *avctx;

int setpal;
int palsize;
uint32_t pal[MAX_PALSIZE];
uint8_t *cur, *prev;
uint8_t frm0[320 * 200], frm1[320 * 200];
GetByteContext g;
} KmvcContext;

typedef struct BitBuf {
int bits;
int bitbuf;
} BitBuf;

#define BLK(data, x, y) data[av_clip((x) + (y) * 320, 0, 320 * 200 -1)]

#define kmvc_init_getbits(bb, g) bb.bits = 7; bb.bitbuf = bytestream2_get_byte(g);

#define kmvc_getbit(bb, g, res) {res = 0; if (bb.bitbuf & (1 << bb.bits)) res = 1; bb.bits--; if(bb.bits == -1) { bb.bitbuf = bytestream2_get_byte(g); bb.bits = 7; } }









static int kmvc_decode_intra_8x8(KmvcContext * ctx, int w, int h)
{
BitBuf bb;
int res, val;
int i, j;
int bx, by;
int l0x, l1x, l0y, l1y;
int mx, my;

kmvc_init_getbits(bb, &ctx->g);

for (by = 0; by < h; by += 8)
for (bx = 0; bx < w; bx += 8) {
if (!bytestream2_get_bytes_left(&ctx->g)) {
av_log(ctx->avctx, AV_LOG_ERROR, "Data overrun\n");
return AVERROR_INVALIDDATA;
}
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
val = bytestream2_get_byte(&ctx->g);
for (i = 0; i < 64; i++)
BLK(ctx->cur, bx + (i & 0x7), by + (i >> 3)) = val;
} else { 
for (i = 0; i < 4; i++) {
l0x = bx + (i & 1) * 4;
l0y = by + (i & 2) * 2;
kmvc_getbit(bb, &ctx->g, res);
if (!res) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
val = bytestream2_get_byte(&ctx->g);
for (j = 0; j < 16; j++)
BLK(ctx->cur, l0x + (j & 3), l0y + (j >> 2)) = val;
} else { 
val = bytestream2_get_byte(&ctx->g);
mx = val & 0xF;
my = val >> 4;
if ((l0x-mx) + 320*(l0y-my) < 0 || (l0x-mx) + 320*(l0y-my) > 320*197 - 4) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid MV\n");
return AVERROR_INVALIDDATA;
}
for (j = 0; j < 16; j++)
BLK(ctx->cur, l0x + (j & 3), l0y + (j >> 2)) =
BLK(ctx->cur, l0x + (j & 3) - mx, l0y + (j >> 2) - my);
}
} else { 
for (j = 0; j < 4; j++) {
l1x = l0x + (j & 1) * 2;
l1y = l0y + (j & 2);
kmvc_getbit(bb, &ctx->g, res);
if (!res) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
val = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x, l1y) = val;
BLK(ctx->cur, l1x + 1, l1y) = val;
BLK(ctx->cur, l1x, l1y + 1) = val;
BLK(ctx->cur, l1x + 1, l1y + 1) = val;
} else { 
val = bytestream2_get_byte(&ctx->g);
mx = val & 0xF;
my = val >> 4;
if ((l1x-mx) + 320*(l1y-my) < 0 || (l1x-mx) + 320*(l1y-my) > 320*199 - 2) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid MV\n");
return AVERROR_INVALIDDATA;
}
BLK(ctx->cur, l1x, l1y) = BLK(ctx->cur, l1x - mx, l1y - my);
BLK(ctx->cur, l1x + 1, l1y) =
BLK(ctx->cur, l1x + 1 - mx, l1y - my);
BLK(ctx->cur, l1x, l1y + 1) =
BLK(ctx->cur, l1x - mx, l1y + 1 - my);
BLK(ctx->cur, l1x + 1, l1y + 1) =
BLK(ctx->cur, l1x + 1 - mx, l1y + 1 - my);
}
} else { 
BLK(ctx->cur, l1x, l1y) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x + 1, l1y) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x, l1y + 1) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x + 1, l1y + 1) = bytestream2_get_byte(&ctx->g);
}
}
}
}
}
}

return 0;
}

static int kmvc_decode_inter_8x8(KmvcContext * ctx, int w, int h)
{
BitBuf bb;
int res, val;
int i, j;
int bx, by;
int l0x, l1x, l0y, l1y;
int mx, my;

kmvc_init_getbits(bb, &ctx->g);

for (by = 0; by < h; by += 8)
for (bx = 0; bx < w; bx += 8) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
if (!bytestream2_get_bytes_left(&ctx->g)) {
av_log(ctx->avctx, AV_LOG_ERROR, "Data overrun\n");
return AVERROR_INVALIDDATA;
}
val = bytestream2_get_byte(&ctx->g);
for (i = 0; i < 64; i++)
BLK(ctx->cur, bx + (i & 0x7), by + (i >> 3)) = val;
} else { 
for (i = 0; i < 64; i++)
BLK(ctx->cur, bx + (i & 0x7), by + (i >> 3)) =
BLK(ctx->prev, bx + (i & 0x7), by + (i >> 3));
}
} else { 
if (!bytestream2_get_bytes_left(&ctx->g)) {
av_log(ctx->avctx, AV_LOG_ERROR, "Data overrun\n");
return AVERROR_INVALIDDATA;
}
for (i = 0; i < 4; i++) {
l0x = bx + (i & 1) * 4;
l0y = by + (i & 2) * 2;
kmvc_getbit(bb, &ctx->g, res);
if (!res) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
val = bytestream2_get_byte(&ctx->g);
for (j = 0; j < 16; j++)
BLK(ctx->cur, l0x + (j & 3), l0y + (j >> 2)) = val;
} else { 
val = bytestream2_get_byte(&ctx->g);
mx = (val & 0xF) - 8;
my = (val >> 4) - 8;
if ((l0x+mx) + 320*(l0y+my) < 0 || (l0x+mx) + 320*(l0y+my) > 320*197 - 4) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid MV\n");
return AVERROR_INVALIDDATA;
}
for (j = 0; j < 16; j++)
BLK(ctx->cur, l0x + (j & 3), l0y + (j >> 2)) =
BLK(ctx->prev, l0x + (j & 3) + mx, l0y + (j >> 2) + my);
}
} else { 
for (j = 0; j < 4; j++) {
l1x = l0x + (j & 1) * 2;
l1y = l0y + (j & 2);
kmvc_getbit(bb, &ctx->g, res);
if (!res) {
kmvc_getbit(bb, &ctx->g, res);
if (!res) { 
val = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x, l1y) = val;
BLK(ctx->cur, l1x + 1, l1y) = val;
BLK(ctx->cur, l1x, l1y + 1) = val;
BLK(ctx->cur, l1x + 1, l1y + 1) = val;
} else { 
val = bytestream2_get_byte(&ctx->g);
mx = (val & 0xF) - 8;
my = (val >> 4) - 8;
if ((l1x+mx) + 320*(l1y+my) < 0 || (l1x+mx) + 320*(l1y+my) > 320*199 - 2) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid MV\n");
return AVERROR_INVALIDDATA;
}
BLK(ctx->cur, l1x, l1y) = BLK(ctx->prev, l1x + mx, l1y + my);
BLK(ctx->cur, l1x + 1, l1y) =
BLK(ctx->prev, l1x + 1 + mx, l1y + my);
BLK(ctx->cur, l1x, l1y + 1) =
BLK(ctx->prev, l1x + mx, l1y + 1 + my);
BLK(ctx->cur, l1x + 1, l1y + 1) =
BLK(ctx->prev, l1x + 1 + mx, l1y + 1 + my);
}
} else { 
BLK(ctx->cur, l1x, l1y) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x + 1, l1y) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x, l1y + 1) = bytestream2_get_byte(&ctx->g);
BLK(ctx->cur, l1x + 1, l1y + 1) = bytestream2_get_byte(&ctx->g);
}
}
}
}
}
}

return 0;
}

static int decode_frame(AVCodecContext * avctx, void *data, int *got_frame,
AVPacket *avpkt)
{
KmvcContext *const ctx = avctx->priv_data;
AVFrame *frame = data;
uint8_t *out, *src;
int i, ret;
int header;
int blocksize;
int pal_size;
const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, &pal_size);

bytestream2_init(&ctx->g, avpkt->data, avpkt->size);

if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

header = bytestream2_get_byte(&ctx->g);


if (bytestream2_peek_byte(&ctx->g) == 127) {
bytestream2_skip(&ctx->g, 3);
for (i = 0; i < 127; i++) {
ctx->pal[i + (header & 0x81)] = 0xFFU << 24 | bytestream2_get_be24(&ctx->g);
bytestream2_skip(&ctx->g, 1);
}
bytestream2_seek(&ctx->g, -127 * 4 - 3, SEEK_CUR);
}

if (header & KMVC_KEYFRAME) {
frame->key_frame = 1;
frame->pict_type = AV_PICTURE_TYPE_I;
} else {
frame->key_frame = 0;
frame->pict_type = AV_PICTURE_TYPE_P;
}

if (header & KMVC_PALETTE) {
frame->palette_has_changed = 1;

for (i = 1; i <= ctx->palsize; i++) {
ctx->pal[i] = 0xFFU << 24 | bytestream2_get_be24(&ctx->g);
}
}

if (pal && pal_size == AVPALETTE_SIZE) {
frame->palette_has_changed = 1;
memcpy(ctx->pal, pal, AVPALETTE_SIZE);
} else if (pal) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", pal_size);
}

if (ctx->setpal) {
ctx->setpal = 0;
frame->palette_has_changed = 1;
}


memcpy(frame->data[1], ctx->pal, 1024);

blocksize = bytestream2_get_byte(&ctx->g);

if (blocksize != 8 && blocksize != 127) {
av_log(avctx, AV_LOG_ERROR, "Block size = %i\n", blocksize);
return AVERROR_INVALIDDATA;
}
memset(ctx->cur, 0, 320 * 200);
switch (header & KMVC_METHOD) {
case 0:
case 1: 
memcpy(ctx->cur, ctx->prev, 320 * 200);
break;
case 3:
kmvc_decode_intra_8x8(ctx, avctx->width, avctx->height);
break;
case 4:
kmvc_decode_inter_8x8(ctx, avctx->width, avctx->height);
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unknown compression method %i\n", header & KMVC_METHOD);
return AVERROR_INVALIDDATA;
}

out = frame->data[0];
src = ctx->cur;
for (i = 0; i < avctx->height; i++) {
memcpy(out, src, avctx->width);
src += 320;
out += frame->linesize[0];
}


if (ctx->cur == ctx->frm0) {
ctx->cur = ctx->frm1;
ctx->prev = ctx->frm0;
} else {
ctx->cur = ctx->frm0;
ctx->prev = ctx->frm1;
}

*got_frame = 1;


return avpkt->size;
}






static av_cold int decode_init(AVCodecContext * avctx)
{
KmvcContext *const c = avctx->priv_data;
int i;

c->avctx = avctx;

if (avctx->width > 320 || avctx->height > 200) {
av_log(avctx, AV_LOG_ERROR, "KMVC supports frames <= 320x200\n");
return AVERROR(EINVAL);
}

c->cur = c->frm0;
c->prev = c->frm1;

for (i = 0; i < 256; i++) {
c->pal[i] = 0xFFU << 24 | i * 0x10101;
}

if (avctx->extradata_size < 12) {
av_log(avctx, AV_LOG_WARNING,
"Extradata missing, decoding may not work properly...\n");
c->palsize = 127;
} else {
c->palsize = AV_RL16(avctx->extradata + 10);
if (c->palsize >= (unsigned)MAX_PALSIZE) {
c->palsize = 127;
av_log(avctx, AV_LOG_ERROR, "KMVC palette too large\n");
return AVERROR_INVALIDDATA;
}
}

if (avctx->extradata_size == 1036) { 
uint8_t *src = avctx->extradata + 12;
for (i = 0; i < 256; i++) {
c->pal[i] = AV_RL32(src);
src += 4;
}
c->setpal = 1;
}

avctx->pix_fmt = AV_PIX_FMT_PAL8;

return 0;
}

AVCodec ff_kmvc_decoder = {
.name = "kmvc",
.long_name = NULL_IF_CONFIG_SMALL("Karl Morton's video codec"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_KMVC,
.priv_data_size = sizeof(KmvcContext),
.init = decode_init,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
