




















#include <stddef.h>
#include <stdint.h>

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/arm/cpu.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/idctdsp.h"
#include "idct.h"
#include "idctdsp_arm.h"

void ff_add_pixels_clamped_arm(const int16_t *block, uint8_t *dest,
ptrdiff_t line_size);



static void j_rev_dct_arm_put(uint8_t *dest, ptrdiff_t line_size,
int16_t *block)
{
ff_j_rev_dct_arm(block);
ff_put_pixels_clamped_c(block, dest, line_size);
}

static void j_rev_dct_arm_add(uint8_t *dest, ptrdiff_t line_size,
int16_t *block)
{
ff_j_rev_dct_arm(block);
ff_add_pixels_clamped_arm(block, dest, line_size);
}

static void simple_idct_arm_put(uint8_t *dest, ptrdiff_t line_size,
int16_t *block)
{
ff_simple_idct_arm(block);
ff_put_pixels_clamped_c(block, dest, line_size);
}

static void simple_idct_arm_add(uint8_t *dest, ptrdiff_t line_size,
int16_t *block)
{
ff_simple_idct_arm(block);
ff_add_pixels_clamped_arm(block, dest, line_size);
}

av_cold void ff_idctdsp_init_arm(IDCTDSPContext *c, AVCodecContext *avctx,
unsigned high_bit_depth)
{
int cpu_flags = av_get_cpu_flags();

if (!avctx->lowres && !high_bit_depth) {
if ((avctx->idct_algo == FF_IDCT_AUTO && !(avctx->flags & AV_CODEC_FLAG_BITEXACT)) ||
avctx->idct_algo == FF_IDCT_ARM) {
c->idct_put = j_rev_dct_arm_put;
c->idct_add = j_rev_dct_arm_add;
c->idct = ff_j_rev_dct_arm;
c->perm_type = FF_IDCT_PERM_LIBMPEG2;
} else if (avctx->idct_algo == FF_IDCT_SIMPLEARM) {
c->idct_put = simple_idct_arm_put;
c->idct_add = simple_idct_arm_add;
c->idct = ff_simple_idct_arm;
c->perm_type = FF_IDCT_PERM_NONE;
}
}

c->add_pixels_clamped = ff_add_pixels_clamped_arm;

if (have_armv5te(cpu_flags))
ff_idctdsp_init_armv5te(c, avctx, high_bit_depth);
if (have_armv6(cpu_flags))
ff_idctdsp_init_armv6(c, avctx, high_bit_depth);
if (have_neon(cpu_flags))
ff_idctdsp_init_neon(c, avctx, high_bit_depth);
}
