































#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "libavutil/common.h"
#include "libavutil/intmath.h"
#include "libavutil/intreadwrite.h"
#include "sbc.h"
#include "sbcdsp.h"
#include "sbcdsp_data.h"




















static av_always_inline void sbc_analyze_simd(const int16_t *in, int32_t *out,
const int16_t *consts,
unsigned subbands)
{
int32_t t1[8];
int16_t t2[8];
int i, j, hop = 0;


for (i = 0; i < subbands; i++)
t1[i] = 1 << (SBC_PROTO_FIXED_SCALE - 1);


for (hop = 0; hop < 10*subbands; hop += 2*subbands)
for (i = 0; i < 2*subbands; i++)
t1[i >> 1] += in[hop + i] * consts[hop + i];


for (i = 0; i < subbands; i++)
t2[i] = t1[i] >> SBC_PROTO_FIXED_SCALE;

memset(t1, 0, sizeof(t1));


for (i = 0; i < subbands/2; i++)
for (j = 0; j < 2*subbands; j++)
t1[j>>1] += t2[i * 2 + (j&1)] * consts[10*subbands + i*2*subbands + j];

for (i = 0; i < subbands; i++)
out[i] = t1[i] >> (SBC_COS_TABLE_FIXED_SCALE - SCALE_OUT_BITS);
}

static void sbc_analyze_4_simd(const int16_t *in, int32_t *out,
const int16_t *consts)
{
sbc_analyze_simd(in, out, consts, 4);
}

static void sbc_analyze_8_simd(const int16_t *in, int32_t *out,
const int16_t *consts)
{
sbc_analyze_simd(in, out, consts, 8);
}

static inline void sbc_analyze_4b_4s_simd(SBCDSPContext *s,
int16_t *x, int32_t *out, int out_stride)
{

s->sbc_analyze_4(x + 12, out, ff_sbcdsp_analysis_consts_fixed4_simd_odd);
out += out_stride;
s->sbc_analyze_4(x + 8, out, ff_sbcdsp_analysis_consts_fixed4_simd_even);
out += out_stride;
s->sbc_analyze_4(x + 4, out, ff_sbcdsp_analysis_consts_fixed4_simd_odd);
out += out_stride;
s->sbc_analyze_4(x + 0, out, ff_sbcdsp_analysis_consts_fixed4_simd_even);
}

static inline void sbc_analyze_4b_8s_simd(SBCDSPContext *s,
int16_t *x, int32_t *out, int out_stride)
{

s->sbc_analyze_8(x + 24, out, ff_sbcdsp_analysis_consts_fixed8_simd_odd);
out += out_stride;
s->sbc_analyze_8(x + 16, out, ff_sbcdsp_analysis_consts_fixed8_simd_even);
out += out_stride;
s->sbc_analyze_8(x + 8, out, ff_sbcdsp_analysis_consts_fixed8_simd_odd);
out += out_stride;
s->sbc_analyze_8(x + 0, out, ff_sbcdsp_analysis_consts_fixed8_simd_even);
}

static inline void sbc_analyze_1b_8s_simd_even(SBCDSPContext *s,
int16_t *x, int32_t *out,
int out_stride);

static inline void sbc_analyze_1b_8s_simd_odd(SBCDSPContext *s,
int16_t *x, int32_t *out,
int out_stride)
{
s->sbc_analyze_8(x, out, ff_sbcdsp_analysis_consts_fixed8_simd_odd);
s->sbc_analyze_8s = sbc_analyze_1b_8s_simd_even;
}

static inline void sbc_analyze_1b_8s_simd_even(SBCDSPContext *s,
int16_t *x, int32_t *out,
int out_stride)
{
s->sbc_analyze_8(x, out, ff_sbcdsp_analysis_consts_fixed8_simd_even);
s->sbc_analyze_8s = sbc_analyze_1b_8s_simd_odd;
}













