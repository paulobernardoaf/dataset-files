#include <stdint.h>
#include "libavutil/float_dsp.h"
#include "atrac.h"
#include "avcodec.h"
#include "fft.h"
#include "get_bits.h"
#define ATRAC3P_SUBBANDS 16 
#define ATRAC3P_SUBBAND_SAMPLES 128 
#define ATRAC3P_FRAME_SAMPLES (ATRAC3P_SUBBAND_SAMPLES * ATRAC3P_SUBBANDS)
#define ATRAC3P_PQF_FIR_LEN 12 
#define ATRAC3P_POWER_COMP_OFF 15 
enum Atrac3pChannelUnitTypes {
CH_UNIT_MONO = 0, 
CH_UNIT_STEREO = 1, 
CH_UNIT_EXTENSION = 2, 
CH_UNIT_TERMINATOR = 3 
};
typedef struct Atrac3pIPQFChannelCtx {
DECLARE_ALIGNED(32, float, buf1)[ATRAC3P_PQF_FIR_LEN * 2][8];
DECLARE_ALIGNED(32, float, buf2)[ATRAC3P_PQF_FIR_LEN * 2][8];
int pos;
} Atrac3pIPQFChannelCtx;
typedef struct Atrac3pWaveEnvelope {
int has_start_point; 
int has_stop_point; 
int start_pos; 
int stop_pos; 
} Atrac3pWaveEnvelope;
typedef struct Atrac3pWavesData {
Atrac3pWaveEnvelope pend_env; 
Atrac3pWaveEnvelope curr_env; 
int num_wavs; 
int start_index; 
} Atrac3pWavesData;
typedef struct Atrac3pWaveParam {
int freq_index; 
int amp_sf; 
int amp_index; 
int phase_index; 
} Atrac3pWaveParam;
typedef struct Atrac3pChanParams {
int ch_num;
int num_coded_vals; 
int fill_mode;
int split_point;
int table_type; 
int qu_wordlen[32]; 
int qu_sf_idx[32]; 
int qu_tab_idx[32]; 
int16_t spectrum[2048]; 
uint8_t power_levs[5]; 
uint8_t wnd_shape_hist[2][ATRAC3P_SUBBANDS]; 
uint8_t *wnd_shape; 
uint8_t *wnd_shape_prev; 
AtracGainInfo gain_data_hist[2][ATRAC3P_SUBBANDS]; 
AtracGainInfo *gain_data; 
AtracGainInfo *gain_data_prev; 
int num_gain_subbands; 
Atrac3pWavesData tones_info_hist[2][ATRAC3P_SUBBANDS];
Atrac3pWavesData *tones_info;
Atrac3pWavesData *tones_info_prev;
} Atrac3pChanParams;
typedef struct Atrac3pWaveSynthParams {
int tones_present; 
int amplitude_mode; 
int num_tone_bands; 
uint8_t tone_sharing[ATRAC3P_SUBBANDS]; 
uint8_t tone_master[ATRAC3P_SUBBANDS]; 
uint8_t invert_phase[ATRAC3P_SUBBANDS]; 
int tones_index; 
Atrac3pWaveParam waves[48];
} Atrac3pWaveSynthParams;
typedef struct Atrac3pChanUnitCtx {
int unit_type; 
int num_quant_units;
int num_subbands;
int used_quant_units; 
int num_coded_subbands; 
int mute_flag; 
int use_full_table; 
int noise_present; 
int noise_level_index; 
int noise_table_index; 
uint8_t swap_channels[ATRAC3P_SUBBANDS]; 
uint8_t negate_coeffs[ATRAC3P_SUBBANDS]; 
Atrac3pChanParams channels[2];
Atrac3pWaveSynthParams wave_synth_hist[2]; 
Atrac3pWaveSynthParams *waves_info;
Atrac3pWaveSynthParams *waves_info_prev;
Atrac3pIPQFChannelCtx ipqf_ctx[2];
DECLARE_ALIGNED(32, float, prev_buf)[2][ATRAC3P_FRAME_SAMPLES]; 
} Atrac3pChanUnitCtx;
void ff_atrac3p_init_vlcs(void);
int ff_atrac3p_decode_channel_unit(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx);
void ff_atrac3p_init_imdct(AVCodecContext *avctx, FFTContext *mdct_ctx);
void ff_atrac3p_init_wave_synth(void);
void ff_atrac3p_generate_tones(Atrac3pChanUnitCtx *ch_unit, AVFloatDSPContext *fdsp,
int ch_num, int sb, float *out);
void ff_atrac3p_power_compensation(Atrac3pChanUnitCtx *ctx, AVFloatDSPContext *fdsp,
int ch_index, float *sp, int rng_index, int sb_num);
void ff_atrac3p_imdct(AVFloatDSPContext *fdsp, FFTContext *mdct_ctx, float *pIn,
float *pOut, int wind_id, int sb);
void ff_atrac3p_ipqf(FFTContext *dct_ctx, Atrac3pIPQFChannelCtx *hist,
const float *in, float *out);
extern const uint16_t ff_atrac3p_qu_to_spec_pos[33];
extern const float ff_atrac3p_sf_tab[64];
extern const float ff_atrac3p_mant_tab[8];
