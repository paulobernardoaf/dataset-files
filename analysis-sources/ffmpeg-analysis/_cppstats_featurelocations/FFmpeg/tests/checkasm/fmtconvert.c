



















#include <math.h>
#include <stdint.h>
#include <string.h>

#include "libavutil/internal.h"
#include "libavutil/common.h"
#include "libavcodec/fmtconvert.h"

#include "checkasm.h"

#define BUF_SIZE 1024

#define randomize_input(len) do { int k; for (k = 0; k < len; k++) { in[k] = rnd() - INT32_MAX; } for ( ; k < BUF_SIZE; k++) { in[k] = INT32_MAX; } } while (0)










void checkasm_check_fmtconvert(void)
{
FmtConvertContext c;
LOCAL_ALIGNED(32, float, dst0, [BUF_SIZE]);
LOCAL_ALIGNED(32, float, dst1, [BUF_SIZE]);
LOCAL_ALIGNED(32, int32_t, in, [BUF_SIZE]);
float scale_arr[128];
int length[] = {8, 16, 24, 56, 72, 128, 512, 520, 656, 768, 992};
int i, j;

for (i = 0; i < FF_ARRAY_ELEMS(scale_arr); i++)
scale_arr[i] = (FF_ARRAY_ELEMS(scale_arr) - FF_ARRAY_ELEMS(scale_arr) / 2) / 13;

ff_fmt_convert_init(&c, NULL);

memset(dst0, 0, sizeof(*dst0) * BUF_SIZE);
memset(dst1, 0, sizeof(*dst1) * BUF_SIZE);

if (check_func(c.int32_to_float_fmul_scalar, "int32_to_float_fmul_scalar")) {
declare_func(void, float *, const int32_t *, float, int);

for (i = 0; i < FF_ARRAY_ELEMS(scale_arr); i++) {
for (j = 0; j < FF_ARRAY_ELEMS(length); j++) {

randomize_input(length[j]);

call_ref(dst0, in, scale_arr[i], length[j]);
call_new(dst1, in, scale_arr[i], length[j]);

if (!float_near_ulp_array(dst0, dst1, 3, length[j])) {
fail();
break;
}

bench_new(dst1, in, scale_arr[i], length[j]);
}
}
}
if (check_func(c.int32_to_float_fmul_array8, "int32_to_float_fmul_array8")) {
declare_func(void, FmtConvertContext *, float *, const int32_t *,
const float *, int);

for (i = 0; i < 4; i++) {
for (j = 0; j < FF_ARRAY_ELEMS(length); j++) {

randomize_input(length[j]);

call_ref(&c, dst0, in, scale_arr, length[j]);
call_new(&c, dst1, in, scale_arr, length[j]);

if (!float_near_ulp_array(dst0, dst1, 3, length[j])) {
fail();
fprintf(stderr, "int32_to_float_fmul_array8: len: %d\n", length[j]);
break;
}

bench_new(&c, dst1, in, scale_arr, length[j]);
}
}
}
report("fmtconvert");
}
