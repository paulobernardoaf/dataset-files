#include <stdint.h>
typedef struct HQDSPContext {
void (*idct_put)(uint8_t *dst, int stride, int16_t *block);
} HQDSPContext;
void ff_hqdsp_init(HQDSPContext *c);
