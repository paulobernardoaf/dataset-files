





















#include <stdlib.h>
#define CONFIG_HARDCODED_TABLES 0
#include "aac_defines.h"

#if USE_FIXED
#define TYPE_NAME "int32_t"
typedef int32_t INT32FLOAT;
#define ARRAY_RENAME(x) write_int32_t_ ##x
#define ARRAY_URENAME(x) write_uint32_t_ ##x
#include "aacps_fixed_tablegen.h"
#else
#define TYPE_NAME "float"
typedef float INT32FLOAT;
#define ARRAY_RENAME(x) write_float_ ##x
#define ARRAY_URENAME(x) write_float_ ##x
#include "aacps_tablegen.h"
#endif 
#include "tableprint.h"

void ARRAY_RENAME(3d_array) (const void *p, int b, int c, int d)
{
int i;
const INT32FLOAT *f = p;
for (i = 0; i < b; i++) {
printf("{\n");
ARRAY_URENAME(2d_array)(f, c, d);
printf("},\n");
f += c * d;
}
}

void ARRAY_RENAME(4d_array) (const void *p, int a, int b, int c, int d)
{
int i;
const INT32FLOAT *f = p;
for (i = 0; i < a; i++) {
printf("{\n");
ARRAY_RENAME(3d_array)(f, b, c, d);
printf("},\n");
f += b * c * d;
}
}

int main(void)
{
ps_tableinit();

write_fileheader();

printf("static const %s pd_re_smooth[8*8*8] = {\n", TYPE_NAME);
ARRAY_RENAME(array)(pd_re_smooth, 8*8*8);
printf("};\n");
printf("static const %s pd_im_smooth[8*8*8] = {\n", TYPE_NAME);
ARRAY_RENAME(array)(pd_im_smooth, 8*8*8);
printf("};\n");

printf("static const %s HA[46][8][4] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(HA, 46, 8, 4);
printf("};\n");
printf("static const %s HB[46][8][4] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(HB, 46, 8, 4);
printf("};\n");

printf("static const DECLARE_ALIGNED(16, %s, f20_0_8)[8][8][2] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(f20_0_8, 8, 8, 2);
printf("};\n");
printf("static const DECLARE_ALIGNED(16, %s, f34_0_12)[12][8][2] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(f34_0_12, 12, 8, 2);
printf("};\n");
printf("static const DECLARE_ALIGNED(16, %s, f34_1_8)[8][8][2] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(f34_1_8, 8, 8, 2);
printf("};\n");
printf("static const DECLARE_ALIGNED(16, %s, f34_2_4)[4][8][2] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(f34_2_4, 4, 8, 2);
printf("};\n");

printf("static const DECLARE_ALIGNED(16, %s, Q_fract_allpass)[2][50][3][2] = {\n", TYPE_NAME);
ARRAY_RENAME(4d_array)(Q_fract_allpass, 2, 50, 3, 2);
printf("};\n");
printf("static const DECLARE_ALIGNED(16, %s, phi_fract)[2][50][2] = {\n", TYPE_NAME);
ARRAY_RENAME(3d_array)(phi_fract, 2, 50, 2);
printf("};\n");

return 0;
}
