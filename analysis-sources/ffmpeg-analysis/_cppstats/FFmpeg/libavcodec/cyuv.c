#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"
typedef struct CyuvDecodeContext {
AVCodecContext *avctx;
int width, height;
} CyuvDecodeContext;
static av_cold int cyuv_decode_init(AVCodecContext *avctx)
{
CyuvDecodeContext *s = avctx->priv_data;
s->avctx = avctx;
s->width = avctx->width;
if (s->width & 0x3)
return AVERROR_INVALIDDATA;
s->height = avctx->height;
return 0;
}
static int cyuv_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
CyuvDecodeContext *s=avctx->priv_data;
AVFrame *frame = data;
unsigned char *y_plane;
unsigned char *u_plane;
unsigned char *v_plane;
int y_ptr;
int u_ptr;
int v_ptr;
const signed char *y_table = (const signed char*)buf + 0;
const signed char *u_table = (const signed char*)buf + 16;
const signed char *v_table = (const signed char*)buf + 32;
unsigned char y_pred, u_pred, v_pred;
int stream_ptr;
unsigned char cur_byte;
int pixel_groups;
int rawsize = s->height * FFALIGN(s->width,2) * 2;
int ret;
if (avctx->codec_id == AV_CODEC_ID_AURA) {
y_table = u_table;
u_table = v_table;
}
if (buf_size == 48 + s->height * (s->width * 3 / 4)) {
avctx->pix_fmt = AV_PIX_FMT_YUV411P;
} else if(buf_size == rawsize ) {
avctx->pix_fmt = AV_PIX_FMT_UYVY422;
} else {
av_log(avctx, AV_LOG_ERROR, "got a buffer with %d bytes when %d were expected\n",
buf_size, 48 + s->height * (s->width * 3 / 4));
return AVERROR_INVALIDDATA;
}
stream_ptr = 48;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;
y_plane = frame->data[0];
u_plane = frame->data[1];
v_plane = frame->data[2];
if (buf_size == rawsize) {
int linesize = FFALIGN(s->width,2) * 2;
y_plane += frame->linesize[0] * s->height;
for (stream_ptr = 0; stream_ptr < rawsize; stream_ptr += linesize) {
y_plane -= frame->linesize[0];
memcpy(y_plane, buf+stream_ptr, linesize);
}
} else {
for (y_ptr = 0, u_ptr = 0, v_ptr = 0;
y_ptr < (s->height * frame->linesize[0]);
y_ptr += frame->linesize[0] - s->width,
u_ptr += frame->linesize[1] - s->width / 4,
v_ptr += frame->linesize[2] - s->width / 4) {
cur_byte = buf[stream_ptr++];
u_plane[u_ptr++] = u_pred = cur_byte & 0xF0;
y_plane[y_ptr++] = y_pred = (cur_byte & 0x0F) << 4;
cur_byte = buf[stream_ptr++];
v_plane[v_ptr++] = v_pred = cur_byte & 0xF0;
y_pred += y_table[cur_byte & 0x0F];
y_plane[y_ptr++] = y_pred;
cur_byte = buf[stream_ptr++];
y_pred += y_table[cur_byte & 0x0F];
y_plane[y_ptr++] = y_pred;
y_pred += y_table[(cur_byte & 0xF0) >> 4];
y_plane[y_ptr++] = y_pred;
pixel_groups = s->width / 4 - 1;
while (pixel_groups--) {
cur_byte = buf[stream_ptr++];
u_pred += u_table[(cur_byte & 0xF0) >> 4];
u_plane[u_ptr++] = u_pred;
y_pred += y_table[cur_byte & 0x0F];
y_plane[y_ptr++] = y_pred;
cur_byte = buf[stream_ptr++];
v_pred += v_table[(cur_byte & 0xF0) >> 4];
v_plane[v_ptr++] = v_pred;
y_pred += y_table[cur_byte & 0x0F];
y_plane[y_ptr++] = y_pred;
cur_byte = buf[stream_ptr++];
y_pred += y_table[cur_byte & 0x0F];
y_plane[y_ptr++] = y_pred;
y_pred += y_table[(cur_byte & 0xF0) >> 4];
y_plane[y_ptr++] = y_pred;
}
}
}
*got_frame = 1;
return buf_size;
}
#if CONFIG_AURA_DECODER
AVCodec ff_aura_decoder = {
.name = "aura",
.long_name = NULL_IF_CONFIG_SMALL("Auravision AURA"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_AURA,
.priv_data_size = sizeof(CyuvDecodeContext),
.init = cyuv_decode_init,
.decode = cyuv_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
#if CONFIG_CYUV_DECODER
AVCodec ff_cyuv_decoder = {
.name = "cyuv",
.long_name = NULL_IF_CONFIG_SMALL("Creative YUV (CYUV)"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_CYUV,
.priv_data_size = sizeof(CyuvDecodeContext),
.init = cyuv_decode_init,
.decode = cyuv_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
#endif
