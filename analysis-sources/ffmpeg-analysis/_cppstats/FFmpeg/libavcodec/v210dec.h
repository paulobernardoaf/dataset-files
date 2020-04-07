#include "libavutil/log.h"
#include "libavutil/opt.h"
typedef struct {
AVClass *av_class;
int custom_stride;
int aligned_input;
int thread_count;
int stride_warning_shown;
void (*unpack_frame)(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
} V210DecContext;
void ff_v210dec_init(V210DecContext *s);
void ff_v210_x86_init(V210DecContext *s);
