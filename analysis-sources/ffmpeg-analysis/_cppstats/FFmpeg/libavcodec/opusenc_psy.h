#include "opusenc.h"
#include "opusenc_utils.h"
#include "libavfilter/window_func.h"
typedef struct OpusPsyStep {
int index; 
int silence;
float energy[OPUS_MAX_CHANNELS][CELT_MAX_BANDS]; 
float tone[OPUS_MAX_CHANNELS][CELT_MAX_BANDS]; 
float stereo[CELT_MAX_BANDS]; 
float change_amp[OPUS_MAX_CHANNELS][CELT_MAX_BANDS]; 
float total_change; 
float *bands[OPUS_MAX_CHANNELS][CELT_MAX_BANDS];
float coeffs[OPUS_MAX_CHANNELS][OPUS_BLOCK_SIZE(CELT_BLOCK_960)];
} OpusPsyStep;
typedef struct OpusBandExcitation {
float excitation;
float excitation_dist;
float excitation_init;
} OpusBandExcitation;
typedef struct PsyChain {
int start;
int end;
} PsyChain;
typedef struct OpusPsyContext {
AVCodecContext *avctx;
AVFloatDSPContext *dsp;
struct FFBufQueue *bufqueue;
OpusEncOptions *options;
PsyChain cs[128];
int cs_num;
OpusBandExcitation ex[OPUS_MAX_CHANNELS][CELT_MAX_BANDS];
FFBesselFilter bfilter_lo[OPUS_MAX_CHANNELS][CELT_MAX_BANDS];
FFBesselFilter bfilter_hi[OPUS_MAX_CHANNELS][CELT_MAX_BANDS];
OpusPsyStep *steps[FF_BUFQUEUE_SIZE + 1];
int max_steps;
float *window[CELT_BLOCK_NB];
MDCT15Context *mdct[CELT_BLOCK_NB];
int bsize_analysis;
DECLARE_ALIGNED(32, float, scratch)[2048];
float rc_waste;
float avg_is_band;
int64_t dual_stereo_used;
int64_t total_packets_out;
FFBesselFilter lambda_lp;
OpusPacketInfo p;
int redo_analysis;
int buffered_steps;
int steps_to_process;
int eof;
float lambda;
int *inflection_points;
int inflection_points_count;
} OpusPsyContext;
int ff_opus_psy_process (OpusPsyContext *s, OpusPacketInfo *p);
void ff_opus_psy_celt_frame_init (OpusPsyContext *s, CeltFrame *f, int index);
int ff_opus_psy_celt_frame_process(OpusPsyContext *s, CeltFrame *f, int index);
void ff_opus_psy_postencode_update (OpusPsyContext *s, CeltFrame *f, OpusRangeCoder *rc);
int ff_opus_psy_init(OpusPsyContext *s, AVCodecContext *avctx,
struct FFBufQueue *bufqueue, OpusEncOptions *options);
void ff_opus_psy_signal_eof(OpusPsyContext *s);
int ff_opus_psy_end(OpusPsyContext *s);
