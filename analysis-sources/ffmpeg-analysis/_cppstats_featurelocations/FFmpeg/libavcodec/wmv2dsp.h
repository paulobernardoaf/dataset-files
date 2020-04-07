

















#if !defined(AVCODEC_WMV2DSP_H)
#define AVCODEC_WMV2DSP_H

#include <stdint.h>

#include "qpeldsp.h"

typedef struct WMV2DSPContext {
void (*idct_add)(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void (*idct_put)(uint8_t *dest, ptrdiff_t line_size, int16_t *block);

qpel_mc_func put_mspel_pixels_tab[8];

int idct_perm;
} WMV2DSPContext;

void ff_wmv2dsp_init(WMV2DSPContext *c);
void ff_wmv2dsp_init_mips(WMV2DSPContext *c);

#endif 
