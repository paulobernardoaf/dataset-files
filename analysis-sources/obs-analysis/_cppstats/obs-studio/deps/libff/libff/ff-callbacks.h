#pragma once
#include "ff-frame.h"
#include <libavcodec/avcodec.h>
#include <stdbool.h>
#if defined(__cplusplus)
extern "C" {
#endif
typedef bool (*ff_callback_frame)(struct ff_frame *frame, void *opaque);
typedef bool (*ff_callback_format)(AVCodecContext *codec_context, void *opaque);
typedef bool (*ff_callback_initialize)(void *opaque);
struct ff_callbacks {
ff_callback_frame frame;
ff_callback_format format;
ff_callback_initialize initialize;
ff_callback_frame frame_initialize;
ff_callback_frame frame_free;
void *opaque;
};
bool ff_callbacks_frame(struct ff_callbacks *callbacks, struct ff_frame *frame);
bool ff_callbacks_format(struct ff_callbacks *callbacks,
AVCodecContext *codec_context);
bool ff_callbacks_initialize(struct ff_callbacks *callbacks);
bool ff_callbacks_frame_initialize(struct ff_frame *frame,
struct ff_callbacks *callbacks);
bool ff_callbacks_frame_free(struct ff_frame *frame,
struct ff_callbacks *callbacks);
#if defined(__cplusplus)
}
#endif
