


























#include "internal.h"
#include "jpegls.h"

void ff_jpegls_init_state(JLSState *state)
{
int i;

state->twonear = state->near * 2 + 1;
state->range = (state->maxval + state->twonear - 1) / state->twonear + 1;


for (state->qbpp = 0; (1 << state->qbpp) < state->range; state->qbpp++)
;

state->bpp = FFMAX(av_log2(state->maxval) + 1, 2);
state->limit = 2*(state->bpp + FFMAX(state->bpp, 8)) - state->qbpp;

for (i = 0; i < 367; i++) {
state->A[i] = FFMAX(state->range + 32 >> 6, 2);
state->N[i] = 1;
}
}




static inline int iso_clip(int v, int vmin, int vmax)
{
if (v > vmax || v < vmin)
return vmin;
else
return v;
}

void ff_jpegls_reset_coding_parameters(JLSState *s, int reset_all)
{
const int basic_t1 = 3;
const int basic_t2 = 7;
const int basic_t3 = 21;
int factor;

if (s->maxval == 0 || reset_all)
s->maxval = (1 << s->bpp) - 1;

if (s->maxval >= 128) {
factor = FFMIN(s->maxval, 4095) + 128 >> 8;

if (s->T1 == 0 || reset_all)
s->T1 = iso_clip(factor * (basic_t1 - 2) + 2 + 3 * s->near,
s->near + 1, s->maxval);
if (s->T2 == 0 || reset_all)
s->T2 = iso_clip(factor * (basic_t2 - 3) + 3 + 5 * s->near,
s->T1, s->maxval);
if (s->T3 == 0 || reset_all)
s->T3 = iso_clip(factor * (basic_t3 - 4) + 4 + 7 * s->near,
s->T2, s->maxval);
} else {
factor = 256 / (s->maxval + 1);

if (s->T1 == 0 || reset_all)
s->T1 = iso_clip(FFMAX(2, basic_t1 / factor + 3 * s->near),
s->near + 1, s->maxval);
if (s->T2 == 0 || reset_all)
s->T2 = iso_clip(FFMAX(3, basic_t2 / factor + 5 * s->near),
s->T1, s->maxval);
if (s->T3 == 0 || reset_all)
s->T3 = iso_clip(FFMAX(4, basic_t3 / factor + 7 * s->near),
s->T2, s->maxval);
}

if (s->reset == 0 || reset_all)
s->reset = 64;
ff_dlog(NULL, "[JPEG-LS RESET] T=%i,%i,%i\n", s->T1, s->T2, s->T3);
}
