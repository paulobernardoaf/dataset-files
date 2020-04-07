




















#if !defined(AVCODEC_PNGDSP_H)
#define AVCODEC_PNGDSP_H

#include <stdint.h>

typedef struct PNGDSPContext {
void (*add_bytes_l2)(uint8_t *dst,
uint8_t *src1 ,
uint8_t *src2, int w);


void (*add_paeth_prediction)(uint8_t *dst, uint8_t *src,
uint8_t *top, int w, int bpp);
} PNGDSPContext;

void ff_pngdsp_init(PNGDSPContext *dsp);
void ff_pngdsp_init_x86(PNGDSPContext *dsp);

#endif 
