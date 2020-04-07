#include "config.h"
typedef struct AVFloatDSPContext {
void (*vector_fmul)(float *dst, const float *src0, const float *src1,
int len);
void (*vector_fmac_scalar)(float *dst, const float *src, float mul,
int len);
void (*vector_dmac_scalar)(double *dst, const double *src, double mul,
int len);
void (*vector_fmul_scalar)(float *dst, const float *src, float mul,
int len);
void (*vector_dmul_scalar)(double *dst, const double *src, double mul,
int len);
void (*vector_fmul_window)(float *dst, const float *src0,
const float *src1, const float *win, int len);
void (*vector_fmul_add)(float *dst, const float *src0, const float *src1,
const float *src2, int len);
void (*vector_fmul_reverse)(float *dst, const float *src0,
const float *src1, int len);
void (*butterflies_float)(float *av_restrict v1, float *av_restrict v2, int len);
float (*scalarproduct_float)(const float *v1, const float *v2, int len);
void (*vector_dmul)(double *dst, const double *src0, const double *src1,
int len);
} AVFloatDSPContext;
float avpriv_scalarproduct_float_c(const float *v1, const float *v2, int len);
void ff_float_dsp_init_aarch64(AVFloatDSPContext *fdsp);
void ff_float_dsp_init_arm(AVFloatDSPContext *fdsp);
void ff_float_dsp_init_ppc(AVFloatDSPContext *fdsp, int strict);
void ff_float_dsp_init_x86(AVFloatDSPContext *fdsp);
void ff_float_dsp_init_mips(AVFloatDSPContext *fdsp);
AVFloatDSPContext *avpriv_float_dsp_alloc(int strict);
