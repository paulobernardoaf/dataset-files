




















#if !defined(SWRESAMPLE_RESAMPLE_H)
#define SWRESAMPLE_RESAMPLE_H

#include "libavutil/log.h"
#include "libavutil/samplefmt.h"

#include "swresample_internal.h"

typedef struct ResampleContext {
const AVClass *av_class;
uint8_t *filter_bank;
int filter_length;
int filter_alloc;
int ideal_dst_incr;
int dst_incr;
int dst_incr_div;
int dst_incr_mod;
int index;
int frac;
int src_incr;
int compensation_distance;
int phase_count;
int linear;
enum SwrFilterType filter_type;
double kaiser_beta;
double factor;
enum AVSampleFormat format;
int felem_size;
int filter_shift;
int phase_count_compensation; 

struct {
void (*resample_one)(void *dst, const void *src,
int n, int64_t index, int64_t incr);
int (*resample_common)(struct ResampleContext *c, void *dst,
const void *src, int n, int update_ctx);
int (*resample_linear)(struct ResampleContext *c, void *dst,
const void *src, int n, int update_ctx);
} dsp;
} ResampleContext;

void swri_resample_dsp_init(ResampleContext *c);
void swri_resample_dsp_x86_init(ResampleContext *c);
void swri_resample_dsp_arm_init(ResampleContext *c);
void swri_resample_dsp_aarch64_init(ResampleContext *c);

#endif 
