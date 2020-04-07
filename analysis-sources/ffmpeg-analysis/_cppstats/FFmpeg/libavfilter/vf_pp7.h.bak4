




















#if !defined(AVFILTER_PP7_H)
#define AVFILTER_PP7_H

#include "avfilter.h"

typedef struct PP7Context {
AVClass *class;
int thres2[99][16];

int qp;
int mode;
int qscale_type;
int hsub;
int vsub;
int temp_stride;
uint8_t *src;

int (*requantize)(struct PP7Context *p, int16_t *src, int qp);
void (*dctB)(int16_t *dst, int16_t *src);

} PP7Context;

void ff_pp7_init_x86(PP7Context *pp7);

#endif 
