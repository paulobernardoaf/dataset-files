#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/asm.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/utvideodsp.h"
void ff_restore_rgb_planes_sse2(uint8_t *src_r, uint8_t *src_g, uint8_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
void ff_restore_rgb_planes_avx2(uint8_t *src_r, uint8_t *src_g, uint8_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
void ff_restore_rgb_planes10_sse2(uint16_t *src_r, uint16_t *src_g, uint16_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
void ff_restore_rgb_planes10_avx2(uint16_t *src_r, uint16_t *src_g, uint16_t *src_b,
ptrdiff_t linesize_r, ptrdiff_t linesize_g,
ptrdiff_t linesize_b, int width, int height);
av_cold void ff_utvideodsp_init_x86(UTVideoDSPContext *c)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_SSE2(cpu_flags)) {
c->restore_rgb_planes = ff_restore_rgb_planes_sse2;
c->restore_rgb_planes10 = ff_restore_rgb_planes10_sse2;
}
if (EXTERNAL_AVX2_FAST(cpu_flags)) {
c->restore_rgb_planes = ff_restore_rgb_planes_avx2;
c->restore_rgb_planes10 = ff_restore_rgb_planes10_avx2;
}
}
