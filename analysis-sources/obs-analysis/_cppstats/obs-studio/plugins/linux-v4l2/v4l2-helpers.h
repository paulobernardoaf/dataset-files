#pragma once
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <obs-module.h>
#include <media-io/video-io.h>
#if defined(__cplusplus)
extern "C" {
#endif
struct v4l2_mmap_info {
size_t length;
void *start;
};
struct v4l2_buffer_data {
uint_fast32_t count;
struct v4l2_mmap_info *info;
};
static inline enum video_format v4l2_to_obs_video_format(uint_fast32_t format)
{
switch (format) {
case V4L2_PIX_FMT_YVYU:
return VIDEO_FORMAT_YVYU;
case V4L2_PIX_FMT_YUYV:
return VIDEO_FORMAT_YUY2;
case V4L2_PIX_FMT_UYVY:
return VIDEO_FORMAT_UYVY;
case V4L2_PIX_FMT_NV12:
return VIDEO_FORMAT_NV12;
case V4L2_PIX_FMT_YUV420:
return VIDEO_FORMAT_I420;
case V4L2_PIX_FMT_YVU420:
return VIDEO_FORMAT_I420;
#if defined(V4L2_PIX_FMT_XBGR32)
case V4L2_PIX_FMT_XBGR32:
return VIDEO_FORMAT_BGRX;
#endif
#if defined(V4L2_PIX_FMT_ABGR32)
case V4L2_PIX_FMT_ABGR32:
return VIDEO_FORMAT_BGRA;
#endif
case V4L2_PIX_FMT_BGR24:
return VIDEO_FORMAT_BGR3;
default:
return VIDEO_FORMAT_NONE;
}
}
static const int v4l2_framesizes[] = {
160 << 16 | 120, 320 << 16 | 240, 480 << 16 | 320, 640 << 16 | 480,
800 << 16 | 600, 1024 << 16 | 768, 1280 << 16 | 960, 1440 << 16 | 1050,
1440 << 16 | 1080, 1600 << 16 | 1200,
640 << 16 | 360, 960 << 16 | 540, 1280 << 16 | 720, 1600 << 16 | 900,
1920 << 16 | 1080, 1920 << 16 | 1200, 2560 << 16 | 1440,
3840 << 16 | 2160,
2560 << 16 | 1080, 3440 << 16 | 1440, 5120 << 16 | 2160,
432 << 16 | 520, 480 << 16 | 320, 480 << 16 | 530, 486 << 16 | 440,
576 << 16 | 310, 576 << 16 | 520, 576 << 16 | 570, 720 << 16 | 576,
1024 << 16 | 576,
0};
static const int v4l2_framerates[] = {1 << 16 | 60,
1 << 16 | 50,
1 << 16 | 30,
1 << 16 | 25,
1 << 16 | 20,
1 << 16 | 15,
1 << 16 | 10,
1 << 16 | 5,
0};
static inline int v4l2_pack_tuple(int a, int b)
{
return (a << 16) | (b & 0xffff);
}
static void v4l2_unpack_tuple(int *a, int *b, int packed)
{
*a = packed >> 16;
*b = packed & 0xffff;
}
int_fast32_t v4l2_start_capture(int_fast32_t dev, struct v4l2_buffer_data *buf);
int_fast32_t v4l2_stop_capture(int_fast32_t dev);
int_fast32_t v4l2_create_mmap(int_fast32_t dev, struct v4l2_buffer_data *buf);
int_fast32_t v4l2_destroy_mmap(struct v4l2_buffer_data *buf);
int_fast32_t v4l2_set_input(int_fast32_t dev, int *input);
int_fast32_t v4l2_get_input_caps(int_fast32_t dev, int input, uint32_t *caps);
int_fast32_t v4l2_set_format(int_fast32_t dev, int *resolution,
int *pixelformat, int *bytesperline);
int_fast32_t v4l2_set_framerate(int_fast32_t dev, int *framerate);
int_fast32_t v4l2_set_standard(int_fast32_t dev, int *standard);
int_fast32_t v4l2_enum_dv_timing(int_fast32_t dev, struct v4l2_dv_timings *dvt,
int index);
int_fast32_t v4l2_set_dv_timing(int_fast32_t dev, int *timing);
#if defined(__cplusplus)
}
#endif
