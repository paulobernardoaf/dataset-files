#include <stdint.h>
#include "config.h"
typedef struct BinkDSPContext {
void (*idct_put)(uint8_t *dest, int line_size, int32_t *block);
void (*idct_add)(uint8_t *dest, int line_size, int32_t *block);
void (*scale_block)(const uint8_t src[64], uint8_t *dst, int linesize);
void (*add_pixels8)(uint8_t *av_restrict pixels, int16_t *block, int line_size);
} BinkDSPContext;
void ff_binkdsp_init(BinkDSPContext *c);
