#include "libavutil/attributes.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/vf_fspp.h"
void ff_store_slice_mmx(uint8_t *dst, int16_t *src,
ptrdiff_t dst_stride, ptrdiff_t src_stride,
ptrdiff_t width, ptrdiff_t height, ptrdiff_t log2_scale);
void ff_store_slice2_mmx(uint8_t *dst, int16_t *src,
ptrdiff_t dst_stride, ptrdiff_t src_stride,
ptrdiff_t width, ptrdiff_t height, ptrdiff_t log2_scale);
void ff_mul_thrmat_mmx(int16_t *thr_adr_noq, int16_t *thr_adr, int q);
void ff_column_fidct_mmx(int16_t *thr_adr, int16_t *data, int16_t *output, int cnt);
void ff_row_idct_mmx(int16_t *workspace, int16_t *output_adr, ptrdiff_t output_stride, int cnt);
void ff_row_fdct_mmx(int16_t *data, const uint8_t *pixels, ptrdiff_t line_size, int cnt);
av_cold void ff_fspp_init_x86(FSPPContext *s)
{
int cpu_flags = av_get_cpu_flags();
if (EXTERNAL_MMX(cpu_flags)) {
s->store_slice = ff_store_slice_mmx;
s->store_slice2 = ff_store_slice2_mmx;
s->mul_thrmat = ff_mul_thrmat_mmx;
s->column_fidct = ff_column_fidct_mmx;
s->row_idct = ff_row_idct_mmx;
s->row_fdct = ff_row_fdct_mmx;
}
}
