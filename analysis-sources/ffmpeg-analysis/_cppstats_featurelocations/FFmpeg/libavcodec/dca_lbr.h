



















#if !defined(AVCODEC_DCA_LBR_H)
#define AVCODEC_DCA_LBR_H

#include "libavutil/common.h"
#include "libavutil/float_dsp.h"
#include "libavutil/mem.h"

#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "dca.h"
#include "dca_exss.h"
#include "dcadsp.h"
#include "fft.h"

#define DCA_LBR_CHANNELS 6
#define DCA_LBR_CHANNELS_TOTAL 32
#define DCA_LBR_SUBBANDS 32
#define DCA_LBR_TONES 512

#define DCA_LBR_TIME_SAMPLES 128
#define DCA_LBR_TIME_HISTORY 8

enum DCALBRHeader {
DCA_LBR_HEADER_SYNC_ONLY = 1,
DCA_LBR_HEADER_DECODER_INIT = 2
};

typedef struct DCALbrTone {
uint8_t x_freq; 
uint8_t f_delt; 
uint8_t ph_rot; 
uint8_t pad; 
uint8_t amp[DCA_LBR_CHANNELS]; 
uint8_t phs[DCA_LBR_CHANNELS]; 
} DCALbrTone;

typedef struct DCALbrDecoder {
AVCodecContext *avctx;
GetBitContext gb;

int sample_rate; 
int ch_mask; 
int flags; 
int bit_rate_orig; 
int bit_rate_scaled; 

int nchannels; 
int nchannels_total; 
int freq_range; 
int band_limit; 
int limited_rate; 
int limited_range; 
int res_profile; 
int nsubbands; 
int g3_avg_only_start_sb; 
int min_mono_subband; 
int max_mono_subband; 

int framenum; 
int lbr_rand; 
int warned; 

uint8_t quant_levels[DCA_LBR_CHANNELS / 2][DCA_LBR_SUBBANDS]; 
uint8_t sb_indices[DCA_LBR_SUBBANDS]; 

uint8_t sec_ch_sbms[DCA_LBR_CHANNELS / 2][DCA_LBR_SUBBANDS]; 
uint8_t sec_ch_lrms[DCA_LBR_CHANNELS / 2][DCA_LBR_SUBBANDS]; 
uint32_t ch_pres[DCA_LBR_CHANNELS]; 

uint8_t grid_1_scf[DCA_LBR_CHANNELS][12][8]; 
uint8_t grid_2_scf[DCA_LBR_CHANNELS][3][64]; 

int8_t grid_3_avg[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS - 4]; 
int8_t grid_3_scf[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS - 4][8]; 
uint32_t grid_3_pres[DCA_LBR_CHANNELS]; 

uint8_t high_res_scf[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS][8]; 

uint8_t part_stereo[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS / 4][5]; 
uint8_t part_stereo_pres; 

float lpc_coeff[2][DCA_LBR_CHANNELS][3][2][8]; 

float sb_scf[DCA_LBR_SUBBANDS]; 

float *time_samples[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS]; 

float *ts_buffer; 
unsigned int ts_size; 

DECLARE_ALIGNED(32, float, history)[DCA_LBR_CHANNELS][DCA_LBR_SUBBANDS * 4]; 
DECLARE_ALIGNED(32, float, window)[DCA_LBR_SUBBANDS * 4]; 

DECLARE_ALIGNED(32, float, lfe_data)[64]; 
DECLARE_ALIGNED(32, float, lfe_history)[5][2]; 
float lfe_scale; 

uint8_t tonal_scf[6]; 
uint16_t tonal_bounds[5][32][2]; 
DCALbrTone tones[DCA_LBR_TONES]; 
int ntones; 

FFTContext imdct;
AVFloatDSPContext *fdsp;
DCADSPContext *dcadsp;
} DCALbrDecoder;

int ff_dca_lbr_parse(DCALbrDecoder *s, uint8_t *data, DCAExssAsset *asset);
int ff_dca_lbr_filter_frame(DCALbrDecoder *s, AVFrame *frame);
av_cold void ff_dca_lbr_flush(DCALbrDecoder *s);
av_cold int ff_dca_lbr_init(DCALbrDecoder *s);
av_cold void ff_dca_lbr_close(DCALbrDecoder *s);

#endif
