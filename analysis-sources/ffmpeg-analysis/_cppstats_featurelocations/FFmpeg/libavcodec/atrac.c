


























#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "avcodec.h"
#include "atrac.h"

float ff_atrac_sf_table[64];
static float qmf_window[48];

static const float qmf_48tap_half[24] = {
-0.00001461907, -0.00009205479,-0.000056157569,0.00030117269,
0.0002422519, -0.00085293897,-0.0005205574, 0.0020340169,
0.00078333891, -0.0042153862, -0.00075614988, 0.0078402944,
-0.000061169922,-0.01344162, 0.0024626821, 0.021736089,
-0.007801671, -0.034090221, 0.01880949, 0.054326009,
-0.043596379, -0.099384367, 0.13207909, 0.46424159
};

av_cold void ff_atrac_generate_tables(void)
{
int i;
float s;


if (!ff_atrac_sf_table[63])
for (i=0 ; i<64 ; i++)
ff_atrac_sf_table[i] = pow(2.0, (i - 15) / 3.0);


if (!qmf_window[47])
for (i=0 ; i<24; i++) {
s = qmf_48tap_half[i] * 2.0;
qmf_window[i] = qmf_window[47 - i] = s;
}
}

av_cold void ff_atrac_init_gain_compensation(AtracGCContext *gctx, int id2exp_offset,
int loc_scale)
{
int i;

gctx->loc_scale = loc_scale;
gctx->loc_size = 1 << loc_scale;
gctx->id2exp_offset = id2exp_offset;


for (i = 0; i < 16; i++)
gctx->gain_tab1[i] = powf(2.0, id2exp_offset - i);


for (i = -15; i < 16; i++)
gctx->gain_tab2[i + 15] = powf(2.0, -1.0f / gctx->loc_size * i);
}

void ff_atrac_gain_compensation(AtracGCContext *gctx, float *in, float *prev,
AtracGainInfo *gc_now, AtracGainInfo *gc_next,
int num_samples, float *out)
{
float lev, gc_scale, gain_inc;
int i, pos, lastpos;

gc_scale = gc_next->num_points ? gctx->gain_tab1[gc_next->lev_code[0]]
: 1.0f;

if (!gc_now->num_points) {
for (pos = 0; pos < num_samples; pos++)
out[pos] = in[pos] * gc_scale + prev[pos];
} else {
pos = 0;

for (i = 0; i < gc_now->num_points; i++) {
lastpos = gc_now->loc_code[i] << gctx->loc_scale;

lev = gctx->gain_tab1[gc_now->lev_code[i]];
gain_inc = gctx->gain_tab2[(i + 1 < gc_now->num_points ? gc_now->lev_code[i + 1]
: gctx->id2exp_offset) -
gc_now->lev_code[i] + 15];


for (; pos < lastpos; pos++)
out[pos] = (in[pos] * gc_scale + prev[pos]) * lev;


for (; pos < lastpos + gctx->loc_size; pos++) {
out[pos] = (in[pos] * gc_scale + prev[pos]) * lev;
lev *= gain_inc;
}
}

for (; pos < num_samples; pos++)
out[pos] = in[pos] * gc_scale + prev[pos];
}


memcpy(prev, &in[num_samples], num_samples * sizeof(float));
}

void ff_atrac_iqmf(float *inlo, float *inhi, unsigned int nIn, float *pOut,
float *delayBuf, float *temp)
{
int i, j;
float *p1, *p3;

memcpy(temp, delayBuf, 46*sizeof(float));

p3 = temp + 46;


for(i=0; i<nIn; i+=2){
p3[2*i+0] = inlo[i ] + inhi[i ];
p3[2*i+1] = inlo[i ] - inhi[i ];
p3[2*i+2] = inlo[i+1] + inhi[i+1];
p3[2*i+3] = inlo[i+1] - inhi[i+1];
}


p1 = temp;
for (j = nIn; j != 0; j--) {
float s1 = 0.0;
float s2 = 0.0;

for (i = 0; i < 48; i += 2) {
s1 += p1[i] * qmf_window[i];
s2 += p1[i+1] * qmf_window[i+1];
}

pOut[0] = s2;
pOut[1] = s1;

p1 += 2;
pOut += 2;
}


memcpy(delayBuf, temp + nIn*2, 46*sizeof(float));
}
