#include <stdint.h>
#include "aac_defines.h"
#include "libavutil/softfloat.h"
typedef struct SBRDSPContext {
void (*sum64x5)(INTFLOAT *z);
AAC_FLOAT (*sum_square)(INTFLOAT (*x)[2], int n);
void (*neg_odd_64)(INTFLOAT *x);
void (*qmf_pre_shuffle)(INTFLOAT *z);
void (*qmf_post_shuffle)(INTFLOAT W[32][2], const INTFLOAT *z);
void (*qmf_deint_neg)(INTFLOAT *v, const INTFLOAT *src);
void (*qmf_deint_bfly)(INTFLOAT *v, const INTFLOAT *src0, const INTFLOAT *src1);
void (*autocorrelate)(const INTFLOAT x[40][2], AAC_FLOAT phi[3][2][2]);
void (*hf_gen)(INTFLOAT (*X_high)[2], const INTFLOAT (*X_low)[2],
const INTFLOAT alpha0[2], const INTFLOAT alpha1[2],
INTFLOAT bw, int start, int end);
void (*hf_g_filt)(INTFLOAT (*Y)[2], const INTFLOAT (*X_high)[40][2],
const AAC_FLOAT *g_filt, int m_max, intptr_t ixh);
void (*hf_apply_noise[4])(INTFLOAT (*Y)[2], const AAC_FLOAT *s_m,
const AAC_FLOAT *q_filt, int noise,
int kx, int m_max);
} SBRDSPContext;
extern const INTFLOAT AAC_RENAME(ff_sbr_noise_table)[][2];
void AAC_RENAME(ff_sbrdsp_init)(SBRDSPContext *s);
void ff_sbrdsp_init_arm(SBRDSPContext *s);
void ff_sbrdsp_init_aarch64(SBRDSPContext *s);
void ff_sbrdsp_init_x86(SBRDSPContext *s);
void ff_sbrdsp_init_mips(SBRDSPContext *s);
