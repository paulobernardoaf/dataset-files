





















#include <inttypes.h>

#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "avcodec.h"
#include "acelp_filters.h"

const int16_t ff_acelp_interp_filter[61] = { 
29443, 28346, 25207, 20449, 14701, 8693,
3143, -1352, -4402, -5865, -5850, -4673,
-2783, -672, 1211, 2536, 3130, 2991,
2259, 1170, 0, -1001, -1652, -1868,
-1666, -1147, -464, 218, 756, 1060,
1099, 904, 550, 135, -245, -514,
-634, -602, -451, -231, 0, 191,
308, 340, 296, 198, 78, -36,
-120, -163, -165, -132, -79, -19,
34, 73, 91, 89, 70, 38,
0,
};

void ff_acelp_interpolate(int16_t* out, const int16_t* in,
const int16_t* filter_coeffs, int precision,
int frac_pos, int filter_length, int length)
{
int n, i;

av_assert1(frac_pos >= 0 && frac_pos < precision);

for (n = 0; n < length; n++) {
int idx = 0;
int v = 0x4000;

for (i = 0; i < filter_length;) {










v += in[n + i] * filter_coeffs[idx + frac_pos];
idx += precision;
i++;
v += in[n - i] * filter_coeffs[idx - frac_pos];
}
if (av_clip_int16(v >> 15) != (v >> 15))
av_log(NULL, AV_LOG_WARNING, "overflow that would need clipping in ff_acelp_interpolate()\n");
out[n] = v >> 15;
}
}

void ff_acelp_interpolatef(float *out, const float *in,
const float *filter_coeffs, int precision,
int frac_pos, int filter_length, int length)
{
int n, i;

for (n = 0; n < length; n++) {
int idx = 0;
float v = 0;

for (i = 0; i < filter_length;) {
v += in[n + i] * filter_coeffs[idx + frac_pos];
idx += precision;
i++;
v += in[n - i] * filter_coeffs[idx - frac_pos];
}
out[n] = v;
}
}


void ff_acelp_high_pass_filter(int16_t* out, int hpf_f[2],
const int16_t* in, int length)
{
int i;
int tmp;

for (i = 0; i < length; i++) {
tmp = (hpf_f[0]* 15836LL) >> 13;
tmp += (hpf_f[1]* -7667LL) >> 13;
tmp += 7699 * (in[i] - 2*in[i-1] + in[i-2]);



out[i] = av_clip_int16((tmp + 0x800) >> 12);

hpf_f[1] = hpf_f[0];
hpf_f[0] = tmp;
}
}

void ff_acelp_apply_order_2_transfer_function(float *out, const float *in,
const float zero_coeffs[2],
const float pole_coeffs[2],
float gain, float mem[2], int n)
{
int i;
float tmp;

for (i = 0; i < n; i++) {
tmp = gain * in[i] - pole_coeffs[0] * mem[0] - pole_coeffs[1] * mem[1];
out[i] = tmp + zero_coeffs[0] * mem[0] + zero_coeffs[1] * mem[1];

mem[1] = mem[0];
mem[0] = tmp;
}
}

void ff_tilt_compensation(float *mem, float tilt, float *samples, int size)
{
float new_tilt_mem = samples[size - 1];
int i;

for (i = size - 1; i > 0; i--)
samples[i] -= tilt * samples[i - 1];

samples[0] -= tilt * *mem;
*mem = new_tilt_mem;
}

void ff_acelp_filter_init(ACELPFContext *c)
{
c->acelp_interpolatef = ff_acelp_interpolatef;
c->acelp_apply_order_2_transfer_function = ff_acelp_apply_order_2_transfer_function;

if(HAVE_MIPSFPU)
ff_acelp_filter_init_mips(c);
}
