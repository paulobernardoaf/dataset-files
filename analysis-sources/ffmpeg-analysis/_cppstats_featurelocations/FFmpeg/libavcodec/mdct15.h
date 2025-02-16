



















#if !defined(AVCODEC_MDCT15_H)
#define AVCODEC_MDCT15_H

#include <stddef.h>

#include "fft.h"

typedef struct MDCT15Context {
int fft_n;
int len2;
int len4;
int inverse;
int *pfa_prereindex;
int *pfa_postreindex;

FFTContext ptwo_fft;
FFTComplex *tmp;
FFTComplex *twiddle_exptab;

DECLARE_ALIGNED(32, FFTComplex, exptab)[64];


void (*fft15)(FFTComplex *out, FFTComplex *in, FFTComplex *exptab, ptrdiff_t stride);


void (*postreindex)(FFTComplex *out, FFTComplex *in, FFTComplex *exp, int *lut, ptrdiff_t len8);


void (*mdct)(struct MDCT15Context *s, float *dst, const float *src, ptrdiff_t stride);


void (*imdct_half)(struct MDCT15Context *s, float *dst, const float *src,
ptrdiff_t stride);
} MDCT15Context;


int ff_mdct15_init(MDCT15Context **ps, int inverse, int N, double scale);
void ff_mdct15_uninit(MDCT15Context **ps);

void ff_mdct15_init_x86(MDCT15Context *s);

#endif 
