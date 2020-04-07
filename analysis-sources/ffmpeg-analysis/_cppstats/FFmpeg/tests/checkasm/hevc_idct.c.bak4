



















#include <string.h>

#include "libavutil/intreadwrite.h"

#include "libavcodec/hevcdsp.h"

#include "checkasm.h"

#define randomize_buffers(buf, size) do { int j; for (j = 0; j < size; j++) { int16_t r = rnd(); AV_WN16A(buf + j, r); } } while (0)








static void check_idct(HEVCDSPContext h, int bit_depth)
{
int i;
LOCAL_ALIGNED(32, int16_t, coeffs0, [32 * 32]);
LOCAL_ALIGNED(32, int16_t, coeffs1, [32 * 32]);

for (i = 2; i <= 5; i++) {
int block_size = 1 << i;
int size = block_size * block_size;
int col_limit = block_size;
declare_func(void, int16_t *coeffs, int col_limit);

randomize_buffers(coeffs0, size);
memcpy(coeffs1, coeffs0, sizeof(*coeffs0) * size);
if (check_func(h.idct[i - 2], "hevc_idct_%dx%d_%d", block_size, block_size, bit_depth)) {
call_ref(coeffs0, col_limit);
call_new(coeffs1, col_limit);
if (memcmp(coeffs0, coeffs1, sizeof(*coeffs0) * size))
fail();
bench_new(coeffs1, col_limit);
}
}
}

static void check_idct_dc(HEVCDSPContext h, int bit_depth)
{
int i;
LOCAL_ALIGNED(32, int16_t, coeffs0, [32 * 32]);
LOCAL_ALIGNED(32, int16_t, coeffs1, [32 * 32]);

for (i = 2; i <= 5; i++) {
int block_size = 1 << i;
int size = block_size * block_size;
declare_func_emms(AV_CPU_FLAG_MMXEXT, void, int16_t *coeffs);

randomize_buffers(coeffs0, size);
memcpy(coeffs1, coeffs0, sizeof(*coeffs0) * size);

if (check_func(h.idct_dc[i - 2], "hevc_idct_%dx%d_dc_%d", block_size, block_size, bit_depth)) {
call_ref(coeffs0);
call_new(coeffs1);
if (memcmp(coeffs0, coeffs1, sizeof(*coeffs0) * size))
fail();
bench_new(coeffs1);
}
}
}

void checkasm_check_hevc_idct(void)
{
int bit_depth;

for (bit_depth = 8; bit_depth <= 12; bit_depth += 2) {
HEVCDSPContext h;

ff_hevc_dsp_init(&h, bit_depth);
check_idct_dc(h, bit_depth);
}
report("idct_dc");

for (bit_depth = 8; bit_depth <= 12; bit_depth += 2) {
HEVCDSPContext h;

ff_hevc_dsp_init(&h, bit_depth);
check_idct(h, bit_depth);
}
report("idct");
}
