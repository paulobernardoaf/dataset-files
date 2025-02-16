#include "../util/bmem.h"
#include "video-scaler.h"
#include <libswscale/swscale.h>
struct video_scaler {
struct SwsContext *swscale;
int src_height;
};
static inline enum AVPixelFormat
get_ffmpeg_video_format(enum video_format format)
{
switch (format) {
case VIDEO_FORMAT_I420:
return AV_PIX_FMT_YUV420P;
case VIDEO_FORMAT_NV12:
return AV_PIX_FMT_NV12;
case VIDEO_FORMAT_YUY2:
return AV_PIX_FMT_YUYV422;
case VIDEO_FORMAT_UYVY:
return AV_PIX_FMT_UYVY422;
case VIDEO_FORMAT_RGBA:
return AV_PIX_FMT_RGBA;
case VIDEO_FORMAT_BGRA:
return AV_PIX_FMT_BGRA;
case VIDEO_FORMAT_BGRX:
return AV_PIX_FMT_BGRA;
case VIDEO_FORMAT_Y800:
return AV_PIX_FMT_GRAY8;
case VIDEO_FORMAT_I444:
return AV_PIX_FMT_YUV444P;
case VIDEO_FORMAT_BGR3:
return AV_PIX_FMT_BGR24;
case VIDEO_FORMAT_I422:
return AV_PIX_FMT_YUV422P;
case VIDEO_FORMAT_I40A:
return AV_PIX_FMT_YUVA420P;
case VIDEO_FORMAT_I42A:
return AV_PIX_FMT_YUVA422P;
case VIDEO_FORMAT_YUVA:
return AV_PIX_FMT_YUVA444P;
case VIDEO_FORMAT_NONE:
case VIDEO_FORMAT_YVYU:
case VIDEO_FORMAT_AYUV:
return AV_PIX_FMT_NONE;
}
return AV_PIX_FMT_NONE;
}
static inline int get_ffmpeg_scale_type(enum video_scale_type type)
{
switch (type) {
case VIDEO_SCALE_DEFAULT:
return SWS_FAST_BILINEAR;
case VIDEO_SCALE_POINT:
return SWS_POINT;
case VIDEO_SCALE_FAST_BILINEAR:
return SWS_FAST_BILINEAR;
case VIDEO_SCALE_BILINEAR:
return SWS_BILINEAR | SWS_AREA;
case VIDEO_SCALE_BICUBIC:
return SWS_BICUBIC;
}
return SWS_POINT;
}
static inline const int *get_ffmpeg_coeffs(enum video_colorspace cs)
{
switch (cs) {
case VIDEO_CS_DEFAULT:
return sws_getCoefficients(SWS_CS_ITU601);
case VIDEO_CS_601:
return sws_getCoefficients(SWS_CS_ITU601);
case VIDEO_CS_709:
return sws_getCoefficients(SWS_CS_ITU709);
default:
return sws_getCoefficients(SWS_CS_ITU601);
}
}
static inline int get_ffmpeg_range_type(enum video_range_type type)
{
switch (type) {
case VIDEO_RANGE_DEFAULT:
return 0;
case VIDEO_RANGE_PARTIAL:
return 0;
case VIDEO_RANGE_FULL:
return 1;
}
return 0;
}
#define FIXED_1_0 (1 << 16)
int video_scaler_create(video_scaler_t **scaler_out,
const struct video_scale_info *dst,
const struct video_scale_info *src,
enum video_scale_type type)
{
enum AVPixelFormat format_src = get_ffmpeg_video_format(src->format);
enum AVPixelFormat format_dst = get_ffmpeg_video_format(dst->format);
int scale_type = get_ffmpeg_scale_type(type);
const int *coeff_src = get_ffmpeg_coeffs(src->colorspace);
const int *coeff_dst = get_ffmpeg_coeffs(dst->colorspace);
int range_src = get_ffmpeg_range_type(src->range);
int range_dst = get_ffmpeg_range_type(dst->range);
struct video_scaler *scaler;
int ret;
if (!scaler_out)
return VIDEO_SCALER_FAILED;
if (format_src == AV_PIX_FMT_NONE || format_dst == AV_PIX_FMT_NONE)
return VIDEO_SCALER_BAD_CONVERSION;
scaler = bzalloc(sizeof(struct video_scaler));
scaler->src_height = src->height;
scaler->swscale = sws_getCachedContext(NULL, src->width, src->height,
format_src, dst->width,
dst->height, format_dst,
scale_type, NULL, NULL, NULL);
if (!scaler->swscale) {
blog(LOG_ERROR, "video_scaler_create: Could not create "
"swscale");
goto fail;
}
ret = sws_setColorspaceDetails(scaler->swscale, coeff_src, range_src,
coeff_dst, range_dst, 0, FIXED_1_0,
FIXED_1_0);
if (ret < 0) {
blog(LOG_DEBUG, "video_scaler_create: "
"sws_setColorspaceDetails failed, ignoring");
}
*scaler_out = scaler;
return VIDEO_SCALER_SUCCESS;
fail:
video_scaler_destroy(scaler);
return VIDEO_SCALER_FAILED;
}
void video_scaler_destroy(video_scaler_t *scaler)
{
if (scaler) {
sws_freeContext(scaler->swscale);
bfree(scaler);
}
}
bool video_scaler_scale(video_scaler_t *scaler, uint8_t *output[],
const uint32_t out_linesize[],
const uint8_t *const input[],
const uint32_t in_linesize[])
{
if (!scaler)
return false;
int ret = sws_scale(scaler->swscale, input, (const int *)in_linesize, 0,
scaler->src_height, output,
(const int *)out_linesize);
if (ret <= 0) {
blog(LOG_ERROR, "video_scaler_scale: sws_scale failed: %d",
ret);
return false;
}
return true;
}
