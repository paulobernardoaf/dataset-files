

















#if !defined(AVCODEC_MPEGVIDEODSP_H)
#define AVCODEC_MPEGVIDEODSP_H

#include <stdint.h>

void ff_gmc_c(uint8_t *dst, uint8_t *src, int stride, int h, int ox, int oy,
int dxx, int dxy, int dyx, int dyy, int shift, int r,
int width, int height);

typedef struct MpegVideoDSPContext {



void (*gmc1)(uint8_t *dst , uint8_t *src ,
int srcStride, int h, int x16, int y16, int rounder);



void (*gmc)(uint8_t *dst , uint8_t *src ,
int stride, int h, int ox, int oy,
int dxx, int dxy, int dyx, int dyy,
int shift, int r, int width, int height);
} MpegVideoDSPContext;

void ff_mpegvideodsp_init(MpegVideoDSPContext *c);
void ff_mpegvideodsp_init_ppc(MpegVideoDSPContext *c);
void ff_mpegvideodsp_init_x86(MpegVideoDSPContext *c);

#endif 
