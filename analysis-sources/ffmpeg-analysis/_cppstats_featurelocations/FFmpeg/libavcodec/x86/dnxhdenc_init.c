






















#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/dnxhdenc.h"

void ff_get_pixels_8x4_sym_sse2(int16_t *block, const uint8_t *pixels,
ptrdiff_t line_size);

av_cold void ff_dnxhdenc_init_x86(DNXHDEncContext *ctx)
{
if (EXTERNAL_SSE2(av_get_cpu_flags())) {
if (ctx->cid_table->bit_depth == 8)
ctx->get_pixels_8x4_sym = ff_get_pixels_8x4_sym_sse2;
}
}
