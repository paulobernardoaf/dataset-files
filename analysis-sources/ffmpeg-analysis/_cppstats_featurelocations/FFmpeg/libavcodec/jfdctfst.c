



































































#include <stdlib.h>
#include <stdio.h>
#include "libavutil/common.h"
#include "dct.h"

#define DCTSIZE 8
#define GLOBAL(x) x
#define RIGHT_SHIFT(x, n) ((x) >> (n))





#if DCTSIZE != 8
Sorry, this code only copes with 8x8 DCTs. 
#endif




















#define CONST_BITS 8









#if CONST_BITS == 8
#define FIX_0_382683433 ((int32_t) 98) 
#define FIX_0_541196100 ((int32_t) 139) 
#define FIX_0_707106781 ((int32_t) 181) 
#define FIX_1_306562965 ((int32_t) 334) 
#else
#define FIX_0_382683433 FIX(0.382683433)
#define FIX_0_541196100 FIX(0.541196100)
#define FIX_0_707106781 FIX(0.707106781)
#define FIX_1_306562965 FIX(1.306562965)
#endif







#if !defined(USE_ACCURATE_ROUNDING)
#undef DESCALE
#define DESCALE(x,n) RIGHT_SHIFT(x, n)
#endif






#define MULTIPLY(var,const) ((int16_t) DESCALE((var) * (const), CONST_BITS))

static av_always_inline void row_fdct(int16_t * data){
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1, z2, z3, z4, z5, z11, z13;
int16_t *dataptr;
int ctr;



dataptr = data;
for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
tmp0 = dataptr[0] + dataptr[7];
tmp7 = dataptr[0] - dataptr[7];
tmp1 = dataptr[1] + dataptr[6];
tmp6 = dataptr[1] - dataptr[6];
tmp2 = dataptr[2] + dataptr[5];
tmp5 = dataptr[2] - dataptr[5];
tmp3 = dataptr[3] + dataptr[4];
tmp4 = dataptr[3] - dataptr[4];



tmp10 = tmp0 + tmp3; 
tmp13 = tmp0 - tmp3;
tmp11 = tmp1 + tmp2;
tmp12 = tmp1 - tmp2;

dataptr[0] = tmp10 + tmp11; 
dataptr[4] = tmp10 - tmp11;

z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); 
dataptr[2] = tmp13 + z1; 
dataptr[6] = tmp13 - z1;



tmp10 = tmp4 + tmp5; 
tmp11 = tmp5 + tmp6;
tmp12 = tmp6 + tmp7;


z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); 
z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; 
z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; 
z3 = MULTIPLY(tmp11, FIX_0_707106781); 

z11 = tmp7 + z3; 
z13 = tmp7 - z3;

dataptr[5] = z13 + z2; 
dataptr[3] = z13 - z2;
dataptr[1] = z11 + z4;
dataptr[7] = z11 - z4;

dataptr += DCTSIZE; 
}
}





GLOBAL(void)
ff_fdct_ifast (int16_t * data)
{
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1, z2, z3, z4, z5, z11, z13;
int16_t *dataptr;
int ctr;

row_fdct(data);



dataptr = data;
for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];



tmp10 = tmp0 + tmp3; 
tmp13 = tmp0 - tmp3;
tmp11 = tmp1 + tmp2;
tmp12 = tmp1 - tmp2;

dataptr[DCTSIZE*0] = tmp10 + tmp11; 
dataptr[DCTSIZE*4] = tmp10 - tmp11;

z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); 
dataptr[DCTSIZE*2] = tmp13 + z1; 
dataptr[DCTSIZE*6] = tmp13 - z1;



tmp10 = tmp4 + tmp5; 
tmp11 = tmp5 + tmp6;
tmp12 = tmp6 + tmp7;


z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); 
z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; 
z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; 
z3 = MULTIPLY(tmp11, FIX_0_707106781); 

z11 = tmp7 + z3; 
z13 = tmp7 - z3;

dataptr[DCTSIZE*5] = z13 + z2; 
dataptr[DCTSIZE*3] = z13 - z2;
dataptr[DCTSIZE*1] = z11 + z4;
dataptr[DCTSIZE*7] = z11 - z4;

dataptr++; 
}
}





GLOBAL(void)
ff_fdct_ifast248 (int16_t * data)
{
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1;
int16_t *dataptr;
int ctr;

row_fdct(data);



dataptr = data;
for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*1];
tmp1 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*3];
tmp2 = dataptr[DCTSIZE*4] + dataptr[DCTSIZE*5];
tmp3 = dataptr[DCTSIZE*6] + dataptr[DCTSIZE*7];
tmp4 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*1];
tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*3];
tmp6 = dataptr[DCTSIZE*4] - dataptr[DCTSIZE*5];
tmp7 = dataptr[DCTSIZE*6] - dataptr[DCTSIZE*7];



tmp10 = tmp0 + tmp3;
tmp11 = tmp1 + tmp2;
tmp12 = tmp1 - tmp2;
tmp13 = tmp0 - tmp3;

dataptr[DCTSIZE*0] = tmp10 + tmp11;
dataptr[DCTSIZE*4] = tmp10 - tmp11;

z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781);
dataptr[DCTSIZE*2] = tmp13 + z1;
dataptr[DCTSIZE*6] = tmp13 - z1;

tmp10 = tmp4 + tmp7;
tmp11 = tmp5 + tmp6;
tmp12 = tmp5 - tmp6;
tmp13 = tmp4 - tmp7;

dataptr[DCTSIZE*1] = tmp10 + tmp11;
dataptr[DCTSIZE*5] = tmp10 - tmp11;

z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781);
dataptr[DCTSIZE*3] = tmp13 + z1;
dataptr[DCTSIZE*7] = tmp13 - z1;

dataptr++; 
}
}


#undef GLOBAL
#undef CONST_BITS
#undef DESCALE
#undef FIX_0_541196100
#undef FIX_1_306562965
