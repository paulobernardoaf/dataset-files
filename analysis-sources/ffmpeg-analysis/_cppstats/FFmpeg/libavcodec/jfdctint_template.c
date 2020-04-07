#include "libavutil/common.h"
#include "dct.h"
#include "bit_depth_template.c"
#define DCTSIZE 8
#define BITS_IN_JSAMPLE BIT_DEPTH
#define GLOBAL(x) x
#define RIGHT_SHIFT(x, n) ((x) >> (n))
#define MULTIPLY16C16(var,const) ((var)*(const))
#define DESCALE(x,n) RIGHT_SHIFT((x) + (1 << ((n) - 1)), n)
#if DCTSIZE != 8
#error "Sorry, this code only copes with 8x8 DCTs."
#endif
#undef CONST_BITS
#undef PASS1_BITS
#undef OUT_SHIFT
#if BITS_IN_JSAMPLE == 8
#define CONST_BITS 13
#define PASS1_BITS 4 
#define OUT_SHIFT PASS1_BITS
#else
#define CONST_BITS 13
#define PASS1_BITS 1 
#define OUT_SHIFT (PASS1_BITS + 1)
#endif
#if CONST_BITS == 13
#define FIX_0_298631336 ((int32_t) 2446) 
#define FIX_0_390180644 ((int32_t) 3196) 
#define FIX_0_541196100 ((int32_t) 4433) 
#define FIX_0_765366865 ((int32_t) 6270) 
#define FIX_0_899976223 ((int32_t) 7373) 
#define FIX_1_175875602 ((int32_t) 9633) 
#define FIX_1_501321110 ((int32_t) 12299) 
#define FIX_1_847759065 ((int32_t) 15137) 
#define FIX_1_961570560 ((int32_t) 16069) 
#define FIX_2_053119869 ((int32_t) 16819) 
#define FIX_2_562915447 ((int32_t) 20995) 
#define FIX_3_072711026 ((int32_t) 25172) 
#else
#define FIX_0_298631336 FIX(0.298631336)
#define FIX_0_390180644 FIX(0.390180644)
#define FIX_0_541196100 FIX(0.541196100)
#define FIX_0_765366865 FIX(0.765366865)
#define FIX_0_899976223 FIX(0.899976223)
#define FIX_1_175875602 FIX(1.175875602)
#define FIX_1_501321110 FIX(1.501321110)
#define FIX_1_847759065 FIX(1.847759065)
#define FIX_1_961570560 FIX(1.961570560)
#define FIX_2_053119869 FIX(2.053119869)
#define FIX_2_562915447 FIX(2.562915447)
#define FIX_3_072711026 FIX(3.072711026)
#endif
#if BITS_IN_JSAMPLE == 8 && CONST_BITS<=13 && PASS1_BITS<=2
#define MULTIPLY(var,const) MULTIPLY16C16(var,const)
#else
#define MULTIPLY(var,const) ((var) * (const))
#endif
static av_always_inline void FUNC(row_fdct)(int16_t *data)
{
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1, z2, z3, z4, z5;
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
dataptr[0] = (int16_t) ((tmp10 + tmp11) * (1 << PASS1_BITS));
dataptr[4] = (int16_t) ((tmp10 - tmp11) * (1 << PASS1_BITS));
z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
dataptr[2] = (int16_t) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
CONST_BITS-PASS1_BITS);
dataptr[6] = (int16_t) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
CONST_BITS-PASS1_BITS);
z1 = tmp4 + tmp7;
z2 = tmp5 + tmp6;
z3 = tmp4 + tmp6;
z4 = tmp5 + tmp7;
z5 = MULTIPLY(z3 + z4, FIX_1_175875602); 
tmp4 = MULTIPLY(tmp4, FIX_0_298631336); 
tmp5 = MULTIPLY(tmp5, FIX_2_053119869); 
tmp6 = MULTIPLY(tmp6, FIX_3_072711026); 
tmp7 = MULTIPLY(tmp7, FIX_1_501321110); 
z1 = MULTIPLY(z1, - FIX_0_899976223); 
z2 = MULTIPLY(z2, - FIX_2_562915447); 
z3 = MULTIPLY(z3, - FIX_1_961570560); 
z4 = MULTIPLY(z4, - FIX_0_390180644); 
z3 += z5;
z4 += z5;
dataptr[7] = (int16_t) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
dataptr[5] = (int16_t) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
dataptr[3] = (int16_t) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
dataptr[1] = (int16_t) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);
dataptr += DCTSIZE; 
}
}
GLOBAL(void)
FUNC(ff_jpeg_fdct_islow)(int16_t *data)
{
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1, z2, z3, z4, z5;
int16_t *dataptr;
int ctr;
FUNC(row_fdct)(data);
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
dataptr[DCTSIZE*0] = DESCALE(tmp10 + tmp11, OUT_SHIFT);
dataptr[DCTSIZE*4] = DESCALE(tmp10 - tmp11, OUT_SHIFT);
z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
dataptr[DCTSIZE*2] = DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
CONST_BITS + OUT_SHIFT);
dataptr[DCTSIZE*6] = DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
CONST_BITS + OUT_SHIFT);
z1 = tmp4 + tmp7;
z2 = tmp5 + tmp6;
z3 = tmp4 + tmp6;
z4 = tmp5 + tmp7;
z5 = MULTIPLY(z3 + z4, FIX_1_175875602); 
tmp4 = MULTIPLY(tmp4, FIX_0_298631336); 
tmp5 = MULTIPLY(tmp5, FIX_2_053119869); 
tmp6 = MULTIPLY(tmp6, FIX_3_072711026); 
tmp7 = MULTIPLY(tmp7, FIX_1_501321110); 
z1 = MULTIPLY(z1, - FIX_0_899976223); 
z2 = MULTIPLY(z2, - FIX_2_562915447); 
z3 = MULTIPLY(z3, - FIX_1_961570560); 
z4 = MULTIPLY(z4, - FIX_0_390180644); 
z3 += z5;
z4 += z5;
dataptr[DCTSIZE*7] = DESCALE(tmp4 + z1 + z3, CONST_BITS + OUT_SHIFT);
dataptr[DCTSIZE*5] = DESCALE(tmp5 + z2 + z4, CONST_BITS + OUT_SHIFT);
dataptr[DCTSIZE*3] = DESCALE(tmp6 + z2 + z3, CONST_BITS + OUT_SHIFT);
dataptr[DCTSIZE*1] = DESCALE(tmp7 + z1 + z4, CONST_BITS + OUT_SHIFT);
dataptr++; 
}
}
GLOBAL(void)
FUNC(ff_fdct248_islow)(int16_t *data)
{
int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
int tmp10, tmp11, tmp12, tmp13;
int z1;
int16_t *dataptr;
int ctr;
FUNC(row_fdct)(data);
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
dataptr[DCTSIZE*0] = DESCALE(tmp10 + tmp11, OUT_SHIFT);
dataptr[DCTSIZE*4] = DESCALE(tmp10 - tmp11, OUT_SHIFT);
z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
dataptr[DCTSIZE*2] = DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
CONST_BITS+OUT_SHIFT);
dataptr[DCTSIZE*6] = DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
CONST_BITS+OUT_SHIFT);
tmp10 = tmp4 + tmp7;
tmp11 = tmp5 + tmp6;
tmp12 = tmp5 - tmp6;
tmp13 = tmp4 - tmp7;
dataptr[DCTSIZE*1] = DESCALE(tmp10 + tmp11, OUT_SHIFT);
dataptr[DCTSIZE*5] = DESCALE(tmp10 - tmp11, OUT_SHIFT);
z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
dataptr[DCTSIZE*3] = DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
CONST_BITS + OUT_SHIFT);
dataptr[DCTSIZE*7] = DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
CONST_BITS + OUT_SHIFT);
dataptr++; 
}
}
