



















#include "config.h"
#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/mem.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/gradfun.h"

void ff_gradfun_filter_line_mmxext(intptr_t x, uint8_t *dst, const uint8_t *src,
const uint16_t *dc, int thresh,
const uint16_t *dithers);
void ff_gradfun_filter_line_ssse3(intptr_t x, uint8_t *dst, const uint8_t *src,
const uint16_t *dc, int thresh,
const uint16_t *dithers);

void ff_gradfun_blur_line_movdqa_sse2(intptr_t x, uint16_t *buf,
const uint16_t *buf1, uint16_t *dc,
const uint8_t *src1, const uint8_t *src2);
void ff_gradfun_blur_line_movdqu_sse2(intptr_t x, uint16_t *buf,
const uint16_t *buf1, uint16_t *dc,
const uint8_t *src1, const uint8_t *src2);

#if HAVE_X86ASM
static void gradfun_filter_line_mmxext(uint8_t *dst, const uint8_t *src,
const uint16_t *dc,
int width, int thresh,
const uint16_t *dithers)
{
intptr_t x;
if (width & 3) {
x = width & ~3;
ff_gradfun_filter_line_c(dst + x, src + x, dc + x / 2,
width - x, thresh, dithers);
width = x;
}
x = -width;
ff_gradfun_filter_line_mmxext(x, dst + width, src + width, dc + width / 2,
thresh, dithers);
}

static void gradfun_filter_line_ssse3(uint8_t *dst, const uint8_t *src, const uint16_t *dc,
int width, int thresh,
const uint16_t *dithers)
{
intptr_t x;
if (width & 7) {

x = width & ~7;
ff_gradfun_filter_line_c(dst + x, src + x, dc + x / 2,
width - x, thresh, dithers);
width = x;
}
x = -width;
ff_gradfun_filter_line_ssse3(x, dst + width, src + width, dc + width / 2,
thresh, dithers);
}

static void gradfun_blur_line_sse2(uint16_t *dc, uint16_t *buf, const uint16_t *buf1,
const uint8_t *src, int src_linesize, int width)
{
intptr_t x = -2 * width;
if (((intptr_t) src | src_linesize) & 15)
ff_gradfun_blur_line_movdqu_sse2(x, buf + width, buf1 + width,
dc + width, src + width * 2,
src + width * 2 + src_linesize);
else
ff_gradfun_blur_line_movdqa_sse2(x, buf + width, buf1 + width,
dc + width, src + width * 2,
src + width * 2 + src_linesize);
}
#endif 

av_cold void ff_gradfun_init_x86(GradFunContext *gf)
{
#if HAVE_X86ASM
int cpu_flags = av_get_cpu_flags();

if (EXTERNAL_MMXEXT(cpu_flags))
gf->filter_line = gradfun_filter_line_mmxext;
if (EXTERNAL_SSSE3(cpu_flags))
gf->filter_line = gradfun_filter_line_ssse3;

if (EXTERNAL_SSE2(cpu_flags))
gf->blur_line = gradfun_blur_line_sse2;
#endif 
}
