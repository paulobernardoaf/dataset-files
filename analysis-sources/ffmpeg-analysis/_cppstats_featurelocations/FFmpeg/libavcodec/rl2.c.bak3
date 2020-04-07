



























#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"
#include "avcodec.h"
#include "internal.h"


#define EXTRADATA1_SIZE (6 + 256 * 3) 

typedef struct Rl2Context {
AVCodecContext *avctx;

uint16_t video_base; 
uint32_t clr_count; 
uint8_t *back_frame; 
uint32_t palette[AVPALETTE_COUNT];
} Rl2Context;










static void rl2_rle_decode(Rl2Context *s, const uint8_t *in, int size,
uint8_t *out, int stride, int video_base)
{
int base_x = video_base % s->avctx->width;
int base_y = video_base / s->avctx->width;
int stride_adj = stride - s->avctx->width;
int i;
const uint8_t *back_frame = s->back_frame;
const uint8_t *in_end = in + size;
const uint8_t *out_end = out + stride * s->avctx->height;
uint8_t *line_end;


for (i = 0; i <= base_y; i++) {
if (s->back_frame)
memcpy(out, back_frame, s->avctx->width);
out += stride;
back_frame += s->avctx->width;
}
back_frame += base_x - s->avctx->width;
line_end = out - stride_adj;
out += base_x - stride;


while (in < in_end) {
uint8_t val = *in++;
int len = 1;
if (val >= 0x80) {
if (in >= in_end)
break;
len = *in++;
if (!len)
break;
}

if (len >= out_end - out)
break;

if (s->back_frame)
val |= 0x80;
else
val &= ~0x80;

while (len--) {
*out++ = (val == 0x80) ? *back_frame : val;
back_frame++;
if (out == line_end) {
out += stride_adj;
line_end += stride;
if (len >= out_end - out)
break;
}
}
}


if (s->back_frame) {
while (out < out_end) {
memcpy(out, back_frame, line_end - out);
back_frame += line_end - out;
out = line_end + stride_adj;
line_end += stride;
}
}
}







static av_cold int rl2_decode_init(AVCodecContext *avctx)
{
Rl2Context *s = avctx->priv_data;
int back_size;
int i;
int ret;

s->avctx = avctx;
avctx->pix_fmt = AV_PIX_FMT_PAL8;

ret = ff_set_dimensions(avctx, 320, 200);
if (ret < 0)
return ret;


if (!avctx->extradata || avctx->extradata_size < EXTRADATA1_SIZE) {
av_log(avctx, AV_LOG_ERROR, "invalid extradata size\n");
return AVERROR(EINVAL);
}


s->video_base = AV_RL16(&avctx->extradata[0]);
s->clr_count = AV_RL32(&avctx->extradata[2]);

if (s->video_base >= avctx->width * avctx->height) {
av_log(avctx, AV_LOG_ERROR, "invalid video_base\n");
return AVERROR_INVALIDDATA;
}


for (i = 0; i < AVPALETTE_COUNT; i++)
s->palette[i] = 0xFFU << 24 | AV_RB24(&avctx->extradata[6 + i * 3]);


back_size = avctx->extradata_size - EXTRADATA1_SIZE;

if (back_size > 0) {
uint8_t *back_frame = av_mallocz(avctx->width*avctx->height);
if (!back_frame)
return AVERROR(ENOMEM);
rl2_rle_decode(s, avctx->extradata + EXTRADATA1_SIZE, back_size,
back_frame, avctx->width, 0);
s->back_frame = back_frame;
}
return 0;
}


static int rl2_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
AVFrame *frame = data;
const uint8_t *buf = avpkt->data;
int ret, buf_size = avpkt->size;
Rl2Context *s = avctx->priv_data;

if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;


rl2_rle_decode(s, buf, buf_size, frame->data[0], frame->linesize[0],
s->video_base);


memcpy(frame->data[1], s->palette, AVPALETTE_SIZE);

*got_frame = 1;


return buf_size;
}







static av_cold int rl2_decode_end(AVCodecContext *avctx)
{
Rl2Context *s = avctx->priv_data;

av_freep(&s->back_frame);

return 0;
}


AVCodec ff_rl2_decoder = {
.name = "rl2",
.long_name = NULL_IF_CONFIG_SMALL("RL2 video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_RL2,
.priv_data_size = sizeof(Rl2Context),
.init = rl2_decode_init,
.close = rl2_decode_end,
.decode = rl2_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
