



















#if !defined(AVCODEC_LIBVPX_H)
#define AVCODEC_LIBVPX_H

#include <vpx/vpx_codec.h>

#include "avcodec.h"

void ff_vp9_init_static(AVCodec *codec);
#if 0
enum AVPixelFormat ff_vpx_imgfmt_to_pixfmt(vpx_img_fmt_t img);
vpx_img_fmt_t ff_vpx_pixfmt_to_imgfmt(enum AVPixelFormat pix);
#endif

#endif 
