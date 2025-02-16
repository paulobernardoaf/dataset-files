#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/convolution.h"
void ff_filter_3x3_sse4(uint8_t *dst, int width,
float rdiv, float bias, const int *const matrix,
const uint8_t *c[], int peak, int radius,
int dstride, int stride);
av_cold void ff_convolution_init_x86(ConvolutionContext *s)
{
#if ARCH_X86_64
int i;
int cpu_flags = av_get_cpu_flags();
for (i = 0; i < 4; i++) {
if (s->mode[i] == MATRIX_SQUARE) {
if (s->matrix_length[i] == 9) {
if (EXTERNAL_SSE4(cpu_flags))
s->filter[i] = ff_filter_3x3_sse4;
}
}
}
#endif
}
