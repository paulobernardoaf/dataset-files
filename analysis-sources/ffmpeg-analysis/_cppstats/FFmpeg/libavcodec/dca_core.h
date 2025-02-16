#include "libavutil/common.h"
#include "libavutil/float_dsp.h"
#include "libavutil/fixed_dsp.h"
#include "libavutil/mem.h"
#include "avcodec.h"
#include "internal.h"
#include "get_bits.h"
#include "dca.h"
#include "dca_exss.h"
#include "dcadsp.h"
#include "dcadct.h"
#include "dcamath.h"
#include "dcahuff.h"
#include "fft.h"
#include "synth_filter.h"
#define DCA_CHANNELS 7
#define DCA_SUBBANDS 32
#define DCA_SUBBANDS_X96 64
#define DCA_SUBFRAMES 16
#define DCA_SUBBAND_SAMPLES 8
#define DCA_PCMBLOCK_SAMPLES 32
#define DCA_LFE_HISTORY 8
#define DCA_ABITS_MAX 26
#define DCA_CORE_CHANNELS_MAX 6
#define DCA_DMIX_CHANNELS_MAX 4
#define DCA_XXCH_CHANNELS_MAX 2
#define DCA_EXSS_CHANNELS_MAX 8
#define DCA_EXSS_CHSETS_MAX 4
#define DCA_FILTER_MODE_X96 0x01
#define DCA_FILTER_MODE_FIXED 0x02
enum DCACoreAudioMode {
DCA_AMODE_MONO, 
DCA_AMODE_MONO_DUAL, 
DCA_AMODE_STEREO, 
DCA_AMODE_STEREO_SUMDIFF, 
DCA_AMODE_STEREO_TOTAL, 
DCA_AMODE_3F, 
DCA_AMODE_2F1R, 
DCA_AMODE_3F1R, 
DCA_AMODE_2F2R, 
DCA_AMODE_3F2R, 
DCA_AMODE_COUNT
};
enum DCACoreExtAudioType {
DCA_EXT_AUDIO_XCH = 0,
DCA_EXT_AUDIO_X96 = 2,
DCA_EXT_AUDIO_XXCH = 6
};
enum DCACoreLFEFlag {
DCA_LFE_FLAG_NONE,
DCA_LFE_FLAG_128,
DCA_LFE_FLAG_64,
DCA_LFE_FLAG_INVALID
};
typedef struct DCADSPData {
union {
struct {
DECLARE_ALIGNED(32, float, hist1)[1024];
DECLARE_ALIGNED(32, float, hist2)[64];
} flt;
struct {
DECLARE_ALIGNED(32, int32_t, hist1)[1024];
DECLARE_ALIGNED(32, int32_t, hist2)[64];
} fix;
} u;
int offset;
} DCADSPData;
typedef struct DCACoreDecoder {
AVCodecContext *avctx;
GetBitContext gb;
GetBitContext gb_in;
int crc_present; 
int npcmblocks; 
int frame_size; 
int audio_mode; 
int sample_rate; 
int bit_rate; 
int drc_present; 
int ts_present; 
int aux_present; 
int ext_audio_type; 
int ext_audio_present; 
int sync_ssf; 
int lfe_present; 
int predictor_history; 
int filter_perfect; 
int source_pcm_res; 
int es_format; 
int sumdiff_front; 
int sumdiff_surround; 
int nsubframes; 
int nchannels; 
int ch_mask; 
int8_t nsubbands[DCA_CHANNELS]; 
int8_t subband_vq_start[DCA_CHANNELS]; 
int8_t joint_intensity_index[DCA_CHANNELS]; 
int8_t transition_mode_sel[DCA_CHANNELS]; 
int8_t scale_factor_sel[DCA_CHANNELS]; 
int8_t bit_allocation_sel[DCA_CHANNELS]; 
int8_t quant_index_sel[DCA_CHANNELS][DCA_CODE_BOOKS]; 
int32_t scale_factor_adj[DCA_CHANNELS][DCA_CODE_BOOKS]; 
int8_t nsubsubframes[DCA_SUBFRAMES]; 
int8_t prediction_mode[DCA_CHANNELS][DCA_SUBBANDS_X96]; 
int16_t prediction_vq_index[DCA_CHANNELS][DCA_SUBBANDS_X96]; 
int8_t bit_allocation[DCA_CHANNELS][DCA_SUBBANDS_X96]; 
int8_t transition_mode[DCA_SUBFRAMES][DCA_CHANNELS][DCA_SUBBANDS]; 
int32_t scale_factors[DCA_CHANNELS][DCA_SUBBANDS][2]; 
int8_t joint_scale_sel[DCA_CHANNELS]; 
int32_t joint_scale_factors[DCA_CHANNELS][DCA_SUBBANDS_X96]; 
int prim_dmix_embedded; 
int prim_dmix_type; 
int prim_dmix_coeff[DCA_DMIX_CHANNELS_MAX * DCA_CORE_CHANNELS_MAX]; 
int ext_audio_mask; 
int xch_pos; 
int xxch_crc_present; 
int xxch_mask_nbits; 
int xxch_core_mask; 
int xxch_spkr_mask; 
int xxch_dmix_embedded; 
int xxch_dmix_scale_inv; 
int xxch_dmix_mask[DCA_XXCH_CHANNELS_MAX]; 
int xxch_dmix_coeff[DCA_XXCH_CHANNELS_MAX * DCA_CORE_CHANNELS_MAX]; 
int xxch_pos; 
int x96_rev_no; 
int x96_crc_present; 
int x96_nchannels; 
int x96_high_res; 
int x96_subband_start; 
int x96_rand; 
int x96_pos; 
unsigned int x96_subband_size;
int32_t *x96_subband_buffer; 
int32_t *x96_subband_samples[DCA_CHANNELS][DCA_SUBBANDS_X96]; 
unsigned int subband_size;
int32_t *subband_buffer; 
int32_t *subband_samples[DCA_CHANNELS][DCA_SUBBANDS]; 
int32_t *lfe_samples; 
DCADSPData dcadsp_data[DCA_CHANNELS]; 
DCADSPContext *dcadsp;
DCADCTContext dcadct;
FFTContext imdct[2];
SynthFilterContext synth;
AVFloatDSPContext *float_dsp;
AVFixedDSPContext *fixed_dsp;
unsigned int output_size;
void *output_buffer; 
int32_t *output_samples[DCA_SPEAKER_COUNT]; 
int32_t output_history_lfe_fixed; 
float output_history_lfe_float; 
int ch_remap[DCA_SPEAKER_COUNT]; 
int request_mask; 
int npcmsamples; 
int output_rate; 
int filter_mode; 
} DCACoreDecoder;
static inline int ff_dca_core_map_spkr(DCACoreDecoder *core, int spkr)
{
if (core->ch_mask & (1U << spkr))
return spkr;
if (spkr == DCA_SPEAKER_Lss && (core->ch_mask & DCA_SPEAKER_MASK_Ls))
return DCA_SPEAKER_Ls;
if (spkr == DCA_SPEAKER_Rss && (core->ch_mask & DCA_SPEAKER_MASK_Rs))
return DCA_SPEAKER_Rs;
return -1;
}
static inline void ff_dca_core_dequantize(int32_t *output, const int32_t *input,
int32_t step_size, int32_t scale, int residual, int len)
{
int64_t step_scale = (int64_t)step_size * scale;
int n, shift = 0;
if (step_scale > (1 << 23)) {
shift = av_log2(step_scale >> 23) + 1;
step_scale >>= shift;
}
if (residual) {
for (n = 0; n < len; n++)
output[n] += clip23(norm__(input[n] * step_scale, 22 - shift));
} else {
for (n = 0; n < len; n++)
output[n] = clip23(norm__(input[n] * step_scale, 22 - shift));
}
}
int ff_dca_core_parse(DCACoreDecoder *s, uint8_t *data, int size);
int ff_dca_core_parse_exss(DCACoreDecoder *s, uint8_t *data, DCAExssAsset *asset);
int ff_dca_core_filter_fixed(DCACoreDecoder *s, int x96_synth);
int ff_dca_core_filter_frame(DCACoreDecoder *s, AVFrame *frame);
av_cold void ff_dca_core_flush(DCACoreDecoder *s);
av_cold int ff_dca_core_init(DCACoreDecoder *s);
av_cold void ff_dca_core_close(DCACoreDecoder *s);
