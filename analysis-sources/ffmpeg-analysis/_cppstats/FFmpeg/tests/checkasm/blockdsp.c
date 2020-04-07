#include <string.h>
#include "checkasm.h"
#include "libavcodec/blockdsp.h"
#include "libavutil/common.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#define randomize_buffers(size) do { int i; for (i = 0; i < size; i++) { uint16_t r = rnd(); AV_WN16A(buf0 + i, r); AV_WN16A(buf1 + i, r); } } while (0)
#define check_clear(func, size) do { if (check_func(h.func, "blockdsp." #func)) { declare_func_emms(AV_CPU_FLAG_MMX, void, int16_t *block); randomize_buffers(size); call_ref(buf0); call_new(buf1); if (memcmp(buf0, buf1, sizeof(*buf0) * size)) fail(); bench_new(buf0); } } while (0)
void checkasm_check_blockdsp(void)
{
LOCAL_ALIGNED_32(uint16_t, buf0, [6 * 8 * 8]);
LOCAL_ALIGNED_32(uint16_t, buf1, [6 * 8 * 8]);
AVCodecContext avctx = { 0 };
BlockDSPContext h;
ff_blockdsp_init(&h, &avctx);
check_clear(clear_block, 8 * 8);
check_clear(clear_blocks, 8 * 8 * 6);
report("blockdsp");
}
