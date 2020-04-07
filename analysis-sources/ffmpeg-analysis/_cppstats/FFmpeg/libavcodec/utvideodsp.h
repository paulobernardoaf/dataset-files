#include <stdint.h>
#include <stddef.h>
#include "libavutil/pixfmt.h"
#include "config.h"
typedef struct UTVideoDSPContext {
void (*restore_rgb_planes)(uint8_t *src_r, uint8_t *src_g, uint8_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
void (*restore_rgb_planes10)(uint16_t *src_r, uint16_t *src_g, uint16_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
} UTVideoDSPContext;
void ff_utvideodsp_init(UTVideoDSPContext *c);
void ff_utvideodsp_init_x86(UTVideoDSPContext *c);
