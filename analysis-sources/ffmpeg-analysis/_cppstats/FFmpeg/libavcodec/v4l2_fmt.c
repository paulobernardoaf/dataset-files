#include <linux/videodev2.h>
#include <search.h>
#include "v4l2_fmt.h"
#define V4L2_FMT(x) V4L2_PIX_FMT_##x
#define AV_CODEC(x) AV_CODEC_ID_##x
#define AV_FMT(x) AV_PIX_FMT_##x
static const struct fmt_conversion {
enum AVPixelFormat avfmt;
enum AVCodecID avcodec;
uint32_t v4l2_fmt;
} fmt_map[] = {
{ AV_FMT(RGB555LE), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB555) },
{ AV_FMT(RGB555BE), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB555X) },
{ AV_FMT(RGB565LE), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB565) },
{ AV_FMT(RGB565BE), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB565X) },
{ AV_FMT(BGR24), AV_CODEC(RAWVIDEO), V4L2_FMT(BGR24) },
{ AV_FMT(RGB24), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB24) },
{ AV_FMT(BGR0), AV_CODEC(RAWVIDEO), V4L2_FMT(BGR32) },
{ AV_FMT(0RGB), AV_CODEC(RAWVIDEO), V4L2_FMT(RGB32) },
{ AV_FMT(GRAY8), AV_CODEC(RAWVIDEO), V4L2_FMT(GREY) },
{ AV_FMT(YUV420P), AV_CODEC(RAWVIDEO), V4L2_FMT(YUV420) },
{ AV_FMT(YUYV422), AV_CODEC(RAWVIDEO), V4L2_FMT(YUYV) },
{ AV_FMT(UYVY422), AV_CODEC(RAWVIDEO), V4L2_FMT(UYVY) },
{ AV_FMT(YUV422P), AV_CODEC(RAWVIDEO), V4L2_FMT(YUV422P) },
{ AV_FMT(YUV411P), AV_CODEC(RAWVIDEO), V4L2_FMT(YUV411P) },
{ AV_FMT(YUV410P), AV_CODEC(RAWVIDEO), V4L2_FMT(YUV410) },
{ AV_FMT(YUV410P), AV_CODEC(RAWVIDEO), V4L2_FMT(YVU410) },
{ AV_FMT(NV12), AV_CODEC(RAWVIDEO), V4L2_FMT(NV12) },
{ AV_FMT(NONE), AV_CODEC(MJPEG), V4L2_FMT(MJPEG) },
{ AV_FMT(NONE), AV_CODEC(MJPEG), V4L2_FMT(JPEG) },
#if defined(V4L2_PIX_FMT_SRGGB8)
{ AV_FMT(BAYER_BGGR8), AV_CODEC(RAWVIDEO), V4L2_FMT(SBGGR8) },
{ AV_FMT(BAYER_GBRG8), AV_CODEC(RAWVIDEO), V4L2_FMT(SGBRG8) },
{ AV_FMT(BAYER_GRBG8), AV_CODEC(RAWVIDEO), V4L2_FMT(SGRBG8) },
{ AV_FMT(BAYER_RGGB8), AV_CODEC(RAWVIDEO), V4L2_FMT(SRGGB8) },
#endif
#if defined(V4L2_PIX_FMT_Y16)
{ AV_FMT(GRAY16LE), AV_CODEC(RAWVIDEO), V4L2_FMT(Y16) },
#endif
#if defined(V4L2_PIX_FMT_NV12M)
{ AV_FMT(NV12), AV_CODEC(RAWVIDEO), V4L2_FMT(NV12M) },
#endif
#if defined(V4L2_PIX_FMT_NV21M)
{ AV_FMT(NV21), AV_CODEC(RAWVIDEO), V4L2_FMT(NV21M) },
#endif
#if defined(V4L2_PIX_FMT_YUV420M)
{ AV_FMT(YUV420P), AV_CODEC(RAWVIDEO), V4L2_FMT(YUV420M) },
#endif
#if defined(V4L2_PIX_FMT_NV16M)
{ AV_FMT(NV16), AV_CODEC(RAWVIDEO), V4L2_FMT(NV16M) },
#endif
#if defined(V4L2_PIX_FMT_H263)
{ AV_FMT(NONE), AV_CODEC(H263), V4L2_FMT(H263) },
#endif
#if defined(V4L2_PIX_FMT_H264)
{ AV_FMT(NONE), AV_CODEC(H264), V4L2_FMT(H264) },
#endif
#if defined(V4L2_PIX_FMT_MPEG4)
{ AV_FMT(NONE), AV_CODEC(MPEG4), V4L2_FMT(MPEG4) },
#endif
#if defined(V4L2_PIX_FMT_CPIA1)
{ AV_FMT(NONE), AV_CODEC(CPIA), V4L2_FMT(CPIA1) },
#endif
#if defined(V4L2_PIX_FMT_DV)
{ AV_FMT(NONE), AV_CODEC(DVVIDEO), V4L2_FMT(DV) },
#endif
#if defined(V4L2_PIX_FMT_MPEG1)
{ AV_FMT(NONE), AV_CODEC(MPEG1VIDEO), V4L2_FMT(MPEG1) },
#endif
#if defined(V4L2_PIX_FMT_MPEG2)
{ AV_FMT(NONE), AV_CODEC(MPEG2VIDEO), V4L2_FMT(MPEG2) },
#endif
#if defined(V4L2_PIX_FMT_VP8)
{ AV_FMT(NONE), AV_CODEC(VP8), V4L2_FMT(VP8) },
#endif
#if defined(V4L2_PIX_FMT_VP9)
{ AV_FMT(NONE), AV_CODEC(VP9), V4L2_FMT(VP9) },
#endif
#if defined(V4L2_PIX_FMT_HEVC)
{ AV_FMT(NONE), AV_CODEC(HEVC), V4L2_FMT(HEVC) },
#endif
#if defined(V4L2_PIX_FMT_VC1_ANNEX_G)
{ AV_FMT(NONE), AV_CODEC(VC1), V4L2_FMT(VC1_ANNEX_G) },
#endif
};
uint32_t ff_v4l2_format_avcodec_to_v4l2(enum AVCodecID avcodec)
{
int i;
for (i = 0; i < FF_ARRAY_ELEMS(fmt_map); i++) {
if (fmt_map[i].avcodec == avcodec)
return fmt_map[i].v4l2_fmt;
}
return 0;
}
uint32_t ff_v4l2_format_avfmt_to_v4l2(enum AVPixelFormat avfmt)
{
int i;
for (i = 0; i < FF_ARRAY_ELEMS(fmt_map); i++) {
if (fmt_map[i].avfmt == avfmt)
return fmt_map[i].v4l2_fmt;
}
return 0;
}
enum AVPixelFormat ff_v4l2_format_v4l2_to_avfmt(uint32_t v4l2_fmt, enum AVCodecID avcodec)
{
int i;
for (i = 0; i < FF_ARRAY_ELEMS(fmt_map); i++) {
if (fmt_map[i].avcodec == avcodec &&
fmt_map[i].v4l2_fmt == v4l2_fmt)
return fmt_map[i].avfmt;
}
return AV_PIX_FMT_NONE;
}
