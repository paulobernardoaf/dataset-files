





















static void sbr_sum64x5_c(INTFLOAT *z)
{
int k;
for (k = 0; k < 64; k++) {
INTFLOAT f = z[k] + z[k + 64] + z[k + 128] + z[k + 192] + z[k + 256];
z[k] = f;
}
}

static void sbr_qmf_deint_bfly_c(INTFLOAT *v, const INTFLOAT *src0, const INTFLOAT *src1)
{
int i;
for (i = 0; i < 64; i++) {
#if USE_FIXED
v[ i] = (int)(0x10U + src0[i] - src1[63 - i]) >> 5;
v[127 - i] = (int)(0x10U + src0[i] + src1[63 - i]) >> 5;
#else
v[ i] = src0[i] - src1[63 - i];
v[127 - i] = src0[i] + src1[63 - i];
#endif
}
}

static void sbr_hf_apply_noise_0(INTFLOAT (*Y)[2], const AAC_FLOAT *s_m,
const AAC_FLOAT *q_filt, int noise,
int kx, int m_max)
{
sbr_hf_apply_noise(Y, s_m, q_filt, noise, (INTFLOAT)1.0, (INTFLOAT)0.0, m_max);
}

static void sbr_hf_apply_noise_1(INTFLOAT (*Y)[2], const AAC_FLOAT *s_m,
const AAC_FLOAT *q_filt, int noise,
int kx, int m_max)
{
INTFLOAT phi_sign = 1 - 2 * (kx & 1);
sbr_hf_apply_noise(Y, s_m, q_filt, noise, (INTFLOAT)0.0, phi_sign, m_max);
}

static void sbr_hf_apply_noise_2(INTFLOAT (*Y)[2], const AAC_FLOAT *s_m,
const AAC_FLOAT *q_filt, int noise,
int kx, int m_max)
{
sbr_hf_apply_noise(Y, s_m, q_filt, noise, (INTFLOAT)-1.0, (INTFLOAT)0.0, m_max);
}

static void sbr_hf_apply_noise_3(INTFLOAT (*Y)[2], const AAC_FLOAT *s_m,
const AAC_FLOAT *q_filt, int noise,
int kx, int m_max)
{
INTFLOAT phi_sign = 1 - 2 * (kx & 1);
sbr_hf_apply_noise(Y, s_m, q_filt, noise, (INTFLOAT)0.0, -phi_sign, m_max);
}

av_cold void AAC_RENAME(ff_sbrdsp_init)(SBRDSPContext *s)
{
s->sum64x5 = sbr_sum64x5_c;
s->sum_square = sbr_sum_square_c;
s->neg_odd_64 = sbr_neg_odd_64_c;
s->qmf_pre_shuffle = sbr_qmf_pre_shuffle_c;
s->qmf_post_shuffle = sbr_qmf_post_shuffle_c;
s->qmf_deint_neg = sbr_qmf_deint_neg_c;
s->qmf_deint_bfly = sbr_qmf_deint_bfly_c;
s->autocorrelate = sbr_autocorrelate_c;
s->hf_gen = sbr_hf_gen_c;
s->hf_g_filt = sbr_hf_g_filt_c;

s->hf_apply_noise[0] = sbr_hf_apply_noise_0;
s->hf_apply_noise[1] = sbr_hf_apply_noise_1;
s->hf_apply_noise[2] = sbr_hf_apply_noise_2;
s->hf_apply_noise[3] = sbr_hf_apply_noise_3;

#if !USE_FIXED
if (ARCH_ARM)
ff_sbrdsp_init_arm(s);
if (ARCH_AARCH64)
ff_sbrdsp_init_aarch64(s);
if (ARCH_X86)
ff_sbrdsp_init_x86(s);
if (ARCH_MIPS)
ff_sbrdsp_init_mips(s);
#endif 
}
