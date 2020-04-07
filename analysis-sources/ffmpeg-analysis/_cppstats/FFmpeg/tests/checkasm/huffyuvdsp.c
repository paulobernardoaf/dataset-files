#include <string.h>
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"
#include "libavcodec/huffyuvdsp.h"
#include "checkasm.h"
#define randomize_buffers(buf, size) do { int j; for (j = 0; j < size; j++) buf[j] = rnd() & 0xFFFF; } while (0)
static void check_add_int16(HuffYUVDSPContext c, unsigned mask, int width, const char * name)
{
uint16_t *src0 = av_mallocz(width * sizeof(uint16_t));
uint16_t *src1 = av_mallocz(width * sizeof(uint16_t));
uint16_t *dst0 = av_mallocz(width * sizeof(uint16_t));
uint16_t *dst1 = av_mallocz(width * sizeof(uint16_t));
declare_func_emms(AV_CPU_FLAG_MMX, void, uint16_t *dst, uint16_t *src, unsigned mask, int w);
if (!src0 || !src1 || !dst0 || !dst1)
fail();
randomize_buffers(src0, width);
memcpy(src1, src0, width * sizeof(uint16_t));
if (check_func(c.add_int16, "%s", name)) {
call_ref(dst0, src0, mask, width);
call_new(dst1, src1, mask, width);
if (memcmp(dst0, dst1, width * sizeof(uint16_t)))
fail();
bench_new(dst1, src1, mask, width);
}
av_free(src0);
av_free(src1);
av_free(dst0);
av_free(dst1);
}
void checkasm_check_huffyuvdsp(void)
{
HuffYUVDSPContext c;
int width = 16 * av_clip(rnd(), 16, 128);
ff_huffyuvdsp_init(&c, AV_PIX_FMT_YUV422P);
check_add_int16(c, 65535, width, "add_int16_rnd_width");
report("add_int16_rnd_width");
check_add_int16(c, 65535, 16*128, "add_int16_128");
report("add_int16_128");
}
