















#pragma once

#include "ff-clock.h"

#include <libavcodec/avcodec.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct ff_frame {
AVFrame *frame;
struct ff_clock *clock;
double pts;
int64_t duration;
};

typedef struct ff_frame ff_frame_t;

#if defined(__cplusplus)
}
#endif
