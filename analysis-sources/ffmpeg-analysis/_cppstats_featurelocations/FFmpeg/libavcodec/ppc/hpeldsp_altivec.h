





















#if !defined(AVCODEC_PPC_HPELDSP_ALTIVEC_H)
#define AVCODEC_PPC_HPELDSP_ALTIVEC_H

#include <stddef.h>
#include <stdint.h>

void ff_avg_pixels16_altivec(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);
void ff_put_pixels16_altivec(uint8_t *block, const uint8_t *pixels,
ptrdiff_t line_size, int h);

#endif 
