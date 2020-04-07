#include "libavcodec/imgconvert.c"
#if FF_API_AVPICTURE
FF_DISABLE_DEPRECATION_WARNINGS
int main(void){
int i;
int err=0;
int skip = 0;
for (i=0; i<AV_PIX_FMT_NB*2; i++) {
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(i);
if(!desc || !desc->name) {
skip ++;
continue;
}
if (skip) {
av_log(NULL, AV_LOG_INFO, "%3d unused pixel format values\n", skip);
skip = 0;
}
av_log(NULL, AV_LOG_INFO, "pix fmt %s yuv_plan:%d avg_bpp:%d\n", desc->name, is_yuv_planar(desc), av_get_padded_bits_per_pixel(desc));
}
return err;
}
FF_ENABLE_DEPRECATION_WARNINGS
#endif 
