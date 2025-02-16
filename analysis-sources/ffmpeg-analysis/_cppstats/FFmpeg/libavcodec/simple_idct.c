#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "mathops.h"
#include "simple_idct.h"
#define IN_IDCT_DEPTH 16
#define BIT_DEPTH 8
#include "simple_idct_template.c"
#undef BIT_DEPTH
#define BIT_DEPTH 10
#include "simple_idct_template.c"
#define EXTRA_SHIFT 2
#include "simple_idct_template.c"
#undef EXTRA_SHIFT
#undef BIT_DEPTH
#define BIT_DEPTH 12
#include "simple_idct_template.c"
#undef BIT_DEPTH
#undef IN_IDCT_DEPTH
#define IN_IDCT_DEPTH 32
#define BIT_DEPTH 10
#include "simple_idct_template.c"
#undef BIT_DEPTH
#undef IN_IDCT_DEPTH
#define CN_SHIFT 12
#define C_FIX(x) ((int)((x) * (1 << CN_SHIFT) + 0.5))
#define C1 C_FIX(0.6532814824)
#define C2 C_FIX(0.2705980501)
#define C_SHIFT (4+1+12)
static inline void idct4col_put(uint8_t *dest, ptrdiff_t line_size, const int16_t *col)
{
int c0, c1, c2, c3, a0, a1, a2, a3;
a0 = col[8*0];
a1 = col[8*2];
a2 = col[8*4];
a3 = col[8*6];
c0 = ((a0 + a2) * (1 << CN_SHIFT - 1)) + (1 << (C_SHIFT - 1));
c2 = ((a0 - a2) * (1 << CN_SHIFT - 1)) + (1 << (C_SHIFT - 1));
c1 = a1 * C1 + a3 * C2;
c3 = a1 * C2 - a3 * C1;
dest[0] = av_clip_uint8((c0 + c1) >> C_SHIFT);
dest += line_size;
dest[0] = av_clip_uint8((c2 + c3) >> C_SHIFT);
dest += line_size;
dest[0] = av_clip_uint8((c2 - c3) >> C_SHIFT);
dest += line_size;
dest[0] = av_clip_uint8((c0 - c1) >> C_SHIFT);
}
#define BF(k) {int a0, a1;a0 = ptr[k];a1 = ptr[8 + k];ptr[k] = a0 + a1;ptr[8 + k] = a0 - a1;}
void ff_simple_idct248_put(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
int i;
int16_t *ptr;
ptr = block;
for(i=0;i<4;i++) {
BF(0);
BF(1);
BF(2);
BF(3);
BF(4);
BF(5);
BF(6);
BF(7);
ptr += 2 * 8;
}
for(i=0; i<8; i++) {
idctRowCondDC_int16_8bit(block + i*8, 0);
}
for(i=0;i<8;i++) {
idct4col_put(dest + i, 2 * line_size, block + i);
idct4col_put(dest + line_size + i, 2 * line_size, block + 8 + i);
}
}
#undef CN_SHIFT
#undef C_SHIFT
#undef C_FIX
#undef C1
#undef C2
#define CN_SHIFT 12
#define C_FIX(x) ((int)((x) * M_SQRT2 * (1 << CN_SHIFT) + 0.5))
#define C1 C_FIX(0.6532814824)
#define C2 C_FIX(0.2705980501)
#define C3 C_FIX(0.5)
#define C_SHIFT (4+1+12)
static inline void idct4col_add(uint8_t *dest, ptrdiff_t line_size, const int16_t *col)
{
int c0, c1, c2, c3, a0, a1, a2, a3;
a0 = col[8*0];
a1 = col[8*1];
a2 = col[8*2];
a3 = col[8*3];
c0 = (a0 + a2)*C3 + (1 << (C_SHIFT - 1));
c2 = (a0 - a2)*C3 + (1 << (C_SHIFT - 1));
c1 = a1 * C1 + a3 * C2;
c3 = a1 * C2 - a3 * C1;
dest[0] = av_clip_uint8(dest[0] + ((c0 + c1) >> C_SHIFT));
dest += line_size;
dest[0] = av_clip_uint8(dest[0] + ((c2 + c3) >> C_SHIFT));
dest += line_size;
dest[0] = av_clip_uint8(dest[0] + ((c2 - c3) >> C_SHIFT));
dest += line_size;
dest[0] = av_clip_uint8(dest[0] + ((c0 - c1) >> C_SHIFT));
}
#define RN_SHIFT 15
#define R_FIX(x) ((int)((x) * M_SQRT2 * (1 << RN_SHIFT) + 0.5))
#define R1 R_FIX(0.6532814824)
#define R2 R_FIX(0.2705980501)
#define R3 R_FIX(0.5)
#define R_SHIFT 11
static inline void idct4row(int16_t *row)
{
int c0, c1, c2, c3, a0, a1, a2, a3;
a0 = row[0];
a1 = row[1];
a2 = row[2];
a3 = row[3];
c0 = (a0 + a2)*R3 + (1 << (R_SHIFT - 1));
c2 = (a0 - a2)*R3 + (1 << (R_SHIFT - 1));
c1 = a1 * R1 + a3 * R2;
c3 = a1 * R2 - a3 * R1;
row[0]= (c0 + c1) >> R_SHIFT;
row[1]= (c2 + c3) >> R_SHIFT;
row[2]= (c2 - c3) >> R_SHIFT;
row[3]= (c0 - c1) >> R_SHIFT;
}
void ff_simple_idct84_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
int i;
for(i=0; i<4; i++) {
idctRowCondDC_int16_8bit(block + i*8, 0);
}
for(i=0;i<8;i++) {
idct4col_add(dest + i, line_size, block + i);
}
}
void ff_simple_idct48_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
int i;
for(i=0; i<8; i++) {
idct4row(block + i*8);
}
for(i=0; i<4; i++){
idctSparseColAdd_int16_8bit(dest + i, line_size, block + i);
}
}
void ff_simple_idct44_add(uint8_t *dest, ptrdiff_t line_size, int16_t *block)
{
int i;
for(i=0; i<4; i++) {
idct4row(block + i*8);
}
for(i=0; i<4; i++){
idct4col_add(dest + i, line_size, block + i);
}
}
void ff_prores_idct_10(int16_t *block, const int16_t *qmat)
{
int i;
for (i = 0; i < 64; i++)
block[i] *= qmat[i];
for (i = 0; i < 8; i++)
idctRowCondDC_extrashift_10(block + i*8, 2);
for (i = 0; i < 8; i++) {
block[i] += 8192;
idctSparseCol_extrashift_10(block + i);
}
}
void ff_prores_idct_12(int16_t *block, const int16_t *qmat)
{
int i;
for (i = 0; i < 64; i++)
block[i] *= qmat[i];
for (i = 0; i < 8; i++)
idctRowCondDC_int16_12bit(block + i*8, 0);
for (i = 0; i < 8; i++) {
block[i] += 8192;
idctSparseCol_int16_12bit(block + i);
}
}
