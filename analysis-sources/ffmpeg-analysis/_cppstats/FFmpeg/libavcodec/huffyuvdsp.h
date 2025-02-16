#include <stdint.h>
#include "libavutil/pixfmt.h"
#include "config.h"
#if HAVE_BIGENDIAN
#define B 3
#define G 2
#define R 1
#define A 0
#else
#define B 0
#define G 1
#define R 2
#define A 3
#endif
typedef struct HuffYUVDSPContext {
void (*add_int16)(uint16_t *dst, const uint16_t *src,
unsigned mask, int w);
void (*add_hfyu_median_pred_int16)(uint16_t *dst, const uint16_t *top,
const uint16_t *diff, unsigned mask,
int w, int *left, int *left_top);
void (*add_hfyu_left_pred_bgr32)(uint8_t *dst, const uint8_t *src,
intptr_t w, uint8_t *left);
} HuffYUVDSPContext;
void ff_huffyuvdsp_init(HuffYUVDSPContext *c, enum AVPixelFormat pix_fmt);
void ff_huffyuvdsp_init_x86(HuffYUVDSPContext *c, enum AVPixelFormat pix_fmt);
