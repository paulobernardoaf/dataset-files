

















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavcodec/idctdsp.h"
#include "libavcodec/xvididct.h"

#include "idctdsp.h"
#include "xvididct.h"

#if ARCH_X86_32 && HAVE_X86ASM
static void xvid_idct_mmx_put(uint8_t *dest, ptrdiff_t line_size, short *block)
{
ff_xvid_idct_mmx(block);
ff_put_pixels_clamped_mmx(block, dest, line_size);
}

static void xvid_idct_mmx_add(uint8_t *dest, ptrdiff_t line_size, short *block)
{
ff_xvid_idct_mmx(block);
ff_add_pixels_clamped_mmx(block, dest, line_size);
}

static void xvid_idct_mmxext_put(uint8_t *dest, ptrdiff_t line_size, short *block)
{
ff_xvid_idct_mmxext(block);
ff_put_pixels_clamped_mmx(block, dest, line_size);
}

static void xvid_idct_mmxext_add(uint8_t *dest, ptrdiff_t line_size, short *block)
{
ff_xvid_idct_mmxext(block);
ff_add_pixels_clamped_mmx(block, dest, line_size);
}
#endif

av_cold void ff_xvid_idct_init_x86(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();

if (high_bit_depth ||
!(avctx->idct_algo == FF_IDCT_AUTO ||
avctx->idct_algo == FF_IDCT_XVID))
return;

#if ARCH_X86_32
if (EXTERNAL_MMX(cpu_flags)) {
c->idct_put = xvid_idct_mmx_put;
c->idct_add = xvid_idct_mmx_add;
c->idct = ff_xvid_idct_mmx;
c->perm_type = FF_IDCT_PERM_NONE;
}

if (EXTERNAL_MMXEXT(cpu_flags)) {
c->idct_put = xvid_idct_mmxext_put;
c->idct_add = xvid_idct_mmxext_add;
c->idct = ff_xvid_idct_mmxext;
c->perm_type = FF_IDCT_PERM_NONE;
}
#endif

if (EXTERNAL_SSE2(cpu_flags)) {
c->idct_put = ff_xvid_idct_put_sse2;
c->idct_add = ff_xvid_idct_add_sse2;
c->idct = ff_xvid_idct_sse2;
c->perm_type = FF_IDCT_PERM_SSE2;
}
#endif 
}
