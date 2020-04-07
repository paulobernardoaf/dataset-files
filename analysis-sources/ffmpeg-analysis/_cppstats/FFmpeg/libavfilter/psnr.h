#include <stddef.h>
#include <stdint.h>
typedef struct PSNRDSPContext {
uint64_t (*sse_line)(const uint8_t *buf, const uint8_t *ref, int w);
} PSNRDSPContext;
void ff_psnr_init_x86(PSNRDSPContext *dsp, int bpp);
