



















#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "videodsp.h"

#define BIT_DEPTH 8
#include "videodsp_template.c"
#undef BIT_DEPTH

#define BIT_DEPTH 16
#include "videodsp_template.c"
#undef BIT_DEPTH

static void just_return(uint8_t *buf, ptrdiff_t stride, int h)
{
}

av_cold void ff_videodsp_init(VideoDSPContext *ctx, int bpc)
{
ctx->prefetch = just_return;
if (bpc <= 8) {
ctx->emulated_edge_mc = ff_emulated_edge_mc_8;
} else {
ctx->emulated_edge_mc = ff_emulated_edge_mc_16;
}

if (ARCH_AARCH64)
ff_videodsp_init_aarch64(ctx, bpc);
if (ARCH_ARM)
ff_videodsp_init_arm(ctx, bpc);
if (ARCH_PPC)
ff_videodsp_init_ppc(ctx, bpc);
if (ARCH_X86)
ff_videodsp_init_x86(ctx, bpc);
if (ARCH_MIPS)
ff_videodsp_init_mips(ctx, bpc);
}
