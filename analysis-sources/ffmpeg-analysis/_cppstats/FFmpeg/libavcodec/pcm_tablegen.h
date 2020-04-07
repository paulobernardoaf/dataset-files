#include <stdint.h>
#include "libavutil/attributes.h"
#define SIGN_BIT (0x80) 
#define QUANT_MASK (0xf) 
#define NSEGS (8) 
#define SEG_SHIFT (4) 
#define SEG_MASK (0x70) 
#define BIAS (0x84) 
#define VIDC_SIGN_BIT (1)
#define VIDC_QUANT_MASK (0x1E)
#define VIDC_QUANT_SHIFT (1)
#define VIDC_SEG_SHIFT (5)
#define VIDC_SEG_MASK (0xE0)
static av_cold int alaw2linear(unsigned char a_val)
{
int t;
int seg;
a_val ^= 0x55;
t = a_val & QUANT_MASK;
seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
if(seg) t= (t + t + 1 + 32) << (seg + 2);
else t= (t + t + 1 ) << 3;
return (a_val & SIGN_BIT) ? t : -t;
}
static av_cold int ulaw2linear(unsigned char u_val)
{
int t;
u_val = ~u_val;
t = ((u_val & QUANT_MASK) << 3) + BIAS;
t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
return (u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS);
}
static av_cold int vidc2linear(unsigned char u_val)
{
int t;
t = (((u_val & VIDC_QUANT_MASK) >> VIDC_QUANT_SHIFT) << 3) + BIAS;
t <<= ((unsigned)u_val & VIDC_SEG_MASK) >> VIDC_SEG_SHIFT;
return (u_val & VIDC_SIGN_BIT) ? (BIAS - t) : (t - BIAS);
}
#if CONFIG_HARDCODED_TABLES
#define pcm_alaw_tableinit()
#define pcm_ulaw_tableinit()
#define pcm_vidc_tableinit()
#include "libavcodec/pcm_tables.h"
#else
static uint8_t linear_to_alaw[16384];
static uint8_t linear_to_ulaw[16384];
static uint8_t linear_to_vidc[16384];
static av_cold void build_xlaw_table(uint8_t *linear_to_xlaw,
int (*xlaw2linear)(unsigned char),
int mask)
{
int i, j, v, v1, v2;
j = 1;
linear_to_xlaw[8192] = mask;
for(i=0;i<127;i++) {
v1 = xlaw2linear(i ^ mask);
v2 = xlaw2linear((i + 1) ^ mask);
v = (v1 + v2 + 4) >> 3;
for(;j<v;j+=1) {
linear_to_xlaw[8192 - j] = (i ^ (mask ^ 0x80));
linear_to_xlaw[8192 + j] = (i ^ mask);
}
}
for(;j<8192;j++) {
linear_to_xlaw[8192 - j] = (127 ^ (mask ^ 0x80));
linear_to_xlaw[8192 + j] = (127 ^ mask);
}
linear_to_xlaw[0] = linear_to_xlaw[1];
}
static void pcm_alaw_tableinit(void)
{
build_xlaw_table(linear_to_alaw, alaw2linear, 0xd5);
}
static void pcm_ulaw_tableinit(void)
{
build_xlaw_table(linear_to_ulaw, ulaw2linear, 0xff);
}
static void pcm_vidc_tableinit(void)
{
build_xlaw_table(linear_to_vidc, vidc2linear, 0xff);
}
#endif 
