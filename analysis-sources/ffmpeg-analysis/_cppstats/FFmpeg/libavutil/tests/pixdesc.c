#include "libavutil/pixdesc.c"
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
av_log(NULL, AV_LOG_INFO, "pix fmt %s avg_bpp:%d colortype:%d\n", desc->name, av_get_padded_bits_per_pixel(desc), get_color_type(desc));
}
return err;
}
