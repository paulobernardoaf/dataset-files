#include "mpegvideo.h"
#include "put_bits.h"
#include "rv10.h"
int ff_rv10_encode_picture_header(MpegEncContext *s, int picture_number)
{
int full_frame= 0;
avpriv_align_put_bits(&s->pb);
put_bits(&s->pb, 1, 1); 
put_bits(&s->pb, 1, (s->pict_type == AV_PICTURE_TYPE_P));
put_bits(&s->pb, 1, 0); 
put_bits(&s->pb, 5, s->qscale);
if (s->pict_type == AV_PICTURE_TYPE_I) {
}
if(!full_frame){
if (s->mb_width * s->mb_height >= (1U << 12)) {
avpriv_report_missing_feature(s->avctx, "Encoding frames with %d (>= 4096) macroblocks",
s->mb_width * s->mb_height);
return AVERROR(ENOSYS);
}
put_bits(&s->pb, 6, 0); 
put_bits(&s->pb, 6, 0); 
put_bits(&s->pb, 12, s->mb_width * s->mb_height);
}
put_bits(&s->pb, 3, 0); 
return 0;
}
static const AVClass rv10_class = {
.class_name = "rv10 encoder",
.item_name = av_default_item_name,
.option = ff_mpv_generic_options,
.version = LIBAVUTIL_VERSION_INT,
};
AVCodec ff_rv10_encoder = {
.name = "rv10",
.long_name = NULL_IF_CONFIG_SMALL("RealVideo 1.0"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_RV10,
.priv_data_size = sizeof(MpegEncContext),
.init = ff_mpv_encode_init,
.encode2 = ff_mpv_encode_picture,
.close = ff_mpv_encode_end,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
.priv_class = &rv10_class,
};
