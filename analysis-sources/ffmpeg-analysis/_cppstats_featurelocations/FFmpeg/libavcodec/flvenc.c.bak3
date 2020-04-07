



















#include "flv.h"
#include "h263.h"
#include "h263data.h"
#include "mpegvideo.h"
#include "mpegvideodata.h"

void ff_flv_encode_picture_header(MpegEncContext *s, int picture_number)
{
int format;

avpriv_align_put_bits(&s->pb);

put_bits(&s->pb, 17, 1);

put_bits(&s->pb, 5, (s->h263_flv - 1));
put_bits(&s->pb, 8,
(((int64_t) s->picture_number * 30 * s->avctx->time_base.num) / 
s->avctx->time_base.den) & 0xff); 
if (s->width == 352 && s->height == 288)
format = 2;
else if (s->width == 176 && s->height == 144)
format = 3;
else if (s->width == 128 && s->height == 96)
format = 4;
else if (s->width == 320 && s->height == 240)
format = 5;
else if (s->width == 160 && s->height == 120)
format = 6;
else if (s->width <= 255 && s->height <= 255)
format = 0; 
else
format = 1; 
put_bits(&s->pb, 3, format); 
if (format == 0) {
put_bits(&s->pb, 8, s->width);
put_bits(&s->pb, 8, s->height);
} else if (format == 1) {
put_bits(&s->pb, 16, s->width);
put_bits(&s->pb, 16, s->height);
}
put_bits(&s->pb, 2, s->pict_type == AV_PICTURE_TYPE_P); 
put_bits(&s->pb, 1, 1); 
put_bits(&s->pb, 5, s->qscale); 
put_bits(&s->pb, 1, 0); 

if (s->h263_aic) {
s->y_dc_scale_table =
s->c_dc_scale_table = ff_aic_dc_scale_table;
} else {
s->y_dc_scale_table =
s->c_dc_scale_table = ff_mpeg1_dc_scale_table;
}
}

void ff_flv2_encode_ac_esc(PutBitContext *pb, int slevel, int level,
int run, int last)
{
if (level < 64) { 
put_bits(pb, 1, 0);
put_bits(pb, 1, last);
put_bits(pb, 6, run);

put_sbits(pb, 7, slevel);
} else {

put_bits(pb, 1, 1);
put_bits(pb, 1, last);
put_bits(pb, 6, run);

put_sbits(pb, 11, slevel);
}
}

static const AVClass flv_class = {
.class_name = "flv encoder",
.item_name = av_default_item_name,
.option = ff_mpv_generic_options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_flv_encoder = {
.name = "flv",
.long_name = NULL_IF_CONFIG_SMALL("FLV / Sorenson Spark / Sorenson H.263 (Flash Video)"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_FLV1,
.priv_data_size = sizeof(MpegEncContext),
.init = ff_mpv_encode_init,
.encode2 = ff_mpv_encode_picture,
.close = ff_mpv_encode_end,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_YUV420P,
AV_PIX_FMT_NONE},
.priv_class = &flv_class,
};
