#include <string.h>
#include "checkasm.h"
#include "libavfilter/hflip.h"
#include "libavutil/intreadwrite.h"
#define WIDTH 256
#define WIDTH_PADDED 256 + 32
#define randomize_buffers(buf, size) do { int j; uint8_t *tmp_buf = (uint8_t *)buf;for (j = 0; j < size; j++) tmp_buf[j] = rnd() & 0xFF; } while (0)
static void check_hflip(int step, const char * report_name){
LOCAL_ALIGNED_32(uint8_t, src, [WIDTH_PADDED]);
LOCAL_ALIGNED_32(uint8_t, dst_ref, [WIDTH_PADDED]);
LOCAL_ALIGNED_32(uint8_t, dst_new, [WIDTH_PADDED]);
int w = WIDTH;
int i;
int step_array[4] = {1, 1, 1, 1};
FlipContext s;
declare_func(void, const uint8_t *src, uint8_t *dst, int w);
memset(src, 0, WIDTH_PADDED);
memset(dst_ref, 0, WIDTH_PADDED);
memset(dst_new, 0, WIDTH_PADDED);
randomize_buffers(src, WIDTH_PADDED);
if (step == 2) {
w /= 2;
for (i = 0; i < 4; i++)
step_array[i] = step;
}
ff_hflip_init(&s, step_array, 4);
if (check_func(s.flip_line[0], "hflip_%s", report_name)) {
for (i = 1; i < w; i++) {
call_ref(src + (w - 1) * step, dst_ref, i);
call_new(src + (w - 1) * step, dst_new, i);
if (memcmp(dst_ref, dst_new, i * step))
fail();
}
bench_new(src + (w - 1) * step, dst_new, w);
}
}
void checkasm_check_vf_hflip(void)
{
check_hflip(1, "byte");
report("hflip_byte");
check_hflip(2, "short");
report("hflip_short");
}
