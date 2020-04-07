#include <stdlib.h>
#include "fft.h"
#include <math.h>
#if !defined(PI)
#if defined(M_PI)
#define PI M_PI
#else
#define PI 3.14159265358979323846 
#endif
#endif
static void fft_prepare(const sound_sample *input, float * re, float * im,
const unsigned int *bitReverse);
static void fft_calculate(float * re, float * im,
const float *costable, const float *sintable );
static void fft_output(const float *re, const float *im, float *output);
static int reverseBits(unsigned int initial);
fft_state *visual_fft_init(void)
{
fft_state *p_state;
unsigned int i;
p_state = malloc( sizeof(*p_state) );
if(! p_state )
return NULL;
for(i = 0; i < FFT_BUFFER_SIZE; i++)
{
p_state->bitReverse[i] = reverseBits(i);
}
for(i = 0; i < FFT_BUFFER_SIZE / 2; i++)
{
float j = 2 * PI * i / FFT_BUFFER_SIZE;
p_state->costable[i] = cos(j);
p_state->sintable[i] = sin(j);
}
return p_state;
}
void fft_perform(const sound_sample *input, float *output, fft_state *state) {
fft_prepare(input, state->real, state->imag, state->bitReverse );
fft_calculate(state->real, state->imag, state->costable, state->sintable);
fft_output(state->real, state->imag, output);
}
void fft_close(fft_state *state) {
free( state );
}
static void fft_prepare( const sound_sample *input, float * re, float * im,
const unsigned int *bitReverse ) {
unsigned int i;
float *p_real = re;
float *p_imag = im;
for(i = 0; i < FFT_BUFFER_SIZE; i++)
{
*p_real++ = input[bitReverse[i]];
*p_imag++ = 0;
}
}
static void fft_output(const float * re, const float * im, float *output)
{
float *p_output = output;
const float *p_real = re;
const float *p_imag = im;
float *p_end = output + FFT_BUFFER_SIZE / 2;
while(p_output <= p_end)
{
*p_output = (*p_real * *p_real) + (*p_imag * *p_imag);
p_output++; p_real++; p_imag++;
}
*output /= 4;
*p_end /= 4;
}
static void fft_calculate(float * re, float * im, const float *costable, const float *sintable )
{
unsigned int i, j, k;
unsigned int exchanges;
float fact_real, fact_imag;
float tmp_real, tmp_imag;
unsigned int factfact;
exchanges = 1;
factfact = FFT_BUFFER_SIZE / 2;
for(i = FFT_BUFFER_SIZE_LOG; i != 0; i--) {
for(j = 0; j != exchanges; j++) {
fact_real = costable[j * factfact];
fact_imag = sintable[j * factfact];
for(k = j; k < FFT_BUFFER_SIZE; k += exchanges << 1) {
int k1 = k + exchanges;
tmp_real = fact_real * re[k1] - fact_imag * im[k1];
tmp_imag = fact_real * im[k1] + fact_imag * re[k1];
re[k1] = re[k] - tmp_real;
im[k1] = im[k] - tmp_imag;
re[k] += tmp_real;
im[k] += tmp_imag;
}
}
exchanges <<= 1;
factfact >>= 1;
}
}
static int reverseBits(unsigned int initial)
{
unsigned int reversed = 0, loop;
for(loop = 0; loop < FFT_BUFFER_SIZE_LOG; loop++) {
reversed <<= 1;
reversed += (initial & 1);
initial >>= 1;
}
return reversed;
}
