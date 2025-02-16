#include "libavutil/cpu.h"
#include "libavcodec/v210dec.h"
extern void ff_v210_planar_unpack_unaligned_ssse3(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
extern void ff_v210_planar_unpack_unaligned_avx(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
extern void ff_v210_planar_unpack_unaligned_avx2(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
extern void ff_v210_planar_unpack_aligned_ssse3(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
extern void ff_v210_planar_unpack_aligned_avx(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
extern void ff_v210_planar_unpack_aligned_avx2(const uint32_t *src, uint16_t *y, uint16_t *u, uint16_t *v, int width);
av_cold void ff_v210_x86_init(V210DecContext *s)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();
if (s->aligned_input) {
if (cpu_flags & AV_CPU_FLAG_SSSE3)
s->unpack_frame = ff_v210_planar_unpack_aligned_ssse3;
if (HAVE_AVX_EXTERNAL && cpu_flags & AV_CPU_FLAG_AVX)
s->unpack_frame = ff_v210_planar_unpack_aligned_avx;
if (HAVE_AVX2_EXTERNAL && cpu_flags & AV_CPU_FLAG_AVX2)
s->unpack_frame = ff_v210_planar_unpack_aligned_avx2;
}
else {
if (cpu_flags & AV_CPU_FLAG_SSSE3)
s->unpack_frame = ff_v210_planar_unpack_unaligned_ssse3;
if (HAVE_AVX_EXTERNAL && cpu_flags & AV_CPU_FLAG_AVX)
s->unpack_frame = ff_v210_planar_unpack_unaligned_avx;
if (HAVE_AVX2_EXTERNAL && cpu_flags & AV_CPU_FLAG_AVX2)
s->unpack_frame = ff_v210_planar_unpack_unaligned_avx2;
}
#endif
}
