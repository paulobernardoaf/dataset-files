#include "libavutil/samplefmt.h"
enum WaveType {
WAVE_SIN,
WAVE_TRI,
WAVE_NB,
};
void ff_generate_wave_table(enum WaveType wave_type,
enum AVSampleFormat sample_fmt,
void *table, int table_size,
double min, double max, double phase);
