






















#if !defined(AVCODEC_V4L2_FMT_H)
#define AVCODEC_V4L2_FMT_H

#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"

enum AVPixelFormat ff_v4l2_format_v4l2_to_avfmt(uint32_t v4l2_fmt, enum AVCodecID avcodec);
uint32_t ff_v4l2_format_avcodec_to_v4l2(enum AVCodecID avcodec);
uint32_t ff_v4l2_format_avfmt_to_v4l2(enum AVPixelFormat avfmt);

#endif 
