






















#if !defined(AVFILTER_VOLUME_H)
#define AVFILTER_VOLUME_H

#include "libavutil/common.h"
#include "libavutil/eval.h"
#include "libavutil/float_dsp.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"

enum PrecisionType {
PRECISION_FIXED = 0,
PRECISION_FLOAT,
PRECISION_DOUBLE,
};

enum EvalMode {
EVAL_MODE_ONCE,
EVAL_MODE_FRAME,
EVAL_MODE_NB
};

enum VolumeVarName {
VAR_N,
VAR_NB_CHANNELS,
VAR_NB_CONSUMED_SAMPLES,
VAR_NB_SAMPLES,
VAR_POS,
VAR_PTS,
VAR_SAMPLE_RATE,
VAR_STARTPTS,
VAR_STARTT,
VAR_T,
VAR_TB,
VAR_VOLUME,
VAR_VARS_NB
};

enum ReplayGainType {
REPLAYGAIN_DROP,
REPLAYGAIN_IGNORE,
REPLAYGAIN_TRACK,
REPLAYGAIN_ALBUM,
};

typedef struct VolumeContext {
const AVClass *class;
AVFloatDSPContext *fdsp;
int precision;
int eval_mode;
const char *volume_expr;
AVExpr *volume_pexpr;
double var_values[VAR_VARS_NB];

int replaygain;
double replaygain_preamp;
int replaygain_noclip;
double volume;
int volume_i;
int channels;
int planes;
enum AVSampleFormat sample_fmt;

void (*scale_samples)(uint8_t *dst, const uint8_t *src, int nb_samples,
int volume);
int samples_align;
} VolumeContext;

void ff_volume_init_x86(VolumeContext *vol);

#endif 
