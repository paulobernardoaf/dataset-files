#include "libavutil/mathematics.h"
#include "dctref.h"
static double coefficients[8 * 8];
av_cold void ff_ref_dct_init(void)
{
unsigned int i, j;
for (j = 0; j < 8; ++j) {
coefficients[j] = sqrt(0.125);
for (i = 8; i < 64; i += 8) {
coefficients[i + j] = 0.5 * cos(i * (j + 0.5) * M_PI / 64.0);
}
}
}
void ff_ref_fdct(short *block)
{
unsigned int i, j, k;
double out[8 * 8];
for (i = 0; i < 64; i += 8) {
for (j = 0; j < 8; ++j) {
double tmp = 0;
for (k = 0; k < 8; ++k) {
tmp += coefficients[i + k] * block[k * 8 + j];
}
out[i + j] = tmp * 8;
}
}
for (j = 0; j < 8; ++j) {
for (i = 0; i < 64; i += 8) {
double tmp = 0;
for (k = 0; k < 8; ++k) {
tmp += out[i + k] * coefficients[j * 8 + k];
}
block[i + j] = floor(tmp + 0.499999999999);
}
}
}
void ff_ref_idct(short *block)
{
unsigned int i, j, k;
double out[8 * 8];
for (i = 0; i < 64; i += 8) {
for (j = 0; j < 8; ++j) {
double tmp = 0;
for (k = 0; k < 8; ++k) {
tmp += block[i + k] * coefficients[k * 8 + j];
}
out[i + j] = tmp;
}
}
for (i = 0; i < 8; ++i) {
for (j = 0; j < 8; ++j) {
double tmp = 0;
for (k = 0; k < 64; k += 8) {
tmp += coefficients[k + i] * out[k + j];
}
block[i * 8 + j] = floor(tmp + 0.5);
}
}
}
