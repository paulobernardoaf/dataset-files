



















#include "libavutil/mips/generic_macros_msa.h"
#include "blockdsp_mips.h"

static void copy_8bit_value_width8_msa(uint8_t *src, uint8_t val,
int32_t src_stride, int32_t height)
{
int32_t cnt;
uint64_t dst0;
v16u8 val0;

val0 = (v16u8) __msa_fill_b(val);
dst0 = __msa_copy_u_d((v2i64) val0, 0);

for (cnt = (height >> 2); cnt--;) {
SD4(dst0, dst0, dst0, dst0, src, src_stride);
src += (4 * src_stride);
}
}

static void copy_8bit_value_width16_msa(uint8_t *src, uint8_t val,
int32_t src_stride, int32_t height)
{
int32_t cnt;
v16u8 val0;

val0 = (v16u8) __msa_fill_b(val);

for (cnt = (height >> 3); cnt--;) {
ST_UB8(val0, val0, val0, val0, val0, val0, val0, val0, src, src_stride);
src += (8 * src_stride);
}
}

static void memset_zero_16width_msa(uint8_t *src, int32_t stride,
int32_t height)
{
int8_t cnt;
v16u8 zero = { 0 };

for (cnt = (height / 2); cnt--;) {
ST_UB(zero, src);
src += stride;
ST_UB(zero, src);
src += stride;
}
}

void ff_fill_block16_msa(uint8_t *src, uint8_t val, ptrdiff_t stride, int height)
{
copy_8bit_value_width16_msa(src, val, stride, height);
}

void ff_fill_block8_msa(uint8_t *src, uint8_t val, ptrdiff_t stride, int height)
{
copy_8bit_value_width8_msa(src, val, stride, height);
}

void ff_clear_block_msa(int16_t *block)
{
memset_zero_16width_msa((uint8_t *) block, 16, 8);
}

void ff_clear_blocks_msa(int16_t *block)
{
memset_zero_16width_msa((uint8_t *) block, 16, 8 * 6);
}
