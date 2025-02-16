

























#include <inttypes.h>

#include "avcodec.h"
#include "internal.h"
#include "mjpeg.h"
#include "mjpegdec.h"

static uint32_t read_offs(AVCodecContext *avctx, GetBitContext *gb, uint32_t size, const char *err_msg){
uint32_t offs= get_bits_long(gb, 32);
if(offs >= size){
av_log(avctx, AV_LOG_WARNING, err_msg, offs, size);
return 0;
}
return offs;
}

static int mjpegb_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
MJpegDecodeContext *s = avctx->priv_data;
const uint8_t *buf_end, *buf_ptr;
GetBitContext hgb; 
uint32_t dqt_offs, dht_offs, sof_offs, sos_offs, second_field_offs;
uint32_t field_size, sod_offs;
int ret;

buf_ptr = buf;
buf_end = buf + buf_size;
s->got_picture = 0;
s->adobe_transform = -1;

read_header:

s->restart_interval = 0;
s->restart_count = 0;
s->mjpb_skiptosod = 0;

if (buf_end - buf_ptr >= 1 << 28)
return AVERROR_INVALIDDATA;

init_get_bits(&hgb, buf_ptr, (buf_end - buf_ptr)*8);

skip_bits(&hgb, 32); 

if (get_bits_long(&hgb, 32) != MKBETAG('m','j','p','g')) {
av_log(avctx, AV_LOG_WARNING, "not mjpeg-b (bad fourcc)\n");
return AVERROR_INVALIDDATA;
}

field_size = get_bits_long(&hgb, 32); 
av_log(avctx, AV_LOG_DEBUG, "field size: 0x%"PRIx32"\n", field_size);
skip_bits(&hgb, 32); 
second_field_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "second_field_offs is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "second field offs: 0x%"PRIx32"\n",
second_field_offs);

dqt_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "dqt is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "dqt offs: 0x%"PRIx32"\n", dqt_offs);
if (dqt_offs) {
init_get_bits(&s->gb, buf_ptr+dqt_offs, (buf_end - (buf_ptr+dqt_offs))*8);
s->start_code = DQT;
ret = ff_mjpeg_decode_dqt(s);
if (ret < 0 && (avctx->err_recognition & AV_EF_EXPLODE))
return ret;
}

dht_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "dht is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "dht offs: 0x%"PRIx32"\n", dht_offs);
if (dht_offs) {
init_get_bits(&s->gb, buf_ptr+dht_offs, (buf_end - (buf_ptr+dht_offs))*8);
s->start_code = DHT;
ff_mjpeg_decode_dht(s);
}

sof_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "sof is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "sof offs: 0x%"PRIx32"\n", sof_offs);
if (sof_offs) {
init_get_bits(&s->gb, buf_ptr+sof_offs, (buf_end - (buf_ptr+sof_offs))*8);
s->start_code = SOF0;
if ((ret = ff_mjpeg_decode_sof(s)) < 0)
return ret;
}

sos_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "sos is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "sos offs: 0x%"PRIx32"\n", sos_offs);
sod_offs = read_offs(avctx, &hgb, buf_end - buf_ptr, "sof is %d and size is %d\n");
av_log(avctx, AV_LOG_DEBUG, "sod offs: 0x%"PRIx32"\n", sod_offs);
if (sos_offs) {
init_get_bits(&s->gb, buf_ptr + sos_offs,
8 * FFMIN(field_size, buf_end - buf_ptr - sos_offs));
s->mjpb_skiptosod = (sod_offs - sos_offs - show_bits(&s->gb, 16));
s->start_code = SOS;
ret = ff_mjpeg_decode_sos(s, NULL, 0, NULL);
if (ret < 0 && (avctx->err_recognition & AV_EF_EXPLODE))
return ret;
}

if (s->interlaced) {
s->bottom_field ^= 1;

if (s->bottom_field != s->interlace_polarity && second_field_offs) {
buf_ptr = buf + second_field_offs;
goto read_header;
}
}



if(!s->got_picture) {
av_log(avctx, AV_LOG_WARNING, "no picture\n");
return buf_size;
}

if ((ret = av_frame_ref(data, s->picture_ptr)) < 0)
return ret;
*got_frame = 1;

if (!s->lossless && avctx->debug & FF_DEBUG_QP) {
av_log(avctx, AV_LOG_DEBUG, "QP: %d\n",
FFMAX3(s->qscale[0], s->qscale[1], s->qscale[2]));
}

return buf_size;
}

AVCodec ff_mjpegb_decoder = {
.name = "mjpegb",
.long_name = NULL_IF_CONFIG_SMALL("Apple MJPEG-B"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_MJPEGB,
.priv_data_size = sizeof(MJpegDecodeContext),
.init = ff_mjpeg_decode_init,
.close = ff_mjpeg_decode_end,
.decode = mjpegb_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.max_lowres = 3,
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE,
};
