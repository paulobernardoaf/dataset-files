#include <stddef.h>
#include <stdint.h>
typedef struct SSIMDSPContext {
void (*ssim_4x4_line)(const uint8_t *buf, ptrdiff_t buf_stride,
const uint8_t *ref, ptrdiff_t ref_stride,
int (*sums)[4], int w);
double (*ssim_end_line)(const int (*sum0)[4], const int (*sum1)[4], int w);
} SSIMDSPContext;
void ff_ssim_init_x86(SSIMDSPContext *dsp);
