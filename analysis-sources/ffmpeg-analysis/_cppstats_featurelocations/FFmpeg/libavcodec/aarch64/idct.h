

















#if !defined(AVCODEC_AARCH64_IDCT_H)
#define AVCODEC_AARCH64_IDCT_H

#include <stdint.h>

void ff_simple_idct_neon(int16_t *data);
void ff_simple_idct_put_neon(uint8_t *dest, ptrdiff_t line_size, int16_t *data);
void ff_simple_idct_add_neon(uint8_t *dest, ptrdiff_t line_size, int16_t *data);

#endif 
