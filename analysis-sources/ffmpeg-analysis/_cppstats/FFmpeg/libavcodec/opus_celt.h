#include <float.h>
#include "opus.h"
#include "opus_pvq.h"
#include "opusdsp.h"
#include "mdct15.h"
#include "libavutil/float_dsp.h"
#include "libavutil/libm.h"
#define CELT_VECTORS 11
#define CELT_ALLOC_STEPS 6
#define CELT_FINE_OFFSET 21
#define CELT_MAX_FINE_BITS 8
#define CELT_NORM_SCALE 16384
#define CELT_QTHETA_OFFSET 4
#define CELT_QTHETA_OFFSET_TWOPHASE 16
#define CELT_POSTFILTER_MINPERIOD 15
#define CELT_ENERGY_SILENCE (-28.0f)
typedef struct CeltPVQ CeltPVQ;
enum CeltSpread {
CELT_SPREAD_NONE,
CELT_SPREAD_LIGHT,
CELT_SPREAD_NORMAL,
CELT_SPREAD_AGGRESSIVE
};
enum CeltBlockSize {
CELT_BLOCK_120,
CELT_BLOCK_240,
CELT_BLOCK_480,
CELT_BLOCK_960,
CELT_BLOCK_NB
};
typedef struct CeltBlock {
float energy[CELT_MAX_BANDS];
float lin_energy[CELT_MAX_BANDS];
float error_energy[CELT_MAX_BANDS];
float prev_energy[2][CELT_MAX_BANDS];
uint8_t collapse_masks[CELT_MAX_BANDS];
DECLARE_ALIGNED(32, float, buf)[2048];
DECLARE_ALIGNED(32, float, coeffs)[CELT_MAX_FRAME_SIZE];
DECLARE_ALIGNED(32, float, overlap)[FFALIGN(CELT_OVERLAP, 16)];
DECLARE_ALIGNED(32, float, samples)[FFALIGN(CELT_MAX_FRAME_SIZE, 16)];
int pf_period_new;
float pf_gains_new[3];
int pf_period;
float pf_gains[3];
int pf_period_old;
float pf_gains_old[3];
float emph_coeff;
} CeltBlock;
struct CeltFrame {
AVCodecContext *avctx;
MDCT15Context *imdct[4];
AVFloatDSPContext *dsp;
CeltBlock block[2];
CeltPVQ *pvq;
OpusDSP opusdsp;
int channels;
int output_channels;
int apply_phase_inv;
enum CeltBlockSize size;
int start_band;
int end_band;
int coded_bands;
int transient;
int pfilter;
int skip_band_floor;
int tf_select;
int alloc_trim;
int alloc_boost[CELT_MAX_BANDS];
int blocks; 
int blocksize; 
int silence; 
int anticollapse_needed; 
int anticollapse; 
int intensity_stereo;
int dual_stereo;
int flushed;
uint32_t seed;
enum CeltSpread spread;
int pf_octave;
int pf_period;
int pf_tapset;
float pf_gain;
int framebits;
int remaining;
int remaining2;
int caps [CELT_MAX_BANDS];
int fine_bits [CELT_MAX_BANDS];
int fine_priority[CELT_MAX_BANDS];
int pulses [CELT_MAX_BANDS];
int tf_change [CELT_MAX_BANDS];
};
static av_always_inline uint32_t celt_rng(CeltFrame *f)
{
f->seed = 1664525 * f->seed + 1013904223;
return f->seed;
}
static av_always_inline void celt_renormalize_vector(float *X, int N, float gain)
{
int i;
float g = 1e-15f;
for (i = 0; i < N; i++)
g += X[i] * X[i];
g = gain / sqrtf(g);
for (i = 0; i < N; i++)
X[i] *= g;
}
int ff_celt_init(AVCodecContext *avctx, CeltFrame **f, int output_channels,
int apply_phase_inv);
void ff_celt_free(CeltFrame **f);
void ff_celt_flush(CeltFrame *f);
int ff_celt_decode_frame(CeltFrame *f, OpusRangeCoder *rc, float **output,
int coded_channels, int frame_size, int startband, int endband);
