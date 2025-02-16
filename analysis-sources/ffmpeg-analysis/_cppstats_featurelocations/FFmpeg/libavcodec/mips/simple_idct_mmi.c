























#include "idctdsp_mips.h"
#include "constants.h"
#include "libavutil/mips/asmdefs.h"
#include "libavutil/mips/mmiutils.h"

#define W1 22725 
#define W2 21407 
#define W3 19266 
#define W4 16383 
#define W5 12873 
#define W6 8867 
#define W7 4520 

#define ROW_SHIFT 11
#define COL_SHIFT 20
#define DC_SHIFT 3

DECLARE_ALIGNED(16, const int16_t, W_arr)[46] = {
W4, W2, W4, W6,
W1, W3, W5, W7,
W4, W6, -W4, -W2,
W3, -W7, -W1, -W5,
W4, -W6, -W4, W2,
W5, -W1, W7, W3,
W4, -W2, W4, -W6,
W7, -W5, W3, -W1,
1024, 0, 1024, 0, 
0, -1, -1, -1, 
32, 32, 32, 32 
};

void ff_simple_idct_8_mmi(int16_t *block)
{
BACKUP_REG
__asm__ volatile (

#define IDCT_ROW_COND_DC(src1, src2) "dmfc1 $11, "#src1" \n\t" "dmfc1 $12, "#src2" \n\t" "and $11, $11, $9 \n\t" "or $10, $11, $12 \n\t" "beqz $10, 1f \n\t" "punpcklhw $f30, "#src1", "#src2" \n\t" "punpckhhw $f31, "#src1", "#src2" \n\t" "punpcklhw "#src1", $f30, $f31 \n\t" "punpckhhw "#src2", $f30, $f31 \n\t" "pmaddhw $f30, "#src1", $f18 \n\t" "pmaddhw $f31, "#src2", $f19 \n\t" "paddw $f28, $f30, $f31 \n\t" "psubw $f29, $f30, $f31 \n\t" "punpcklwd $f30, $f28, $f29 \n\t" "punpckhwd $f31, $f28, $f29 \n\t" "paddw $f26, $f30, $f31 \n\t" "paddw $f26, $f26, $f16 \n\t" "psraw $f26, $f26, $f17 \n\t" "pmaddhw $f30, "#src1", $f20 \n\t" "pmaddhw $f31, "#src2", $f21 \n\t" "paddw $f28, $f30, $f31 \n\t" "psubw $f29, $f30, $f31 \n\t" "punpcklwd $f30, $f28, $f29 \n\t" "punpckhwd $f31, $f28, $f29 \n\t" "paddw $f27, $f30, $f31 \n\t" "paddw $f27, $f27, $f16 \n\t" "psraw $f27, $f27, $f17 \n\t" "pmaddhw $f30, "#src1", $f22 \n\t" "pmaddhw $f31, "#src2", $f23 \n\t" "paddw $f28, $f30, $f31 \n\t" "psubw $f29, $f30, $f31 \n\t" "punpcklwd $f30, $f28, $f29 \n\t" "punpckhwd $f31, $f28, $f29 \n\t" "paddw $f28, $f30, $f31 \n\t" "paddw $f28, $f28, $f16 \n\t" "psraw $f28, $f28, $f17 \n\t" "pmaddhw $f30, "#src1", $f24 \n\t" "pmaddhw $f31, "#src2", $f25 \n\t" "paddw "#src1", $f30, $f31 \n\t" "psubw "#src2", $f30, $f31 \n\t" "punpcklwd $f30, "#src1", "#src2" \n\t" "punpckhwd $f31, "#src1", "#src2" \n\t" "paddw $f29, $f30, $f31 \n\t" "paddw $f29, $f29, $f16 \n\t" "psraw $f29, $f29, $f17 \n\t" "punpcklhw "#src1", $f26, $f27 \n\t" "punpckhhw $f30, $f27, $f26 \n\t" "punpcklhw $f31, $f28, $f29 \n\t" "punpckhhw "#src2", $f29, $f28 \n\t" "punpcklwd "#src1", "#src1", $f31 \n\t" "punpcklwd "#src2", "#src2", $f30 \n\t" "j 2f \n\t" "1: \n\t" "li $10, 3 \n\t" "dmtc1 $10, $f30 \n\t" "psllh $f28, "#src1", $f30 \n\t" "dmtc1 $9, $f31 \n\t" "punpcklhw $f29, $f28, $f28 \n\t" "and $f29, $f29, $f31 \n\t" "paddw $f28, $f28, $f29 \n\t" "punpcklwd "#src1", $f28, $f28 \n\t" "punpcklwd "#src2", $f28, $f28 \n\t" "2: \n\t" 


















































































"gslqc1 $f19, $f18, 0x00(%[w_arr]) \n\t"
"gslqc1 $f21, $f20, 0x10(%[w_arr]) \n\t"
"gslqc1 $f23, $f22, 0x20(%[w_arr]) \n\t"
"gslqc1 $f25, $f24, 0x30(%[w_arr]) \n\t"
"gslqc1 $f17, $f16, 0x40(%[w_arr]) \n\t"

"gslqc1 $f1, $f0, 0x00(%[block]) \n\t"
"gslqc1 $f3, $f2, 0x10(%[block]) \n\t"
"gslqc1 $f5, $f4, 0x20(%[block]) \n\t"
"gslqc1 $f7, $f6, 0x30(%[block]) \n\t"
"gslqc1 $f9, $f8, 0x40(%[block]) \n\t"
"gslqc1 $f11, $f10, 0x50(%[block]) \n\t"
"gslqc1 $f13, $f12, 0x60(%[block]) \n\t"
"gslqc1 $f15, $f14, 0x70(%[block]) \n\t"


"dmfc1 $9, $f17 \n\t"
"li $10, 11 \n\t"
"mtc1 $10, $f17 \n\t"
IDCT_ROW_COND_DC($f0,$f1)
IDCT_ROW_COND_DC($f2,$f3)
IDCT_ROW_COND_DC($f4,$f5)
IDCT_ROW_COND_DC($f6,$f7)
IDCT_ROW_COND_DC($f8,$f9)
IDCT_ROW_COND_DC($f10,$f11)
IDCT_ROW_COND_DC($f12,$f13)
IDCT_ROW_COND_DC($f14,$f15)

#define IDCT_COL_CASE1(src, out1, out2) "pmaddhw $f26, "#src", $f18 \n\t" "pmaddhw $f27, "#src", $f20 \n\t" "pmaddhw $f28, "#src", $f22 \n\t" "pmaddhw $f29, "#src", $f24 \n\t" "punpcklwd $f30, $f26, $f26 \n\t" "punpckhwd $f31, $f26, $f26 \n\t" "paddw $f26, $f30, $f31 \n\t" "punpcklwd $f30, $f27, $f27 \n\t" "punpckhwd $f31, $f27, $f27 \n\t" "paddw $f27, $f30, $f31 \n\t" "punpcklwd $f30, $f28, $f28 \n\t" "punpckhwd $f31, $f28, $f28 \n\t" "paddw $f28, $f30, $f31 \n\t" "punpcklwd $f30, $f29, $f29 \n\t" "punpckhwd $f31, $f29, $f29 \n\t" "paddw $f29, $f30, $f31 \n\t" "punpcklhw $f30, $f26, $f27 \n\t" "punpcklhw $f31, $f28, $f29 \n\t" "punpckhwd "#out1", $f30, $f31 \n\t" "psrah "#out1", "#out1", $f16 \n\t" "punpcklhw $f30, $f27, $f26 \n\t" "punpcklhw $f31, $f29, $f28 \n\t" "punpckhwd "#out2", $f31, $f30 \n\t" "psrah "#out2", "#out2", $f16 \n\t"

































#define IDCT_COL_CASE2(src1, src2, out1, out2) "pmaddhw $f28, "#src1", $f18 \n\t" "pmaddhw $f29, "#src2", $f19 \n\t" "paddw $f30, $f28, $f29 \n\t" "psubw $f31, $f28, $f29 \n\t" "punpcklwd $f28, $f30, $f31 \n\t" "punpckhwd $f29, $f30, $f31 \n\t" "pmaddhw $f30, "#src1", $f20 \n\t" "pmaddhw $f31, "#src2", $f21 \n\t" "paddw $f26, $f28, $f29 \n\t" "paddw $f28, $f30, $f31 \n\t" "psubw $f29, $f30, $f31 \n\t" "punpcklwd $f30, $f28, $f29 \n\t" "punpckhwd $f31, $f28, $f29 \n\t" "pmaddhw $f28, "#src1", $f22 \n\t" "pmaddhw $f29, "#src2", $f23 \n\t" "paddw $f27, $f30, $f31 \n\t" "paddw $f30, $f28, $f29 \n\t" "psubw $f31, $f28, $f29 \n\t" "punpcklwd $f28, $f30, $f31 \n\t" "punpckhwd $f29, $f30, $f31 \n\t" "pmaddhw $f30, "#src1", $f24 \n\t" "pmaddhw $f31, "#src2", $f25 \n\t" "paddw $f28, $f28, $f29 \n\t" "paddw "#out1", $f30, $f31 \n\t" "psubw "#out2", $f30, $f31 \n\t" "punpcklwd $f30, "#out1", "#out2" \n\t" "punpckhwd $f31, "#out1", "#out2" \n\t" "paddw $f29, $f30, $f31 \n\t" "punpcklhw "#out1", $f26, $f27 \n\t" "punpckhhw "#out2", $f27, $f26 \n\t" "punpcklhw $f30, $f28, $f29 \n\t" "punpckhhw $f31, $f29, $f28 \n\t" "punpckhwd "#out1", "#out1", $f30 \n\t" "punpckhwd "#out2", $f31, "#out2" \n\t" "psrah "#out1", "#out1", $f16 \n\t" "psrah "#out2", "#out2", $f16 \n\t"
















































"gsldlc1 $f17, 0x57(%[w_arr]) \n\t"
"gsldrc1 $f17, 0x50(%[w_arr]) \n\t"
"li $10, 4 \n\t"
"dmtc1 $10, $f16 \n\t"
"paddh $f0, $f0, $f17 \n\t"

"punpcklhw $f26, $f0, $f4 \n\t"
"punpckhhw $f27, $f0, $f4 \n\t"
"punpcklhw $f28, $f8, $f12 \n\t"
"punpckhhw $f29, $f8, $f12 \n\t"
"punpcklwd $f0, $f26, $f28 \n\t"
"punpckhwd $f4, $f26, $f28 \n\t"
"punpcklwd $f8, $f27, $f29 \n\t"
"punpckhwd $f12, $f27, $f29 \n\t"

"or $f26, $f2, $f6 \n\t"
"or $f26, $f26, $f10 \n\t"
"or $f26, $f26, $f14 \n\t"
"dmfc1 $10, $f26 \n\t"
"bnez $10, 1f \n\t"


IDCT_COL_CASE1($f0, $f0, $f2)

IDCT_COL_CASE1($f4, $f4, $f6)

IDCT_COL_CASE1($f8, $f8, $f10)

IDCT_COL_CASE1($f12, $f12, $f14)
"j 2f \n\t"

"1: \n\t"


"punpcklhw $f26, $f2, $f6 \n\t"
"punpckhhw $f27, $f2, $f6 \n\t"
"punpcklhw $f28, $f10, $f14 \n\t"
"punpckhhw $f29, $f10, $f14 \n\t"
"punpcklwd $f2, $f26, $f28 \n\t"
"punpckhwd $f6, $f26, $f28 \n\t"
"punpcklwd $f10, $f27, $f29 \n\t"
"punpckhwd $f14, $f27, $f29 \n\t"


IDCT_COL_CASE2($f0, $f2, $f0, $f2)

IDCT_COL_CASE2($f4, $f6, $f4, $f6)

IDCT_COL_CASE2($f8, $f10, $f8, $f10)

IDCT_COL_CASE2($f12, $f14, $f12, $f14)

"2: \n\t"

"punpcklhw $f26, $f0, $f4 \n\t"
"punpckhhw $f27, $f0, $f4 \n\t"
"punpcklhw $f28, $f8, $f12 \n\t"
"punpckhhw $f29, $f8, $f12 \n\t"
"punpcklwd $f0, $f26, $f28 \n\t"
"punpckhwd $f4, $f26, $f28 \n\t"
"punpcklwd $f8, $f27, $f29 \n\t"
"punpckhwd $f12, $f27, $f29 \n\t"

"punpcklhw $f26, $f2, $f6 \n\t"
"punpckhhw $f27, $f2, $f6 \n\t"
"punpcklhw $f28, $f10, $f14 \n\t"
"punpckhhw $f29, $f10, $f14 \n\t"
"punpcklwd $f2, $f26, $f28 \n\t"
"punpckhwd $f6, $f26, $f28 \n\t"
"punpcklwd $f10, $f27, $f29 \n\t"
"punpckhwd $f14, $f27, $f29 \n\t"



"paddh $f1, $f1, $f17 \n\t"

"punpcklhw $f26, $f1, $f5 \n\t"
"punpckhhw $f27, $f1, $f5 \n\t"
"punpcklhw $f28, $f9, $f13 \n\t"
"punpckhhw $f29, $f9, $f13 \n\t"
"punpcklwd $f1, $f26, $f28 \n\t"
"punpckhwd $f5, $f26, $f28 \n\t"
"punpcklwd $f9, $f27, $f29 \n\t"
"punpckhwd $f13, $f27, $f29 \n\t"

"or $f26, $f3, $f7 \n\t"
"or $f26, $f26, $f11 \n\t"
"or $f26, $f26, $f15 \n\t"
"dmfc1 $10, $f26 \n\t"
"bnez $10, 1f \n\t"


IDCT_COL_CASE1($f1, $f1, $f3)

IDCT_COL_CASE1($f5, $f5, $f7)

IDCT_COL_CASE1($f9, $f9, $f11)

IDCT_COL_CASE1($f13, $f13, $f15)
"j 2f \n\t"

"1: \n\t"


"punpcklhw $f26, $f3, $f7 \n\t"
"punpckhhw $f27, $f3, $f7 \n\t"
"punpcklhw $f28, $f11, $f15 \n\t"
"punpckhhw $f29, $f11, $f15 \n\t"
"punpcklwd $f3, $f26, $f28 \n\t"
"punpckhwd $f7, $f26, $f28 \n\t"
"punpcklwd $f11, $f27, $f29 \n\t"
"punpckhwd $f15, $f27, $f29 \n\t"


IDCT_COL_CASE2($f1, $f3, $f1, $f3)

IDCT_COL_CASE2($f5, $f7, $f5, $f7)

IDCT_COL_CASE2($f9, $f11, $f9, $f11)

IDCT_COL_CASE2($f13, $f15, $f13, $f15)

"2: \n\t"

"punpcklhw $f26, $f1, $f5 \n\t"
"punpckhhw $f27, $f1, $f5 \n\t"
"punpcklhw $f28, $f9, $f13 \n\t"
"punpckhhw $f29, $f9, $f13 \n\t"
"punpcklwd $f1, $f26, $f28 \n\t"
"punpckhwd $f5, $f26, $f28 \n\t"
"punpcklwd $f9, $f27, $f29 \n\t"
"punpckhwd $f13, $f27, $f29 \n\t"

"punpcklhw $f26, $f3, $f7 \n\t"
"punpckhhw $f27, $f3, $f7 \n\t"
"punpcklhw $f28, $f11, $f15 \n\t"
"punpckhhw $f29, $f11, $f15 \n\t"
"punpcklwd $f3, $f26, $f28 \n\t"
"punpckhwd $f7, $f26, $f28 \n\t"
"punpcklwd $f11, $f27, $f29 \n\t"
"punpckhwd $f15, $f27, $f29 \n\t"

"gssqc1 $f1, $f0, 0x00(%[block]) \n\t"
"gssqc1 $f5, $f4, 0x10(%[block]) \n\t"
"gssqc1 $f9, $f8, 0x20(%[block]) \n\t"
"gssqc1 $f13, $f12, 0x30(%[block]) \n\t"
"gssqc1 $f3, $f2, 0x40(%[block]) \n\t"
"gssqc1 $f7, $f6, 0x50(%[block]) \n\t"
"gssqc1 $f11, $f10, 0x60(%[block]) \n\t"
"gssqc1 $f15, $f14, 0x70(%[block]) \n\t"

: [block]"+&r"(block)
: [w_arr]"r"(W_arr)
: "memory"
);

RECOVER_REG
}

void ff_simple_idct_put_8_mmi(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
ff_simple_idct_8_mmi(block);
ff_put_pixels_clamped_mmi(block, dest, line_size);
}
void ff_simple_idct_add_8_mmi(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
ff_simple_idct_8_mmi(block);
ff_add_pixels_clamped_mmi(block, dest, line_size);
}
