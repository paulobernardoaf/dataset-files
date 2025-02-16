#include "mpegvideo.h"
#include "mpegvideodata.h"
#include "h263.h"
#include "h263data.h"
#include "put_bits.h"
#include "rv10.h"
void ff_rv20_encode_picture_header(MpegEncContext *s, int picture_number){
put_bits(&s->pb, 2, s->pict_type); 
put_bits(&s->pb, 1, 0); 
put_bits(&s->pb, 5, s->qscale);
put_sbits(&s->pb, 8, picture_number); 
s->mb_x= s->mb_y= 0;
ff_h263_encode_mba(s);
put_bits(&s->pb, 1, s->no_rounding);
av_assert0(s->f_code == 1);
av_assert0(s->unrestricted_mv == 0);
av_assert0(s->alt_inter_vlc == 0);
av_assert0(s->umvplus == 0);
av_assert0(s->modified_quant==1);
av_assert0(s->loop_filter==1);
s->h263_aic= s->pict_type == AV_PICTURE_TYPE_I;
if(s->h263_aic){
s->y_dc_scale_table=
s->c_dc_scale_table= ff_aic_dc_scale_table;
}else{
s->y_dc_scale_table=
s->c_dc_scale_table= ff_mpeg1_dc_scale_table;
}
}
static const AVClass rv20_class = {
.class_name = "rv20 encoder",
.item_name = av_default_item_name,
.option = ff_mpv_generic_options,
.version = LIBAVUTIL_VERSION_INT,
};
AVCodec ff_rv20_encoder = {
.name = "rv20",
.long_name = NULL_IF_CONFIG_SMALL("RealVideo 2.0"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_RV20,
.priv_data_size = sizeof(MpegEncContext),
.init = ff_mpv_encode_init,
.encode2 = ff_mpv_encode_picture,
.close = ff_mpv_encode_end,
.pix_fmts = (const enum AVPixelFormat[]){ AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE },
.priv_class = &rv20_class,
};
