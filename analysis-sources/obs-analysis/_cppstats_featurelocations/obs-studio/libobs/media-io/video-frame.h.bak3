
















#pragma once

#include "../util/bmem.h"
#include "video-io.h"

struct video_frame {
uint8_t *data[MAX_AV_PLANES];
uint32_t linesize[MAX_AV_PLANES];
};

EXPORT void video_frame_init(struct video_frame *frame,
enum video_format format, uint32_t width,
uint32_t height);

static inline void video_frame_free(struct video_frame *frame)
{
if (frame) {
bfree(frame->data[0]);
memset(frame, 0, sizeof(struct video_frame));
}
}

static inline struct video_frame *
video_frame_create(enum video_format format, uint32_t width, uint32_t height)
{
struct video_frame *frame;

frame = (struct video_frame *)bzalloc(sizeof(struct video_frame));
video_frame_init(frame, format, width, height);
return frame;
}

static inline void video_frame_destroy(struct video_frame *frame)
{
if (frame) {
bfree(frame->data[0]);
bfree(frame);
}
}

EXPORT void video_frame_copy(struct video_frame *dst,
const struct video_frame *src,
enum video_format format, uint32_t height);
