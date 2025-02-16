#pragma once
#include "../util/c99defs.h"
#include "video-io.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct video_scaler;
typedef struct video_scaler video_scaler_t;
#define VIDEO_SCALER_SUCCESS 0
#define VIDEO_SCALER_BAD_CONVERSION -1
#define VIDEO_SCALER_FAILED -2
EXPORT int video_scaler_create(video_scaler_t **scaler,
const struct video_scale_info *dst,
const struct video_scale_info *src,
enum video_scale_type type);
EXPORT void video_scaler_destroy(video_scaler_t *scaler);
EXPORT bool video_scaler_scale(video_scaler_t *scaler, uint8_t *output[],
const uint32_t out_linesize[],
const uint8_t *const input[],
const uint32_t in_linesize[]);
#if defined(__cplusplus)
}
#endif