static int sbc_enc_process_input_4s(int position, const uint8_t *pcm,
int16_t X[2][SBC_X_BUFFER_SIZE],
int nsamples, int nchannels)
{
int c;


if (position < nsamples) {
for (c = 0; c < nchannels; c++)
memcpy(&X[c][SBC_X_BUFFER_SIZE - 40], &X[c][position],
36 * sizeof(int16_t));
position = SBC_X_BUFFER_SIZE - 40;
}


for (; nsamples >= 8; nsamples -= 8, pcm += 16 * nchannels) {
position -= 8;
for (c = 0; c < nchannels; c++) {
int16_t *x = &X[c][position];
x[0] = AV_RN16(pcm + 14*nchannels + 2*c);
x[1] = AV_RN16(pcm + 6*nchannels + 2*c);
x[2] = AV_RN16(pcm + 12*nchannels + 2*c);
x[3] = AV_RN16(pcm + 8*nchannels + 2*c);
x[4] = AV_RN16(pcm + 0*nchannels + 2*c);
x[5] = AV_RN16(pcm + 4*nchannels + 2*c);
x[6] = AV_RN16(pcm + 2*nchannels + 2*c);
x[7] = AV_RN16(pcm + 10*nchannels + 2*c);
}
}

return position;
}

static int sbc_enc_process_input_8s(int position, const uint8_t *pcm,
int16_t X[2][SBC_X_BUFFER_SIZE],
int nsamples, int nchannels)
{
int c;


if (position < nsamples) {
for (c = 0; c < nchannels; c++)
memcpy(&X[c][SBC_X_BUFFER_SIZE - 72], &X[c][position],
72 * sizeof(int16_t));
position = SBC_X_BUFFER_SIZE - 72;
}

if (position % 16 == 8) {
position -= 8;
nsamples -= 8;
for (c = 0; c < nchannels; c++) {
int16_t *x = &X[c][position];
x[0] = AV_RN16(pcm + 14*nchannels + 2*c);
x[2] = AV_RN16(pcm + 12*nchannels + 2*c);
x[3] = AV_RN16(pcm + 0*nchannels + 2*c);
x[4] = AV_RN16(pcm + 10*nchannels + 2*c);
x[5] = AV_RN16(pcm + 2*nchannels + 2*c);
x[6] = AV_RN16(pcm + 8*nchannels + 2*c);
x[7] = AV_RN16(pcm + 4*nchannels + 2*c);
x[8] = AV_RN16(pcm + 6*nchannels + 2*c);
}
pcm += 16 * nchannels;
}


for (; nsamples >= 16; nsamples -= 16, pcm += 32 * nchannels) {
position -= 16;
for (c = 0; c < nchannels; c++) {
int16_t *x = &X[c][position];
x[0] = AV_RN16(pcm + 30*nchannels + 2*c);
x[1] = AV_RN16(pcm + 14*nchannels + 2*c);
x[2] = AV_RN16(pcm + 28*nchannels + 2*c);
x[3] = AV_RN16(pcm + 16*nchannels + 2*c);
x[4] = AV_RN16(pcm + 26*nchannels + 2*c);
x[5] = AV_RN16(pcm + 18*nchannels + 2*c);
x[6] = AV_RN16(pcm + 24*nchannels + 2*c);
x[7] = AV_RN16(pcm + 20*nchannels + 2*c);
x[8] = AV_RN16(pcm + 22*nchannels + 2*c);
x[9] = AV_RN16(pcm + 6*nchannels + 2*c);
x[10] = AV_RN16(pcm + 12*nchannels + 2*c);
x[11] = AV_RN16(pcm + 0*nchannels + 2*c);
x[12] = AV_RN16(pcm + 10*nchannels + 2*c);
x[13] = AV_RN16(pcm + 2*nchannels + 2*c);
x[14] = AV_RN16(pcm + 8*nchannels + 2*c);
x[15] = AV_RN16(pcm + 4*nchannels + 2*c);
}
}

if (nsamples == 8) {
position -= 8;
for (c = 0; c < nchannels; c++) {
int16_t *x = &X[c][position];
x[-7] = AV_RN16(pcm + 14*nchannels + 2*c);
x[1] = AV_RN16(pcm + 6*nchannels + 2*c);
x[2] = AV_RN16(pcm + 12*nchannels + 2*c);
x[3] = AV_RN16(pcm + 0*nchannels + 2*c);
x[4] = AV_RN16(pcm + 10*nchannels + 2*c);
x[5] = AV_RN16(pcm + 2*nchannels + 2*c);
x[6] = AV_RN16(pcm + 8*nchannels + 2*c);
x[7] = AV_RN16(pcm + 4*nchannels + 2*c);
}
}

return position;
}

