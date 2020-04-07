#include <stddef.h>
#include <stdint.h>
void ff_simple_idct_put_int16_8bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_add_int16_8bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_int16_8bit(int16_t *block);
void ff_simple_idct_put_int16_10bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_add_int16_10bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_int16_10bit(int16_t *block);
void ff_simple_idct_put_int32_10bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_add_int32_10bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_int32_10bit(int16_t *block);
void ff_simple_idct_put_int16_12bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_add_int16_12bit(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct_int16_12bit(int16_t *block);
void ff_prores_idct_10(int16_t *block, const int16_t *qmat);
void ff_prores_idct_12(int16_t *block, const int16_t *qmat);
void ff_simple_idct248_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct84_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct48_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
void ff_simple_idct44_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block);
