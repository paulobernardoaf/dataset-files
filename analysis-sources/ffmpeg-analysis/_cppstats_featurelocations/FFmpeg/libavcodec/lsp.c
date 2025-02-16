






















#include <inttypes.h>

#include "avcodec.h"
#define FRAC_BITS 14
#include "mathops.h"
#include "lsp.h"
#include "libavcodec/mips/lsp_mips.h"
#include "libavutil/avassert.h"

void ff_acelp_reorder_lsf(int16_t* lsfq, int lsfq_min_distance, int lsfq_min, int lsfq_max, int lp_order)
{
int i, j;



for(i=0; i<lp_order-1; i++)
for(j=i; j>=0 && lsfq[j] > lsfq[j+1]; j--)
FFSWAP(int16_t, lsfq[j], lsfq[j+1]);

for(i=0; i<lp_order; i++)
{
lsfq[i] = FFMAX(lsfq[i], lsfq_min);
lsfq_min = lsfq[i] + lsfq_min_distance;
}
lsfq[lp_order-1] = FFMIN(lsfq[lp_order-1], lsfq_max);
}

void ff_set_min_dist_lsf(float *lsf, double min_spacing, int size)
{
int i;
float prev = 0.0;
for (i = 0; i < size; i++)
prev = lsf[i] = FFMAX(lsf[i], prev + min_spacing);
}



static const int16_t tab_cos[65] =
{
32767, 32738, 32617, 32421, 32145, 31793, 31364, 30860,
30280, 29629, 28905, 28113, 27252, 26326, 25336, 24285,
23176, 22011, 20793, 19525, 18210, 16851, 15451, 14014,
12543, 11043, 9515, 7965, 6395, 4810, 3214, 1609,
1, -1607, -3211, -4808, -6393, -7962, -9513, -11040,
-12541, -14012, -15449, -16848, -18207, -19523, -20791, -22009,
-23174, -24283, -25334, -26324, -27250, -28111, -28904, -29627,
-30279, -30858, -31363, -31792, -32144, -32419, -32616, -32736, -32768,
};

static int16_t ff_cos(uint16_t arg)
{
uint8_t offset= arg;
uint8_t ind = arg >> 8;

av_assert2(arg <= 0x3fff);

return tab_cos[ind] + (offset * (tab_cos[ind+1] - tab_cos[ind]) >> 8);
}

void ff_acelp_lsf2lsp(int16_t *lsp, const int16_t *lsf, int lp_order)
{
int i;


for(i=0; i<lp_order; i++)

lsp[i] = ff_cos(lsf[i] * 20861 >> 15); 
}

void ff_acelp_lsf2lspd(double *lsp, const float *lsf, int lp_order)
{
int i;

for(i = 0; i < lp_order; i++)
lsp[i] = cos(2.0 * M_PI * lsf[i]);
}






static void lsp2poly(int* f, const int16_t* lsp, int lp_half_order)
{
int i, j;

f[0] = 0x400000; 
f[1] = -lsp[0] * 256; 

for(i=2; i<=lp_half_order; i++)
{
f[i] = f[i-2];
for(j=i; j>1; j--)
f[j] -= MULL(f[j-1], lsp[2*i-2], FRAC_BITS) - f[j-2];

f[1] -= lsp[2*i-2] * 256;
}
}

void ff_acelp_lsp2lpc(int16_t* lp, const int16_t* lsp, int lp_half_order)
{
int i;
int f1[MAX_LP_HALF_ORDER+1]; 
int f2[MAX_LP_HALF_ORDER+1]; 

lsp2poly(f1, lsp , lp_half_order);
lsp2poly(f2, lsp+1, lp_half_order);


lp[0] = 4096;
for(i=1; i<lp_half_order+1; i++)
{
int ff1 = f1[i] + f1[i-1]; 
int ff2 = f2[i] - f2[i-1]; 

ff1 += 1 << 10; 
lp[i] = (ff1 + ff2) >> 11; 
lp[(lp_half_order << 1) + 1 - i] = (ff1 - ff2) >> 11; 
}
}

void ff_amrwb_lsp2lpc(const double *lsp, float *lp, int lp_order)
{
int lp_half_order = lp_order >> 1;
double buf[MAX_LP_HALF_ORDER + 1];
double pa[MAX_LP_HALF_ORDER + 1];
double *qa = buf + 1;
int i,j;

qa[-1] = 0.0;

ff_lsp2polyf(lsp , pa, lp_half_order );
ff_lsp2polyf(lsp + 1, qa, lp_half_order - 1);

for (i = 1, j = lp_order - 1; i < lp_half_order; i++, j--) {
double paf = pa[i] * (1 + lsp[lp_order - 1]);
double qaf = (qa[i] - qa[i-2]) * (1 - lsp[lp_order - 1]);
lp[i-1] = (paf + qaf) * 0.5;
lp[j-1] = (paf - qaf) * 0.5;
}

lp[lp_half_order - 1] = (1.0 + lsp[lp_order - 1]) *
pa[lp_half_order] * 0.5;

lp[lp_order - 1] = lsp[lp_order - 1];
}

void ff_acelp_lp_decode(int16_t* lp_1st, int16_t* lp_2nd, const int16_t* lsp_2nd, const int16_t* lsp_prev, int lp_order)
{
int16_t lsp_1st[MAX_LP_ORDER]; 
int i;


for(i=0; i<lp_order; i++)
#if defined(G729_BITEXACT)
lsp_1st[i] = (lsp_2nd[i] >> 1) + (lsp_prev[i] >> 1);
#else
lsp_1st[i] = (lsp_2nd[i] + lsp_prev[i]) >> 1;
#endif

ff_acelp_lsp2lpc(lp_1st, lsp_1st, lp_order >> 1);


ff_acelp_lsp2lpc(lp_2nd, lsp_2nd, lp_order >> 1);
}

#if !defined(ff_lsp2polyf)
void ff_lsp2polyf(const double *lsp, double *f, int lp_half_order)
{
int i, j;

f[0] = 1.0;
f[1] = -2 * lsp[0];
lsp -= 2;
for(i=2; i<=lp_half_order; i++)
{
double val = -2 * lsp[2*i];
f[i] = val * f[i-1] + 2*f[i-2];
for(j=i-1; j>1; j--)
f[j] += f[j-1] * val + f[j-2];
f[1] += val;
}
}
#endif 

void ff_acelp_lspd2lpc(const double *lsp, float *lpc, int lp_half_order)
{
double pa[MAX_LP_HALF_ORDER+1], qa[MAX_LP_HALF_ORDER+1];
float *lpc2 = lpc + (lp_half_order << 1) - 1;

av_assert2(lp_half_order <= MAX_LP_HALF_ORDER);

ff_lsp2polyf(lsp, pa, lp_half_order);
ff_lsp2polyf(lsp + 1, qa, lp_half_order);

while (lp_half_order--) {
double paf = pa[lp_half_order+1] + pa[lp_half_order];
double qaf = qa[lp_half_order+1] - qa[lp_half_order];

lpc [ lp_half_order] = 0.5*(paf+qaf);
lpc2[-lp_half_order] = 0.5*(paf-qaf);
}
}

void ff_sort_nearly_sorted_floats(float *vals, int len)
{
int i,j;

for (i = 0; i < len - 1; i++)
for (j = i; j >= 0 && vals[j] > vals[j+1]; j--)
FFSWAP(float, vals[j], vals[j+1]);
}
