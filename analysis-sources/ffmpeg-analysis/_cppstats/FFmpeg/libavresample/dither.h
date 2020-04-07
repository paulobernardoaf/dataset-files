#include "avresample.h"
#include "audio_data.h"
typedef struct DitherContext DitherContext;
typedef struct DitherDSPContext {
void (*quantize)(int16_t *dst, const float *src, float *dither, int len);
int ptr_align; 
int samples_align; 
void (*dither_int_to_float)(float *dst, int *src0, int len);
} DitherDSPContext;
DitherContext *ff_dither_alloc(AVAudioResampleContext *avr,
enum AVSampleFormat out_fmt,
enum AVSampleFormat in_fmt,
int channels, int sample_rate, int apply_map);
void ff_dither_free(DitherContext **c);
int ff_convert_dither(DitherContext *c, AudioData *dst, AudioData *src);
void ff_dither_init_x86(DitherDSPContext *ddsp,
enum AVResampleDitherMethod method);