static void sbc_calc_scalefactors(int32_t sb_sample_f[16][2][8],
uint32_t scale_factor[2][8],
int blocks, int channels, int subbands)
{
int ch, sb, blk;
for (ch = 0; ch < channels; ch++) {
for (sb = 0; sb < subbands; sb++) {
uint32_t x = 1 << SCALE_OUT_BITS;
for (blk = 0; blk < blocks; blk++) {
int32_t tmp = FFABS(sb_sample_f[blk][ch][sb]);
if (tmp != 0)
x |= tmp - 1;
}
scale_factor[ch][sb] = (31 - SCALE_OUT_BITS) - ff_clz(x);
}
}
}

static int sbc_calc_scalefactors_j(int32_t sb_sample_f[16][2][8],
uint32_t scale_factor[2][8],
int blocks, int subbands)
{
int blk, joint = 0;
int32_t tmp0, tmp1;
uint32_t x, y;


int sb = subbands - 1;
x = 1 << SCALE_OUT_BITS;
y = 1 << SCALE_OUT_BITS;
for (blk = 0; blk < blocks; blk++) {
tmp0 = FFABS(sb_sample_f[blk][0][sb]);
tmp1 = FFABS(sb_sample_f[blk][1][sb]);
if (tmp0 != 0)
x |= tmp0 - 1;
if (tmp1 != 0)
y |= tmp1 - 1;
}
scale_factor[0][sb] = (31 - SCALE_OUT_BITS) - ff_clz(x);
scale_factor[1][sb] = (31 - SCALE_OUT_BITS) - ff_clz(y);


while (--sb >= 0) {
int32_t sb_sample_j[16][2];
x = 1 << SCALE_OUT_BITS;
y = 1 << SCALE_OUT_BITS;
for (blk = 0; blk < blocks; blk++) {
tmp0 = sb_sample_f[blk][0][sb];
tmp1 = sb_sample_f[blk][1][sb];
sb_sample_j[blk][0] = (tmp0 >> 1) + (tmp1 >> 1);
sb_sample_j[blk][1] = (tmp0 >> 1) - (tmp1 >> 1);
tmp0 = FFABS(tmp0);
tmp1 = FFABS(tmp1);
if (tmp0 != 0)
x |= tmp0 - 1;
if (tmp1 != 0)
y |= tmp1 - 1;
}
scale_factor[0][sb] = (31 - SCALE_OUT_BITS) -
ff_clz(x);
scale_factor[1][sb] = (31 - SCALE_OUT_BITS) -
ff_clz(y);
x = 1 << SCALE_OUT_BITS;
y = 1 << SCALE_OUT_BITS;
for (blk = 0; blk < blocks; blk++) {
tmp0 = FFABS(sb_sample_j[blk][0]);
tmp1 = FFABS(sb_sample_j[blk][1]);
if (tmp0 != 0)
x |= tmp0 - 1;
if (tmp1 != 0)
y |= tmp1 - 1;
}
x = (31 - SCALE_OUT_BITS) - ff_clz(x);
y = (31 - SCALE_OUT_BITS) - ff_clz(y);


if ((scale_factor[0][sb] + scale_factor[1][sb]) > x + y) {
joint |= 1 << (subbands - 1 - sb);
scale_factor[0][sb] = x;
scale_factor[1][sb] = y;
for (blk = 0; blk < blocks; blk++) {
sb_sample_f[blk][0][sb] = sb_sample_j[blk][0];
sb_sample_f[blk][1][sb] = sb_sample_j[blk][1];
}
}
}


return joint;
}




av_cold void ff_sbcdsp_init(SBCDSPContext *s)
{

s->sbc_analyze_4 = sbc_analyze_4_simd;
s->sbc_analyze_8 = sbc_analyze_8_simd;
s->sbc_analyze_4s = sbc_analyze_4b_4s_simd;
if (s->increment == 1)
s->sbc_analyze_8s = sbc_analyze_1b_8s_simd_odd;
else
s->sbc_analyze_8s = sbc_analyze_4b_8s_simd;


s->sbc_enc_process_input_4s = sbc_enc_process_input_4s;
s->sbc_enc_process_input_8s = sbc_enc_process_input_8s;


s->sbc_calc_scalefactors = sbc_calc_scalefactors;
s->sbc_calc_scalefactors_j = sbc_calc_scalefactors_j;

if (ARCH_ARM)
ff_sbcdsp_init_arm(s);
if (ARCH_X86)
ff_sbcdsp_init_x86(s);
}
