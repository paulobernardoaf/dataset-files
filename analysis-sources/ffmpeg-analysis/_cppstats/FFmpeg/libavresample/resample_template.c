#include <math.h>
#include <stdint.h>
#include "libavutil/common.h"
#include "internal.h"
#if defined(CONFIG_RESAMPLE_DBL)
#define SET_TYPE(func) func ##_dbl
#define FELEM double
#define FELEM2 double
#define FELEML double
#define OUT(d, v) d = v
#define DBL_TO_FELEM(d, v) d = v
#elif defined(CONFIG_RESAMPLE_FLT)
#define SET_TYPE(func) func ##_flt
#define FELEM float
#define FELEM2 float
#define FELEML float
#define OUT(d, v) d = v
#define DBL_TO_FELEM(d, v) d = v
#elif defined(CONFIG_RESAMPLE_S32)
#define SET_TYPE(func) func ##_s32
#define FELEM int32_t
#define FELEM2 int64_t
#define FELEML int64_t
#define OUT(d, v) d = av_clipl_int32((v + (1 << 29)) >> 30)
#define DBL_TO_FELEM(d, v) d = av_clipl_int32(llrint(v * (1 << 30)));
#else
#define SET_TYPE(func) func ##_s16
#define FELEM int16_t
#define FELEM2 int32_t
#define FELEML int64_t
#define OUT(d, v) d = av_clip_int16((v + (1 << 14)) >> 15)
#define DBL_TO_FELEM(d, v) d = av_clip_int16(lrint(v * (1 << 15)))
#endif
static void SET_TYPE(resample_nearest)(void *dst0, int dst_index, const void *src0, unsigned int index)
{
FELEM *dst = dst0;
const FELEM *src = src0;
dst[dst_index] = src[index];
}
static void SET_TYPE(resample_linear)(ResampleContext *c, void *dst0, int dst_index,
const void *src0, unsigned int index, int frac)
{
FELEM *dst = dst0;
const FELEM *src = src0;
int i;
unsigned int sample_index = index >> c->phase_shift;
FELEM2 val = 0;
FELEM *filter = ((FELEM *)c->filter_bank) +
c->filter_length * (index & c->phase_mask);
FELEM2 v2 = 0;
for (i = 0; i < c->filter_length; i++) {
val += src[sample_index + i] * (FELEM2)filter[i];
v2 += src[sample_index + i] * (FELEM2)filter[i + c->filter_length];
}
val += (v2 - val) * (FELEML)frac / c->src_incr;
OUT(dst[dst_index], val);
}
static void SET_TYPE(resample_one)(ResampleContext *c,
void *dst0, int dst_index, const void *src0,
unsigned int index, int frac)
{
FELEM *dst = dst0;
const FELEM *src = src0;
int i;
unsigned int sample_index = index >> c->phase_shift;
FELEM2 val = 0;
FELEM *filter = ((FELEM *)c->filter_bank) +
c->filter_length * (index & c->phase_mask);
for (i = 0; i < c->filter_length; i++)
val += src[sample_index + i] * (FELEM2)filter[i];
OUT(dst[dst_index], val);
}
static void SET_TYPE(set_filter)(void *filter0, double *tab, int phase,
int tap_count)
{
int i;
FELEM *filter = ((FELEM *)filter0) + phase * tap_count;
for (i = 0; i < tap_count; i++) {
DBL_TO_FELEM(filter[i], tab[i]);
}
}
#undef SET_TYPE
#undef FELEM
#undef FELEM2
#undef FELEML
#undef OUT
#undef DBL_TO_FELEM
