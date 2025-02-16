#include <stdint.h>
#include "fft.h"
#include "aacps.h"
#include "sbrdsp.h"
typedef struct AACContext AACContext;
typedef struct SpectrumParameters {
uint8_t bs_start_freq;
uint8_t bs_stop_freq;
uint8_t bs_xover_band;
uint8_t bs_freq_scale;
uint8_t bs_alter_scale;
uint8_t bs_noise_bands;
} SpectrumParameters;
#define SBR_SYNTHESIS_BUF_SIZE ((1280-128)*2)
typedef struct SBRData {
unsigned bs_frame_class;
unsigned bs_add_harmonic_flag;
AAC_SIGNE bs_num_env;
uint8_t bs_freq_res[7];
AAC_SIGNE bs_num_noise;
uint8_t bs_df_env[5];
uint8_t bs_df_noise[2];
uint8_t bs_invf_mode[2][5];
uint8_t bs_add_harmonic[48];
unsigned bs_amp_res;
DECLARE_ALIGNED(32, INTFLOAT, synthesis_filterbank_samples)[SBR_SYNTHESIS_BUF_SIZE];
DECLARE_ALIGNED(32, INTFLOAT, analysis_filterbank_samples) [1312];
int synthesis_filterbank_samples_offset;
int e_a[2];
INTFLOAT bw_array[5];
INTFLOAT W[2][32][32][2];
int Ypos;
DECLARE_ALIGNED(16, INTFLOAT, Y)[2][38][64][2];
DECLARE_ALIGNED(16, AAC_FLOAT, g_temp)[42][48];
AAC_FLOAT q_temp[42][48];
uint8_t s_indexmapped[8][48];
uint8_t env_facs_q[6][48];
AAC_FLOAT env_facs[6][48];
uint8_t noise_facs_q[3][5];
AAC_FLOAT noise_facs[3][5];
uint8_t t_env[8];
uint8_t t_env_num_env_old;
uint8_t t_q[3];
unsigned f_indexnoise;
unsigned f_indexsine;
} SBRData;
typedef struct SpectralBandReplication SpectralBandReplication;
typedef struct AACSBRContext {
int (*sbr_lf_gen)(AACContext *ac, SpectralBandReplication *sbr,
INTFLOAT X_low[32][40][2], const INTFLOAT W[2][32][32][2],
int buf_idx);
void (*sbr_hf_assemble)(INTFLOAT Y1[38][64][2],
const INTFLOAT X_high[64][40][2],
SpectralBandReplication *sbr, SBRData *ch_data,
const int e_a[2]);
int (*sbr_x_gen)(SpectralBandReplication *sbr, INTFLOAT X[2][38][64],
const INTFLOAT Y0[38][64][2], const INTFLOAT Y1[38][64][2],
const INTFLOAT X_low[32][40][2], int ch);
void (*sbr_hf_inverse_filter)(SBRDSPContext *dsp,
INTFLOAT (*alpha0)[2], INTFLOAT (*alpha1)[2],
const INTFLOAT X_low[32][40][2], int k0);
} AACSBRContext;
struct SpectralBandReplication {
int sample_rate;
int start;
int ready_for_dequant;
int id_aac;
int reset;
SpectrumParameters spectrum_params;
int bs_amp_res_header;
unsigned bs_limiter_bands;
unsigned bs_limiter_gains;
unsigned bs_interpol_freq;
unsigned bs_smoothing_mode;
unsigned bs_coupling;
AAC_SIGNE k[5]; 
AAC_SIGNE kx[2];
AAC_SIGNE m[2];
unsigned kx_and_m_pushed;
AAC_SIGNE n_master;
SBRData data[2];
PSContext ps;
AAC_SIGNE n[2];
AAC_SIGNE n_q;
AAC_SIGNE n_lim;
uint16_t f_master[49];
uint16_t f_tablelow[25];
uint16_t f_tablehigh[49];
uint16_t f_tablenoise[6];
uint16_t f_tablelim[30];
AAC_SIGNE num_patches;
uint8_t patch_num_subbands[6];
uint8_t patch_start_subband[6];
DECLARE_ALIGNED(16, INTFLOAT, X_low)[32][40][2];
DECLARE_ALIGNED(16, INTFLOAT, X_high)[64][40][2];
DECLARE_ALIGNED(16, INTFLOAT, X)[2][2][38][64];
DECLARE_ALIGNED(16, INTFLOAT, alpha0)[64][2];
DECLARE_ALIGNED(16, INTFLOAT, alpha1)[64][2];
AAC_FLOAT e_origmapped[7][48];
AAC_FLOAT q_mapped[7][48];
uint8_t s_mapped[7][48];
AAC_FLOAT e_curr[7][48];
AAC_FLOAT q_m[7][48];
AAC_FLOAT s_m[7][48];
AAC_FLOAT gain[7][48];
DECLARE_ALIGNED(32, INTFLOAT, qmf_filter_scratch)[5][64];
FFTContext mdct_ana;
FFTContext mdct;
SBRDSPContext dsp;
AACSBRContext c;
};
