#include "idctdsp_mips.h"
#include "xvididct_mips.h"
#define BITS_INV_ACC 5 
#define SHIFT_INV_ROW (16 - BITS_INV_ACC) 
#define SHIFT_INV_COL (1 + BITS_INV_ACC) 
#define RND_INV_ROW (1024 * (6 - BITS_INV_ACC))
#define RND_INV_COL (16 * (BITS_INV_ACC - 3))
#define RND_INV_CORR (RND_INV_COL - 1)
#define BITS_FRW_ACC 3 
#define SHIFT_FRW_COL BITS_FRW_ACC
#define SHIFT_FRW_ROW (BITS_FRW_ACC + 17)
#define RND_FRW_ROW (262144*(BITS_FRW_ACC - 1))
DECLARE_ALIGNED(8, static const int16_t, tg_1_16)[4*4] = {
13036, 13036, 13036, 13036, 
27146, 27146, 27146, 27146, 
-21746,-21746,-21746,-21746, 
23170, 23170, 23170, 23170 
};
DECLARE_ALIGNED(8, static const int32_t, rounder_0)[2*8] = {
65536,65536,
3597, 3597,
2260, 2260,
1203, 1203,
0, 0,
120, 120,
512, 512,
512, 512
};
DECLARE_ALIGNED(8, static const int16_t, tab_i_04_mmi)[32*4] = {
16384, 21407, 16384, 8867, 
16384, 8867,-16384,-21407, 
16384, -8867, 16384,-21407, 
-16384, 21407, 16384, -8867, 
22725, 19266, 19266, -4520, 
12873, 4520,-22725,-12873, 
12873,-22725, 4520,-12873, 
4520, 19266, 19266,-22725, 
22725, 29692, 22725, 12299, 
22725, 12299,-22725,-29692, 
22725,-12299, 22725,-29692, 
-22725, 29692, 22725,-12299, 
31521, 26722, 26722, -6270, 
17855, 6270,-31521,-17855, 
17855,-31521, 6270,-17855, 
6270, 26722, 26722,-31521, 
21407, 27969, 21407, 11585, 
21407, 11585,-21407,-27969, 
21407,-11585, 21407,-27969, 
-21407, 27969, 21407,-11585, 
29692, 25172, 25172, -5906, 
16819, 5906,-29692,-16819, 
16819,-29692, 5906,-16819, 
5906, 25172, 25172,-29692, 
19266, 25172, 19266, 10426, 
19266, 10426,-19266,-25172, 
19266,-10426, 19266,-25172, 
-19266, 25172, 19266,-10426, 
26722, 22654, 22654, -5315, 
15137, 5315,-26722,-15137, 
15137,-26722, 5315,-15137, 
5315, 22654, 22654,-26722, 
};
#define DCT_8_INV_ROW_MMI(A1,A2,A3,A4) "dli $10, 0x88 \n\t" "ldc1 $f4, "#A1" \n\t" "dmtc1 $10, $f16 \n\t" "ldc1 $f10, 8+"#A1" \n\t" "ldc1 $f6, "#A3" \n\t" "pshufh $f0, $f4, $f16 \n\t" "ldc1 $f8, 8+"#A3" \n\t" "ldc1 $f12, 32+"#A3" \n\t" "pmaddhw $f6, $f6, $f0 \n\t" "dli $10, 0xdd \n\t" "pshufh $f2, $f10, $f16 \n\t" "dmtc1 $10, $f16 \n\t" "pmaddhw $f8, $f8, $f2 \n\t" "ldc1 $f14, 40+"#A3" \n\t" "pshufh $f4, $f4, $f16 \n\t" "pmaddhw $f12, $f12, $f4 \n\t" "pshufh $f10, $f10, $f16 \n\t" "ldc1 $f18, "#A4" \n\t" "pmaddhw $f14, $f14, $f10 \n\t" "paddw $f6, $f6, $f18 \n\t" "ldc1 $f16, 16+"#A3" \n\t" "pmaddhw $f0, $f0, $f16 \n\t" "ldc1 $f16, 24+"#A3" \n\t" "paddw $f6, $f6, $f8 \n\t" "pmaddhw $f2, $f2, $f16 \n\t" "ldc1 $f16, 48+"#A3" \n\t" "pmaddhw $f4, $f4, $f16 \n\t" "ldc1 $f16, 56+"#A3" \n\t" "paddw $f12, $f12, $f14 \n\t" "dli $10, 11 \n\t" "pmaddhw $f10, $f10, $f16 \n\t" "dmtc1 $10, $f16 \n\t" "psubw $f8, $f6, $f12 \n\t" "paddw $f6, $f6, $f12 \n\t" "paddw $f0, $f0, $f18 \n\t" "psraw $f6, $f6, $f16 \n\t" "paddw $f0, $f0, $f2 \n\t" "paddw $f4, $f4, $f10 \n\t" "psraw $f8, $f8, $f16 \n\t" "psubw $f14, $f0, $f4 \n\t" "paddw $f0, $f0, $f4 \n\t" "psraw $f0, $f0, $f16 \n\t" "psraw $f14, $f14, $f16 \n\t" "dli $10, 0xb1 \n\t" "packsswh $f6, $f6, $f0 \n\t" "dmtc1 $10, $f16 \n\t" "packsswh $f14, $f14, $f8 \n\t" "sdc1 $f6, "#A2" \n\t" "pshufh $f14, $f14, $f16 \n\t" "sdc1 $f14, 8+"#A2" \n\t" 
#define DCT_8_INV_COL(A1,A2) "ldc1 $f2, 2*8(%3) \n\t" "ldc1 $f6, 16*3+"#A1" \n\t" "ldc1 $f10, 16*5+"#A1" \n\t" "pmulhh $f0, $f2, $f6 \n\t" "ldc1 $f4, 0(%3) \n\t" "pmulhh $f2, $f2, $f10 \n\t" "ldc1 $f14, 16*7+"#A1" \n\t" "ldc1 $f12, 16*1+"#A1" \n\t" "pmulhh $f8, $f4, $f14 \n\t" "paddsh $f0, $f0, $f6 \n\t" "pmulhh $f4, $f4, $f12 \n\t" "paddsh $f2, $f2, $f6 \n\t" "psubsh $f0, $f0, $f10 \n\t" "ldc1 $f6, 3*8(%3) \n\t" "paddsh $f2, $f2, $f10 \n\t" "paddsh $f8, $f8, $f12 \n\t" "psubsh $f4, $f4, $f14 \n\t" "paddsh $f10, $f8, $f2 \n\t" "psubsh $f12, $f4, $f0 \n\t" "psubsh $f8, $f8, $f2 \n\t" "paddsh $f4, $f4, $f0 \n\t" "ldc1 $f14, 1*8(%3) \n\t" "sdc1 $f10, 3*16+"#A2" \n\t" "paddsh $f2, $f8, $f4 \n\t" "sdc1 $f12, 5*16+"#A2" \n\t" "psubsh $f8, $f8, $f4 \n\t" "ldc1 $f10, 2*16+"#A1" \n\t" "ldc1 $f12, 6*16+"#A1" \n\t" "pmulhh $f0, $f14, $f10 \n\t" "pmulhh $f14, $f14, $f12 \n\t" "pmulhh $f2, $f2, $f6 \n\t" "ldc1 $f4, 0*16+"#A1" \n\t" "pmulhh $f8, $f8, $f6 \n\t" "psubsh $f0, $f0, $f12 \n\t" "ldc1 $f12, 4*16+"#A1" \n\t" "paddsh $f14, $f14, $f10 \n\t" "psubsh $f6, $f4, $f12 \n\t" "paddsh $f4, $f4, $f12 \n\t" "paddsh $f10, $f4, $f14 \n\t" "psubsh $f12, $f6, $f0 \n\t" "psubsh $f4, $f4, $f14 \n\t" "paddsh $f6, $f6, $f0 \n\t" "paddsh $f2, $f2, $f2 \n\t" "paddsh $f8, $f8, $f8 \n\t" "psubsh $f14, $f6, $f2 \n\t" "dli $10, 6 \n\t" "paddsh $f6, $f6, $f2 \n\t" "dmtc1 $10, $f16 \n\t" "psubsh $f0, $f12, $f8 \n\t" "paddsh $f12, $f12, $f8 \n\t" "psrah $f6, $f6, $f16 \n\t" "psrah $f12, $f12, $f16 \n\t" "ldc1 $f2, 3*16+"#A2" \n\t" "psrah $f14, $f14, $f16 \n\t" "psrah $f0, $f0, $f16 \n\t" "sdc1 $f6, 1*16+"#A2" \n\t" "psubsh $f8, $f10, $f2 \n\t" "paddsh $f10, $f10, $f2 \n\t" "sdc1 $f12, 2*16+"#A2" \n\t" "ldc1 $f6, 5*16+"#A2" \n\t" "psrah $f10, $f10, $f16 \n\t" "psrah $f8, $f8, $f16 \n\t" "sdc1 $f0, 5*16+"#A2" \n\t" "psubsh $f12, $f4, $f6 \n\t" "paddsh $f4, $f4, $f6 \n\t" "sdc1 $f14, 6*16+"#A2" \n\t" "sdc1 $f10, 0*16+"#A2" \n\t" "psrah $f4, $f4, $f16 \n\t" "sdc1 $f8, 7*16+"#A2" \n\t" "psrah $f12, $f12, $f16 \n\t" "sdc1 $f4, 3*16+"#A2" \n\t" "sdc1 $f12, 4*16+"#A2" \n\t" 
void ff_xvid_idct_mmi(int16_t *block)
{
__asm__ volatile (
DCT_8_INV_ROW_MMI(0*16(%0), 0*16(%0), 64*0(%2), 8*0(%1))
DCT_8_INV_ROW_MMI(1*16(%0), 1*16(%0), 64*1(%2), 8*1(%1))
DCT_8_INV_ROW_MMI(2*16(%0), 2*16(%0), 64*2(%2), 8*2(%1))
DCT_8_INV_ROW_MMI(3*16(%0), 3*16(%0), 64*3(%2), 8*3(%1))
DCT_8_INV_ROW_MMI(4*16(%0), 4*16(%0), 64*0(%2), 8*4(%1))
DCT_8_INV_ROW_MMI(5*16(%0), 5*16(%0), 64*3(%2), 8*5(%1))
DCT_8_INV_ROW_MMI(6*16(%0), 6*16(%0), 64*2(%2), 8*6(%1))
DCT_8_INV_ROW_MMI(7*16(%0), 7*16(%0), 64*1(%2), 8*7(%1))
DCT_8_INV_COL(0(%0), 0(%0))
DCT_8_INV_COL(8(%0), 8(%0))
::"r"(block),"r"(rounder_0),"r"(tab_i_04_mmi),"r"(tg_1_16)
: "$10"
);
}
void ff_xvid_idct_put_mmi(uint8_t *dest, int32_t line_size, int16_t *block)
{
ff_xvid_idct_mmi(block);
ff_put_pixels_clamped_mmi(block, dest, line_size);
}
void ff_xvid_idct_add_mmi(uint8_t *dest, int32_t line_size, int16_t *block)
{
ff_xvid_idct_mmi(block);
ff_add_pixels_clamped_mmi(block, dest, line_size);
}
