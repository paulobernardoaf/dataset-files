#include <stddef.h>
#include <stdint.h>
void ff_faanidct(int16_t block[64]);
void ff_faanidct_add(uint8_t *dest, ptrdiff_t line_size, int16_t block[64]);
void ff_faanidct_put(uint8_t *dest, ptrdiff_t line_size, int16_t block[64]);
