



















#include "checkasm.h"
#include "libavcodec/jpeg2000dsp.h"
#include "libavutil/common.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"

#define BUF_SIZE 512

#define randomize_buffers() do { int i; for (i = 0; i < BUF_SIZE*3; i++) src[i] = rnd(); } while (0)






#define randomize_buffers_float() do { int i; for (i = 0; i < BUF_SIZE*3; i++) src[i] = (float)rnd() / (UINT_MAX >> 5); } while (0)






static void check_rct_int(void)
{
LOCAL_ALIGNED_32(int32_t, src, [BUF_SIZE*3]);
LOCAL_ALIGNED_32(int32_t, ref, [BUF_SIZE*3]);
LOCAL_ALIGNED_32(int32_t, new, [BUF_SIZE*3]);
int32_t *ref0 = &ref[BUF_SIZE*0], *new0 = &new[BUF_SIZE*0];
int32_t *ref1 = &ref[BUF_SIZE*1], *new1 = &new[BUF_SIZE*1];
int32_t *ref2 = &ref[BUF_SIZE*2], *new2 = &new[BUF_SIZE*2];

declare_func(void, void *src0, void *src1, void *src2, int csize);

randomize_buffers();
memcpy(ref, src, BUF_SIZE * 3 * sizeof(*src));
memcpy(new, src, BUF_SIZE * 3 * sizeof(*src));
call_ref(ref0, ref1, ref2, BUF_SIZE);
call_new(new0, new1, new2, BUF_SIZE);
if (memcmp(ref0, new0, BUF_SIZE * sizeof(*src)) ||
memcmp(ref1, new1, BUF_SIZE * sizeof(*src)) ||
memcmp(ref2, new2, BUF_SIZE * sizeof(*src)))
fail();
memcpy(new, src, BUF_SIZE * 3 * sizeof(*src));
bench_new(new0, new1, new2, BUF_SIZE);
}

static void check_ict_float(void)
{
LOCAL_ALIGNED_32(float, src, [BUF_SIZE*3]);
LOCAL_ALIGNED_32(float, ref, [BUF_SIZE*3]);
LOCAL_ALIGNED_32(float, new, [BUF_SIZE*3]);
float *ref0 = &ref[BUF_SIZE*0], *new0 = &new[BUF_SIZE*0];
float *ref1 = &ref[BUF_SIZE*1], *new1 = &new[BUF_SIZE*1];
float *ref2 = &ref[BUF_SIZE*2], *new2 = &new[BUF_SIZE*2];

declare_func(void, void *src0, void *src1, void *src2, int csize);

randomize_buffers_float();
memcpy(ref, src, BUF_SIZE * 3 * sizeof(*src));
memcpy(new, src, BUF_SIZE * 3 * sizeof(*src));
call_ref(ref0, ref1, ref2, BUF_SIZE);
call_new(new0, new1, new2, BUF_SIZE);
if (!float_near_abs_eps_array(ref0, new0, 1.0e-5, BUF_SIZE) ||
!float_near_abs_eps_array(ref1, new1, 1.0e-5, BUF_SIZE) ||
!float_near_abs_eps_array(ref2, new2, 1.0e-5, BUF_SIZE))
fail();
memcpy(new, src, BUF_SIZE * 3 * sizeof(*src));
bench_new(new0, new1, new2, BUF_SIZE);
}

void checkasm_check_jpeg2000dsp(void)
{
Jpeg2000DSPContext h;

ff_jpeg2000dsp_init(&h);

if (check_func(h.mct_decode[FF_DWT53], "jpeg2000_rct_int"))
check_rct_int();
if (check_func(h.mct_decode[FF_DWT97], "jpeg2000_ict_float"))
check_ict_float();

report("mct_decode");
}
