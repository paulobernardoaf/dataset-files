#include <stddef.h>
#include <stdint.h>
void ff_mss34_gen_quant_mat(uint16_t *qmat, int quality, int luma);
void ff_mss34_dct_put(uint8_t *dst, ptrdiff_t stride, int *block);
