























#if !defined(AVFILTER_EBUR128_H)
#define AVFILTER_EBUR128_H






#include <stddef.h> 





enum channel {
FF_EBUR128_UNUSED = 0, 
FF_EBUR128_LEFT,
FF_EBUR128_Mp030 = 1, 
FF_EBUR128_RIGHT,
FF_EBUR128_Mm030 = 2, 
FF_EBUR128_CENTER,
FF_EBUR128_Mp000 = 3, 
FF_EBUR128_LEFT_SURROUND,
FF_EBUR128_Mp110 = 4, 
FF_EBUR128_RIGHT_SURROUND,
FF_EBUR128_Mm110 = 5, 
FF_EBUR128_DUAL_MONO, 
FF_EBUR128_MpSC, 
FF_EBUR128_MmSC, 
FF_EBUR128_Mp060, 
FF_EBUR128_Mm060, 
FF_EBUR128_Mp090, 
FF_EBUR128_Mm090, 
FF_EBUR128_Mp135, 
FF_EBUR128_Mm135, 
FF_EBUR128_Mp180, 
FF_EBUR128_Up000, 
FF_EBUR128_Up030, 
FF_EBUR128_Um030, 
FF_EBUR128_Up045, 
FF_EBUR128_Um045, 
FF_EBUR128_Up090, 
FF_EBUR128_Um090, 
FF_EBUR128_Up110, 
FF_EBUR128_Um110, 
FF_EBUR128_Up135, 
FF_EBUR128_Um135, 
FF_EBUR128_Up180, 
FF_EBUR128_Tp000, 
FF_EBUR128_Bp000, 
FF_EBUR128_Bp045, 
FF_EBUR128_Bm045 
};





enum mode {

FF_EBUR128_MODE_M = (1 << 0),

FF_EBUR128_MODE_S = (1 << 1) | FF_EBUR128_MODE_M,

FF_EBUR128_MODE_I = (1 << 2) | FF_EBUR128_MODE_M,

FF_EBUR128_MODE_LRA = (1 << 3) | FF_EBUR128_MODE_S,

FF_EBUR128_MODE_SAMPLE_PEAK = (1 << 4) | FF_EBUR128_MODE_M,
};


struct FFEBUR128StateInternal;





typedef struct FFEBUR128State {
int mode; 
unsigned int channels; 
unsigned long samplerate; 
struct FFEBUR128StateInternal *d; 
} FFEBUR128State;









FFEBUR128State *ff_ebur128_init(unsigned int channels,
unsigned long samplerate,
unsigned long window, int mode);





void ff_ebur128_destroy(FFEBUR128State ** st);


















int ff_ebur128_set_channel(FFEBUR128State * st,
unsigned int channel_number, int value);







void ff_ebur128_add_frames_short(FFEBUR128State * st,
const short *src, size_t frames);

void ff_ebur128_add_frames_int(FFEBUR128State * st,
const int *src, size_t frames);

void ff_ebur128_add_frames_float(FFEBUR128State * st,
const float *src, size_t frames);

void ff_ebur128_add_frames_double(FFEBUR128State * st,
const double *src, size_t frames);








void ff_ebur128_add_frames_planar_short(FFEBUR128State * st,
const short **srcs,
size_t frames, int stride);

void ff_ebur128_add_frames_planar_int(FFEBUR128State * st,
const int **srcs,
size_t frames, int stride);

void ff_ebur128_add_frames_planar_float(FFEBUR128State * st,
const float **srcs,
size_t frames, int stride);

void ff_ebur128_add_frames_planar_double(FFEBUR128State * st,
const double **srcs,
size_t frames, int stride);










int ff_ebur128_loudness_global(FFEBUR128State * st, double *out);










int ff_ebur128_loudness_global_multiple(FFEBUR128State ** sts,
size_t size, double *out);









int ff_ebur128_loudness_momentary(FFEBUR128State * st, double *out);









int ff_ebur128_loudness_shortterm(FFEBUR128State * st, double *out);












int ff_ebur128_loudness_window(FFEBUR128State * st,
unsigned long window, double *out);












int ff_ebur128_loudness_range(FFEBUR128State * st, double *out);












int ff_ebur128_loudness_range_multiple(FFEBUR128State ** sts,
size_t size, double *out);











int ff_ebur128_sample_peak(FFEBUR128State * st,
unsigned int channel_number, double *out);









int ff_ebur128_relative_threshold(FFEBUR128State * st, double *out);

#endif 
