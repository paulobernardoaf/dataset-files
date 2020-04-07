#include "libavformat/avformat.h"
av_warn_unused_result
int ff_alloc_input_device_context(struct AVFormatContext **avctx, struct AVInputFormat *iformat,
const char *format);
