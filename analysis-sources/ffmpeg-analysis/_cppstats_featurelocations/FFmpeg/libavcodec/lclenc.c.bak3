






































#include <stdio.h>
#include <stdlib.h>

#include "libavutil/avassert.h"
#include "avcodec.h"
#include "internal.h"
#include "lcl.h"
#include "libavutil/internal.h"
#include "libavutil/mem.h"

#include <zlib.h>

typedef struct LclEncContext {

AVCodecContext *avctx;


int imgtype;

int compression;

int flags;
z_stream zstream;
} LclEncContext;

static int encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *p, int *got_packet)
{
LclEncContext *c = avctx->priv_data;
int i, ret;
int zret; 
int max_size = deflateBound(&c->zstream, avctx->width * avctx->height * 3);

if ((ret = ff_alloc_packet2(avctx, pkt, max_size, 0)) < 0)
return ret;

if(avctx->pix_fmt != AV_PIX_FMT_BGR24){
av_log(avctx, AV_LOG_ERROR, "Format not supported!\n");
return -1;
}

zret = deflateReset(&c->zstream);
if (zret != Z_OK) {
av_log(avctx, AV_LOG_ERROR, "Deflate reset error: %d\n", zret);
return -1;
}
c->zstream.next_out = pkt->data;
c->zstream.avail_out = pkt->size;

for(i = avctx->height - 1; i >= 0; i--) {
c->zstream.next_in = p->data[0]+p->linesize[0]*i;
c->zstream.avail_in = avctx->width*3;
zret = deflate(&c->zstream, Z_NO_FLUSH);
if (zret != Z_OK) {
av_log(avctx, AV_LOG_ERROR, "Deflate error: %d\n", zret);
return -1;
}
}
zret = deflate(&c->zstream, Z_FINISH);
if (zret != Z_STREAM_END) {
av_log(avctx, AV_LOG_ERROR, "Deflate error: %d\n", zret);
return -1;
}

pkt->size = c->zstream.total_out;
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;

return 0;
}

static av_cold int encode_init(AVCodecContext *avctx)
{
LclEncContext *c = avctx->priv_data;
int zret; 

c->avctx= avctx;

av_assert0(avctx->width && avctx->height);

avctx->extradata = av_mallocz(8 + AV_INPUT_BUFFER_PADDING_SIZE);
if (!avctx->extradata)
return AVERROR(ENOMEM);

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
avctx->coded_frame->key_frame = 1;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

c->compression = avctx->compression_level == FF_COMPRESSION_DEFAULT ?
COMP_ZLIB_NORMAL :
av_clip(avctx->compression_level, 0, 9);
c->flags = 0;
c->imgtype = IMGTYPE_RGB24;
avctx->bits_per_coded_sample= 24;

avctx->extradata[0]= 4;
avctx->extradata[1]= 0;
avctx->extradata[2]= 0;
avctx->extradata[3]= 0;
avctx->extradata[4]= c->imgtype;
avctx->extradata[5]= c->compression;
avctx->extradata[6]= c->flags;
avctx->extradata[7]= CODEC_ZLIB;
c->avctx->extradata_size= 8;

c->zstream.zalloc = Z_NULL;
c->zstream.zfree = Z_NULL;
c->zstream.opaque = Z_NULL;
zret = deflateInit(&c->zstream, c->compression);
if (zret != Z_OK) {
av_log(avctx, AV_LOG_ERROR, "Deflate init error: %d\n", zret);
return AVERROR_UNKNOWN;
}

return 0;
}

static av_cold int encode_end(AVCodecContext *avctx)
{
LclEncContext *c = avctx->priv_data;

av_freep(&avctx->extradata);
deflateEnd(&c->zstream);

return 0;
}

AVCodec ff_zlib_encoder = {
.name = "zlib",
.long_name = NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) ZLIB"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_ZLIB,
.priv_data_size = sizeof(LclEncContext),
.init = encode_init,
.encode2 = encode_frame,
.close = encode_end,
.capabilities = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_BGR24, AV_PIX_FMT_NONE },
.caps_internal = FF_CODEC_CAP_INIT_THREADSAFE |
FF_CODEC_CAP_INIT_CLEANUP,
};
