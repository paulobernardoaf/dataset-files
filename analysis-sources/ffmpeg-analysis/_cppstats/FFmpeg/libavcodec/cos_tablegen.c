#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libavutil/mathematics.h"
#define BITS 17
#define FLOATFMT "%.18e"
#define FIXEDFMT "%6d"
static int clip_f15(int v)
{
return v < -32767 ? -32767 :
v > 32767 ? 32767 :
v;
}
static void printval(double val, int fixed)
{
if (fixed) {
double new_val = val * (double) (1 << 15);
new_val = new_val >= 0 ? floor(new_val + 0.5) : ceil(new_val - 0.5);
printf(" "FIXEDFMT",", clip_f15((long int) new_val));
} else {
printf(" "FLOATFMT",", val);
}
}
int main(int argc, char *argv[])
{
int i, j;
int do_sin = argc > 1 && !strcmp(argv[1], "sin");
int fixed = argc > 1 && strstr(argv[1], "fixed");
double (*func)(double) = do_sin ? sin : cos;
printf("/* This file was automatically generated. */\n");
printf("#define FFT_FLOAT %d\n", !fixed);
printf("#include \"libavcodec/%s\"\n", do_sin ? "rdft.h" : "fft.h");
for (i = 4; i <= BITS; i++) {
int m = 1 << i;
double freq = 2*M_PI/m;
printf("%s(%i) = {\n ", do_sin ? "SINTABLE" : "COSTABLE", m);
for (j = 0; j < m/2 - 1; j++) {
int idx = j > m/4 ? m/2 - j : j;
if (do_sin && j >= m/4)
idx = m/4 - j;
printval(func(idx*freq), fixed);
if ((j & 3) == 3)
printf("\n ");
}
printval(func(do_sin ? -(m/4 - 1)*freq : freq), fixed);
printf("\n};\n");
}
return 0;
}
