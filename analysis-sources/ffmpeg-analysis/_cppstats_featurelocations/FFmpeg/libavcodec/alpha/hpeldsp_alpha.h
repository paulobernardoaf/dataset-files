

















#if !defined(AVCODEC_ALPHA_HPELDSP_ALPHA_H)
#define AVCODEC_ALPHA_HPELDSP_ALPHA_H

#include <stdint.h>
#include <stddef.h>

void put_pixels_axp_asm(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

#endif 
