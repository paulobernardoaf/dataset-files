#include "rv34dsp.h"
#include "libavutil/common.h"
static av_always_inline void rv34_row_transform(int temp[16], int16_t *block)
{
int i;
for(i = 0; i < 4; i++){
const int z0 = 13*(block[i+4*0] + block[i+4*2]);
const int z1 = 13*(block[i+4*0] - block[i+4*2]);
const int z2 = 7* block[i+4*1] - 17*block[i+4*3];
const int z3 = 17* block[i+4*1] + 7*block[i+4*3];
temp[4*i+0] = z0 + z3;
temp[4*i+1] = z1 + z2;
temp[4*i+2] = z1 - z2;
temp[4*i+3] = z0 - z3;
}
}
static void rv34_idct_add_c(uint8_t *dst, ptrdiff_t stride, int16_t *block){
int temp[16];
int i;
rv34_row_transform(temp, block);
memset(block, 0, 16*sizeof(int16_t));
for(i = 0; i < 4; i++){
const int z0 = 13*(temp[4*0+i] + temp[4*2+i]) + 0x200;
const int z1 = 13*(temp[4*0+i] - temp[4*2+i]) + 0x200;
const int z2 = 7* temp[4*1+i] - 17*temp[4*3+i];
const int z3 = 17* temp[4*1+i] + 7*temp[4*3+i];
dst[0] = av_clip_uint8( dst[0] + ( (z0 + z3) >> 10 ) );
dst[1] = av_clip_uint8( dst[1] + ( (z1 + z2) >> 10 ) );
dst[2] = av_clip_uint8( dst[2] + ( (z1 - z2) >> 10 ) );
dst[3] = av_clip_uint8( dst[3] + ( (z0 - z3) >> 10 ) );
dst += stride;
}
}
static void rv34_inv_transform_noround_c(int16_t *block){
int temp[16];
int i;
rv34_row_transform(temp, block);
for(i = 0; i < 4; i++){
const int z0 = 39*(temp[4*0+i] + temp[4*2+i]);
const int z1 = 39*(temp[4*0+i] - temp[4*2+i]);
const int z2 = 21* temp[4*1+i] - 51*temp[4*3+i];
const int z3 = 51* temp[4*1+i] + 21*temp[4*3+i];
block[i*4+0] = (z0 + z3) >> 11;
block[i*4+1] = (z1 + z2) >> 11;
block[i*4+2] = (z1 - z2) >> 11;
block[i*4+3] = (z0 - z3) >> 11;
}
}
static void rv34_idct_dc_add_c(uint8_t *dst, ptrdiff_t stride, int dc)
{
int i, j;
dc = (13*13*dc + 0x200) >> 10;
for (i = 0; i < 4; i++)
{
for (j = 0; j < 4; j++)
dst[j] = av_clip_uint8( dst[j] + dc );
dst += stride;
}
}
static void rv34_inv_transform_dc_noround_c(int16_t *block)
{
int16_t dc = (13 * 13 * 3 * block[0]) >> 11;
int i, j;
for (i = 0; i < 4; i++, block += 4)
for (j = 0; j < 4; j++)
block[j] = dc;
}
av_cold void ff_rv34dsp_init(RV34DSPContext *c)
{
c->rv34_inv_transform = rv34_inv_transform_noround_c;
c->rv34_inv_transform_dc = rv34_inv_transform_dc_noround_c;
c->rv34_idct_add = rv34_idct_add_c;
c->rv34_idct_dc_add = rv34_idct_dc_add_c;
if (ARCH_ARM)
ff_rv34dsp_init_arm(c);
if (ARCH_X86)
ff_rv34dsp_init_x86(c);
}
