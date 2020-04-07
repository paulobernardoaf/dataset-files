#include <string.h>
#include "checkasm.h"
#include "libavcodec/pixblockdsp.h"
#include "libavutil/common.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#define BUF_UNITS 8
#define BUF_SIZE (BUF_UNITS * 128 + 8 * BUF_UNITS)
#define randomize_buffers() do { int i; for (i = 0; i < BUF_SIZE; i += 4) { uint32_t r = rnd(); AV_WN32A(src10 + i, r); AV_WN32A(src11 + i, r); r = rnd(); AV_WN32A(src20 + i, r); AV_WN32A(src21 + i, r); r = rnd(); AV_WN32A(dst0_ + i, r); AV_WN32A(dst1_ + i, r); } } while (0)
#define check_get_pixels(type) do { int i; declare_func_emms(AV_CPU_FLAG_MMX, void, int16_t *block, const uint8_t *pixels, ptrdiff_t line_size); for (i = 0; i < BUF_UNITS; i++) { int src_offset = i * 64 * sizeof(type) + 8 * i; int dst_offset = i * 64; randomize_buffers(); call_ref(dst0 + dst_offset, src10 + src_offset, 8); call_new(dst1 + dst_offset, src11 + src_offset, 8); if (memcmp(src10, src11, BUF_SIZE)|| memcmp(dst0, dst1, BUF_SIZE)) fail(); bench_new(dst1 + dst_offset, src11 + src_offset, 8); } } while (0)
#define check_diff_pixels(type) do { int i; declare_func_emms(AV_CPU_FLAG_MMX, void, int16_t *av_restrict block, const uint8_t *s1, const uint8_t *s2, ptrdiff_t stride); for (i = 0; i < BUF_UNITS; i++) { int src_offset = i * 64 * sizeof(type) + 8 * i; int dst_offset = i * 64; randomize_buffers(); call_ref(dst0 + dst_offset, src10 + src_offset, src20 + src_offset, 8); call_new(dst1 + dst_offset, src11 + src_offset, src21 + src_offset, 8); if (memcmp(src10, src11, BUF_SIZE) || memcmp(src20, src21, BUF_SIZE) || memcmp(dst0, dst1, BUF_SIZE)) fail(); bench_new(dst1 + dst_offset, src11 + src_offset, src21 + src_offset, 8); } } while (0)
void checkasm_check_pixblockdsp(void)
{
LOCAL_ALIGNED_16(uint8_t, src10, [BUF_SIZE]);
LOCAL_ALIGNED_16(uint8_t, src11, [BUF_SIZE]);
LOCAL_ALIGNED_16(uint8_t, src20, [BUF_SIZE]);
LOCAL_ALIGNED_16(uint8_t, src21, [BUF_SIZE]);
LOCAL_ALIGNED_16(uint8_t, dst0_, [BUF_SIZE]);
LOCAL_ALIGNED_16(uint8_t, dst1_, [BUF_SIZE]);
uint16_t *dst0 = (uint16_t *)dst0_;
uint16_t *dst1 = (uint16_t *)dst1_;
PixblockDSPContext h;
AVCodecContext avctx = {
.bits_per_raw_sample = 8,
};
ff_pixblockdsp_init(&h, &avctx);
if (check_func(h.get_pixels, "get_pixels"))
check_get_pixels(uint8_t);
report("get_pixels");
if (check_func(h.diff_pixels, "diff_pixels"))
check_diff_pixels(uint8_t);
report("diff_pixels");
}
