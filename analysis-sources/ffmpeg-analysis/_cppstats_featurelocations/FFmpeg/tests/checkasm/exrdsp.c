



















#include <string.h>

#include "checkasm.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/exrdsp.h"
#include "libavutil/intreadwrite.h"

#define BUF_SIZE 5120
#define PADDED_BUF_SIZE BUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE*2

#define randomize_buffers() do { int i; for (i = 0; i < BUF_SIZE; i += 4) { uint32_t r = rnd(); AV_WN32A(src + i, r); } } while (0)








static void check_reorder_pixels(void) {
LOCAL_ALIGNED_32(uint8_t, src, [PADDED_BUF_SIZE]);
LOCAL_ALIGNED_32(uint8_t, dst_ref, [PADDED_BUF_SIZE]);
LOCAL_ALIGNED_32(uint8_t, dst_new, [PADDED_BUF_SIZE]);

declare_func(void, uint8_t *dst, const uint8_t *src, ptrdiff_t size);

memset(src, 0, PADDED_BUF_SIZE);
memset(dst_ref, 0, PADDED_BUF_SIZE);
memset(dst_new, 0, PADDED_BUF_SIZE);
randomize_buffers();
call_ref(dst_ref, src, BUF_SIZE);
call_new(dst_new, src, BUF_SIZE);
if (memcmp(dst_ref, dst_new, BUF_SIZE))
fail();
bench_new(dst_new, src, BUF_SIZE);
}

static void check_predictor(void) {
LOCAL_ALIGNED_32(uint8_t, src, [PADDED_BUF_SIZE]);
LOCAL_ALIGNED_32(uint8_t, dst_ref, [PADDED_BUF_SIZE]);
LOCAL_ALIGNED_32(uint8_t, dst_new, [PADDED_BUF_SIZE]);

declare_func(void, uint8_t *src, ptrdiff_t size);

memset(src, 0, PADDED_BUF_SIZE);
randomize_buffers();
memcpy(dst_ref, src, PADDED_BUF_SIZE);
memcpy(dst_new, src, PADDED_BUF_SIZE);
call_ref(dst_ref, BUF_SIZE);
call_new(dst_new, BUF_SIZE);
if (memcmp(dst_ref, dst_new, BUF_SIZE))
fail();
bench_new(dst_new, BUF_SIZE);
}

void checkasm_check_exrdsp(void)
{
ExrDSPContext h;

ff_exrdsp_init(&h);

if (check_func(h.reorder_pixels, "reorder_pixels"))
check_reorder_pixels();

report("reorder_pixels");

if (check_func(h.predictor, "predictor"))
check_predictor();

report("predictor");
}
