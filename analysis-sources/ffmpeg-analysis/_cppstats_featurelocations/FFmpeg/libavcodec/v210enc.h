

















#if !defined(AVCODEC_V210ENC_H)
#define AVCODEC_V210ENC_H

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/pixfmt.h"

typedef struct V210EncContext {
void (*pack_line_8)(const uint8_t *y, const uint8_t *u,
const uint8_t *v, uint8_t *dst, ptrdiff_t width);
void (*pack_line_10)(const uint16_t *y, const uint16_t *u,
const uint16_t *v, uint8_t *dst, ptrdiff_t width);
int sample_factor_8;
int sample_factor_10;
} V210EncContext;

void ff_v210enc_init(V210EncContext *s);

void ff_v210enc_init_x86(V210EncContext *s);

#endif 
