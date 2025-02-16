

















#if !defined(AVDEVICE_V4L2_COMMON_H)
#define AVDEVICE_V4L2_COMMON_H

#undef __STRICT_ANSI__ 
#include "config.h"
#include "libavformat/internal.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#if HAVE_SYS_VIDEOIO_H
#include <sys/videoio.h>
#else
#if HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif
#include <linux/videodev2.h>
#endif
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "avdevice.h"
#include "timefilter.h"
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/time.h"
#include "libavutil/avstring.h"

struct fmt_map {
enum AVPixelFormat ff_fmt;
enum AVCodecID codec_id;
uint32_t v4l2_fmt;
};

extern const struct fmt_map ff_fmt_conversion_table[];

uint32_t ff_fmt_ff2v4l(enum AVPixelFormat pix_fmt, enum AVCodecID codec_id);
enum AVPixelFormat ff_fmt_v4l2ff(uint32_t v4l2_fmt, enum AVCodecID codec_id);
enum AVCodecID ff_fmt_v4l2codec(uint32_t v4l2_fmt);

#endif 
