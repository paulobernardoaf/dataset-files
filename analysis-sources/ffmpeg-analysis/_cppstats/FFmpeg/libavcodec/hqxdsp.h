#include <stddef.h>
#include <stdint.h>
typedef struct HQXDSPContext {
void (*idct_put)(uint16_t *dst, ptrdiff_t stride,
int16_t *block, const uint8_t *quant);
} HQXDSPContext;
void ff_hqxdsp_init(HQXDSPContext *c);
