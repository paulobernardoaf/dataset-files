#include <stdint.h>
#include "libavutil/avassert.h"
#include "avfilter.h"
#include "generate_wave_table.h"
void ff_generate_wave_table(enum WaveType wave_type,
enum AVSampleFormat sample_fmt,
void *table, int table_size,
double min, double max, double phase)
{
uint32_t i, phase_offset = phase / M_PI / 2 * table_size + 0.5;
for (i = 0; i < table_size; i++) {
uint32_t point = (i + phase_offset) % table_size;
double d;
switch (wave_type) {
case WAVE_SIN:
d = (sin((double)point / table_size * 2 * M_PI) + 1) / 2;
break;
case WAVE_TRI:
d = (double)point * 2 / table_size;
switch (4 * point / table_size) {
case 0: d = d + 0.5; break;
case 1:
case 2: d = 1.5 - d; break;
case 3: d = d - 1.5; break;
}
break;
default:
av_assert0(0);
}
d = d * (max - min) + min;
switch (sample_fmt) {
case AV_SAMPLE_FMT_FLT: {
float *fp = (float *)table;
*fp++ = (float)d;
table = fp;
continue; }
case AV_SAMPLE_FMT_DBL: {
double *dp = (double *)table;
*dp++ = d;
table = dp;
continue; }
}
d += d < 0 ? -0.5 : 0.5;
switch (sample_fmt) {
case AV_SAMPLE_FMT_S16: {
int16_t *sp = table;
*sp++ = (int16_t)d;
table = sp;
continue; }
case AV_SAMPLE_FMT_S32: {
int32_t *ip = table;
*ip++ = (int32_t)d;
table = ip;
continue; }
default:
av_assert0(0);
}
}
}
