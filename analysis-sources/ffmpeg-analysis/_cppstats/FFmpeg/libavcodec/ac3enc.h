#include <stdint.h>
#include "libavutil/float_dsp.h"
#include "ac3.h"
#include "ac3dsp.h"
#include "avcodec.h"
#include "fft.h"
#include "mathops.h"
#include "me_cmp.h"
#include "put_bits.h"
#include "audiodsp.h"
#if !defined(CONFIG_AC3ENC_FLOAT)
#define CONFIG_AC3ENC_FLOAT 0
#endif
#define OFFSET(param) offsetof(AC3EncodeContext, options.param)
#define AC3ENC_PARAM (AV_OPT_FLAG_AUDIO_PARAM | AV_OPT_FLAG_ENCODING_PARAM)
#define AC3ENC_TYPE_AC3_FIXED 0
#define AC3ENC_TYPE_AC3 1
#define AC3ENC_TYPE_EAC3 2
#if CONFIG_AC3ENC_FLOAT
#define AC3_NAME(x) ff_ac3_float_ ##x
#define MAC_COEF(d,a,b) ((d)+=(a)*(b))
#define COEF_MIN (-16777215.0/16777216.0)
#define COEF_MAX ( 16777215.0/16777216.0)
#define NEW_CPL_COORD_THRESHOLD 0.03
typedef float SampleType;
typedef float CoefType;
typedef float CoefSumType;
#else
#define AC3_NAME(x) ff_ac3_fixed_ ##x
#define MAC_COEF(d,a,b) MAC64(d,a,b)
#define COEF_MIN -16777215
#define COEF_MAX 16777215
#define NEW_CPL_COORD_THRESHOLD 503317
typedef int16_t SampleType;
typedef int32_t CoefType;
typedef int64_t CoefSumType;
#endif
#define AC3ENC_OPT_NONE -1
#define AC3ENC_OPT_AUTO -1
#define AC3ENC_OPT_OFF 0
#define AC3ENC_OPT_ON 1
#define AC3ENC_OPT_NOT_INDICATED 0
#define AC3ENC_OPT_MODE_ON 2
#define AC3ENC_OPT_MODE_OFF 1
#define AC3ENC_OPT_DSUREX_DPLIIZ 3
#define AC3ENC_OPT_LARGE_ROOM 1
#define AC3ENC_OPT_SMALL_ROOM 2
#define AC3ENC_OPT_DOWNMIX_LTRT 1
#define AC3ENC_OPT_DOWNMIX_LORO 2
#define AC3ENC_OPT_DOWNMIX_DPLII 3 
#define AC3ENC_OPT_ADCONV_STANDARD 0
#define AC3ENC_OPT_ADCONV_HDCD 1
typedef struct AC3EncOptions {
int dialogue_level;
int bitstream_mode;
float center_mix_level;
float surround_mix_level;
int dolby_surround_mode;
int audio_production_info;
int mixing_level;
int room_type;
int copyright;
int original;
int extended_bsi_1;
int preferred_stereo_downmix;
float ltrt_center_mix_level;
float ltrt_surround_mix_level;
float loro_center_mix_level;
float loro_surround_mix_level;
int extended_bsi_2;
int dolby_surround_ex_mode;
int dolby_headphone_mode;
int ad_converter_type;
int eac3_mixing_metadata;
int eac3_info_metadata;
int allow_per_frame_metadata;
int stereo_rematrixing;
int channel_coupling;
int cpl_start;
} AC3EncOptions;
typedef struct AC3Block {
CoefType **mdct_coef; 
int32_t **fixed_coef; 
uint8_t **exp; 
uint8_t **grouped_exp; 
int16_t **psd; 
int16_t **band_psd; 
int16_t **mask; 
uint16_t **qmant; 
uint8_t **cpl_coord_exp; 
uint8_t **cpl_coord_mant; 
uint8_t coeff_shift[AC3_MAX_CHANNELS]; 
uint8_t new_rematrixing_strategy; 
int num_rematrixing_bands; 
uint8_t rematrixing_flags[4]; 
int new_cpl_strategy; 
int cpl_in_use; 
uint8_t channel_in_cpl[AC3_MAX_CHANNELS]; 
int num_cpl_channels; 
uint8_t new_cpl_coords[AC3_MAX_CHANNELS]; 
uint8_t cpl_master_exp[AC3_MAX_CHANNELS]; 
int new_snr_offsets; 
int new_cpl_leak; 
int end_freq[AC3_MAX_CHANNELS]; 
} AC3Block;
typedef struct AC3EncodeContext {
AVClass *av_class; 
AC3EncOptions options; 
AVCodecContext *avctx; 
PutBitContext pb; 
AudioDSPContext adsp;
AVFloatDSPContext *fdsp;
MECmpContext mecc;
AC3DSPContext ac3dsp; 
FFTContext mdct; 
const SampleType *mdct_window; 
AC3Block blocks[AC3_MAX_BLOCKS]; 
int fixed_point; 
int eac3; 
int bitstream_id; 
int bitstream_mode; 
int bit_rate; 
int sample_rate; 
int num_blks_code; 
int num_blocks; 
int frame_size_min; 
int frame_size; 
int frame_size_code; 
uint16_t crc_inv[2];
int64_t bits_written; 
int64_t samples_written; 
int fbw_channels; 
int channels; 
int lfe_on; 
int lfe_channel; 
int has_center; 
int has_surround; 
int channel_mode; 
const uint8_t *channel_map; 
int center_mix_level; 
int surround_mix_level; 
int ltrt_center_mix_level; 
int ltrt_surround_mix_level; 
int loro_center_mix_level; 
int loro_surround_mix_level; 
int cutoff; 
int bandwidth_code; 
int start_freq[AC3_MAX_CHANNELS]; 
int cpl_end_freq; 
int cpl_on; 
int cpl_enabled; 
int num_cpl_subbands; 
int num_cpl_bands; 
uint8_t cpl_band_sizes[AC3_MAX_CPL_BANDS]; 
int rematrixing_enabled; 
int slow_gain_code; 
int slow_decay_code; 
int fast_decay_code; 
int db_per_bit_code; 
int floor_code; 
AC3BitAllocParameters bit_alloc; 
int coarse_snr_offset; 
int fast_gain_code[AC3_MAX_CHANNELS]; 
int fine_snr_offset[AC3_MAX_CHANNELS]; 
int frame_bits_fixed; 
int frame_bits; 
int exponent_bits; 
SampleType *windowed_samples;
SampleType **planar_samples;
uint8_t *bap_buffer;
uint8_t *bap1_buffer;
CoefType *mdct_coef_buffer;
int32_t *fixed_coef_buffer;
uint8_t *exp_buffer;
uint8_t *grouped_exp_buffer;
int16_t *psd_buffer;
int16_t *band_psd_buffer;
int16_t *mask_buffer;
int16_t *qmant_buffer;
uint8_t *cpl_coord_exp_buffer;
uint8_t *cpl_coord_mant_buffer;
uint8_t exp_strategy[AC3_MAX_CHANNELS][AC3_MAX_BLOCKS]; 
uint8_t frame_exp_strategy[AC3_MAX_CHANNELS]; 
int use_frame_exp_strategy; 
uint8_t exp_ref_block[AC3_MAX_CHANNELS][AC3_MAX_BLOCKS]; 
uint8_t *ref_bap [AC3_MAX_CHANNELS][AC3_MAX_BLOCKS]; 
int ref_bap_set; 
int warned_alternate_bitstream;
void (*mdct_end)(struct AC3EncodeContext *s);
int (*mdct_init)(struct AC3EncodeContext *s);
int (*allocate_sample_buffers)(struct AC3EncodeContext *s);
void (*output_frame_header)(struct AC3EncodeContext *s);
} AC3EncodeContext;
extern const uint64_t ff_ac3_channel_layouts[19];
int ff_ac3_encode_init(AVCodecContext *avctx);
int ff_ac3_float_encode_init(AVCodecContext *avctx);
int ff_ac3_encode_close(AVCodecContext *avctx);
int ff_ac3_validate_metadata(AC3EncodeContext *s);
void ff_ac3_adjust_frame_size(AC3EncodeContext *s);
void ff_ac3_compute_coupling_strategy(AC3EncodeContext *s);
void ff_ac3_apply_rematrixing(AC3EncodeContext *s);
void ff_ac3_process_exponents(AC3EncodeContext *s);
int ff_ac3_compute_bit_allocation(AC3EncodeContext *s);
void ff_ac3_group_exponents(AC3EncodeContext *s);
void ff_ac3_quantize_mantissas(AC3EncodeContext *s);
void ff_ac3_output_frame(AC3EncodeContext *s, unsigned char *frame);
void ff_ac3_fixed_mdct_end(AC3EncodeContext *s);
void ff_ac3_float_mdct_end(AC3EncodeContext *s);
int ff_ac3_fixed_mdct_init(AC3EncodeContext *s);
int ff_ac3_float_mdct_init(AC3EncodeContext *s);
int ff_ac3_fixed_allocate_sample_buffers(AC3EncodeContext *s);
int ff_ac3_float_allocate_sample_buffers(AC3EncodeContext *s);
int ff_ac3_fixed_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr);
int ff_ac3_float_encode_frame(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr);
