#include <stdio.h>
#include "libavutil/common.h"
#include "libavutil/lfg.h"
#include "libavdevice/timefilter.h"
#define LFG_MAX ((1LL << 32) - 1)
int main(void)
{
AVLFG prng;
double n0, n1;
#define SAMPLES 1000
double ideal[SAMPLES];
double samples[SAMPLES];
double samplet[SAMPLES];
for (n0 = 0; n0 < 40; n0 = 2 * n0 + 1) {
for (n1 = 0; n1 < 10; n1 = 2 * n1 + 1) {
double best_error = 1000000000;
double bestpar0 = n0 ? 1 : 100000;
double bestpar1 = 1;
int better, i;
av_lfg_init(&prng, 123);
for (i = 0; i < SAMPLES; i++) {
samplet[i] = 10 + i + (av_lfg_get(&prng) < LFG_MAX/2 ? 0 : 0.999);
ideal[i] = samplet[i] + n1 * i / (1000);
samples[i] = ideal[i] + n0 * (av_lfg_get(&prng) - LFG_MAX / 2) / (LFG_MAX * 10LL);
if(i && samples[i]<samples[i-1])
samples[i]=samples[i-1]+0.001;
}
do {
double par0, par1;
better = 0;
for (par0 = bestpar0 * 0.8; par0 <= bestpar0 * 1.21; par0 += bestpar0 * 0.05) {
for (par1 = bestpar1 * 0.8; par1 <= bestpar1 * 1.21; par1 += bestpar1 * 0.05) {
double error = 0;
TimeFilter *tf = ff_timefilter_new(1, par0, par1);
if (!tf) {
printf("Could not allocate memory for timefilter.\n");
exit(1);
}
for (i = 0; i < SAMPLES; i++) {
double filtered;
filtered = ff_timefilter_update(tf, samples[i], i ? (samplet[i] - samplet[i-1]) : 1);
if(filtered < 0 || filtered > 1000000000)
printf("filter is unstable\n");
error += (filtered - ideal[i]) * (filtered - ideal[i]);
}
ff_timefilter_destroy(tf);
if (error < best_error) {
best_error = error;
bestpar0 = par0;
bestpar1 = par1;
better = 1;
}
}
}
} while (better);
printf(" [%12f %11f %9f]", bestpar0, bestpar1, best_error);
}
printf("\n");
}
return 0;
}
