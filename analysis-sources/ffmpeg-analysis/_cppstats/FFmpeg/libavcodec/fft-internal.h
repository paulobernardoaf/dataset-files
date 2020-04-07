#if FFT_FLOAT
#define FIX15(v) (v)
#define sqrthalf (float)M_SQRT1_2
#define BF(x, y, a, b) do { x = a - b; y = a + b; } while (0)
#define CMUL(dre, dim, are, aim, bre, bim) do { (dre) = (are) * (bre) - (aim) * (bim); (dim) = (are) * (bim) + (aim) * (bre); } while (0)
#else
#define SCALE_FLOAT(a, bits) lrint((a) * (double)(1 << (bits)))
#if FFT_FIXED_32
#define CMUL(dre, dim, are, aim, bre, bim) do { int64_t accu; (accu) = (int64_t)(bre) * (are); (accu) -= (int64_t)(bim) * (aim); (dre) = (int)(((accu) + 0x40000000) >> 31); (accu) = (int64_t)(bre) * (aim); (accu) += (int64_t)(bim) * (are); (dim) = (int)(((accu) + 0x40000000) >> 31); } while (0)
#define FIX15(a) av_clip(SCALE_FLOAT(a, 31), -2147483647, 2147483647)
#else 
#include "fft.h"
#include "mathops.h"
void ff_mdct_calcw_c(FFTContext *s, FFTDouble *output, const FFTSample *input);
#define FIX15(a) av_clip(SCALE_FLOAT(a, 15), -32767, 32767)
#define sqrthalf ((int16_t)((1<<15)*M_SQRT1_2))
#define BF(x, y, a, b) do { x = (a - b) >> 1; y = (a + b) >> 1; } while (0)
#define CMULS(dre, dim, are, aim, bre, bim, sh) do { (dre) = (MUL16(are, bre) - MUL16(aim, bim)) >> sh; (dim) = (MUL16(are, bim) + MUL16(aim, bre)) >> sh; } while (0)
#define CMUL(dre, dim, are, aim, bre, bim) CMULS(dre, dim, are, aim, bre, bim, 15)
#define CMULL(dre, dim, are, aim, bre, bim) CMULS(dre, dim, are, aim, bre, bim, 0)
#endif 
#endif 
#define ff_imdct_calc_c FFT_NAME(ff_imdct_calc_c)
#define ff_imdct_half_c FFT_NAME(ff_imdct_half_c)
#define ff_mdct_calc_c FFT_NAME(ff_mdct_calc_c)
void ff_imdct_calc_c(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_imdct_half_c(FFTContext *s, FFTSample *output, const FFTSample *input);
void ff_mdct_calc_c(FFTContext *s, FFTSample *output, const FFTSample *input);
