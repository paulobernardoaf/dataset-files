#include <stdint.h>
#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/internal.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/blockdsp.h"
#include "libavcodec/version.h"
void ff_clear_block_mmx(int16_t *block);
void ff_clear_block_sse(int16_t *block);
void ff_clear_block_avx(int16_t *block);
void ff_clear_blocks_mmx(int16_t *blocks);
void ff_clear_blocks_sse(int16_t *blocks);
void ff_clear_blocks_avx(int16_t *blocks);
av_cold void ff_blockdsp_init_x86(BlockDSPContext *c,
AVCodecContext *avctx)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_MMX(cpu_flags)) {
c->clear_block = ff_clear_block_mmx;
c->clear_blocks = ff_clear_blocks_mmx;
}
if (CONFIG_XVMC && avctx->hwaccel && avctx->hwaccel->decode_mb)
return;
if (EXTERNAL_SSE(cpu_flags)) {
c->clear_block = ff_clear_block_sse;
c->clear_blocks = ff_clear_blocks_sse;
}
if (EXTERNAL_AVX_FAST(cpu_flags)) {
c->clear_block = ff_clear_block_avx;
c->clear_blocks = ff_clear_blocks_avx;
}
#endif 
}
