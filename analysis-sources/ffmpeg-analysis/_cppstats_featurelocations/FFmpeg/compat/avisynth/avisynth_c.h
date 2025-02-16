




















































#if !defined(__AVISYNTH_C__)
#define __AVISYNTH_C__

#include "avs/config.h"
#include "avs/capi.h"
#include "avs/types.h"

#define AVS_FRAME_ALIGN FRAME_ALIGN





#if !defined(__AVISYNTH_6_H__)
enum { AVISYNTH_INTERFACE_VERSION = 6 };
#endif

enum {AVS_SAMPLE_INT8 = 1<<0,
AVS_SAMPLE_INT16 = 1<<1,
AVS_SAMPLE_INT24 = 1<<2,
AVS_SAMPLE_INT32 = 1<<3,
AVS_SAMPLE_FLOAT = 1<<4};

enum {AVS_PLANAR_Y=1<<0,
AVS_PLANAR_U=1<<1,
AVS_PLANAR_V=1<<2,
AVS_PLANAR_ALIGNED=1<<3,
AVS_PLANAR_Y_ALIGNED=AVS_PLANAR_Y|AVS_PLANAR_ALIGNED,
AVS_PLANAR_U_ALIGNED=AVS_PLANAR_U|AVS_PLANAR_ALIGNED,
AVS_PLANAR_V_ALIGNED=AVS_PLANAR_V|AVS_PLANAR_ALIGNED,
AVS_PLANAR_A=1<<4,
AVS_PLANAR_R=1<<5,
AVS_PLANAR_G=1<<6,
AVS_PLANAR_B=1<<7,
AVS_PLANAR_A_ALIGNED=AVS_PLANAR_A|AVS_PLANAR_ALIGNED,
AVS_PLANAR_R_ALIGNED=AVS_PLANAR_R|AVS_PLANAR_ALIGNED,
AVS_PLANAR_G_ALIGNED=AVS_PLANAR_G|AVS_PLANAR_ALIGNED,
AVS_PLANAR_B_ALIGNED=AVS_PLANAR_B|AVS_PLANAR_ALIGNED};


enum {
AVS_CS_YUVA = 1 << 27,
AVS_CS_BGR = 1 << 28,
AVS_CS_YUV = 1 << 29,
AVS_CS_INTERLEAVED = 1 << 30,
AVS_CS_PLANAR = 1 << 31,

AVS_CS_SHIFT_SUB_WIDTH = 0,
AVS_CS_SHIFT_SUB_HEIGHT = 8,
AVS_CS_SHIFT_SAMPLE_BITS = 16,

AVS_CS_SUB_WIDTH_MASK = 7 << AVS_CS_SHIFT_SUB_WIDTH,
AVS_CS_SUB_WIDTH_1 = 3 << AVS_CS_SHIFT_SUB_WIDTH, 
AVS_CS_SUB_WIDTH_2 = 0 << AVS_CS_SHIFT_SUB_WIDTH, 
AVS_CS_SUB_WIDTH_4 = 1 << AVS_CS_SHIFT_SUB_WIDTH, 

AVS_CS_VPLANEFIRST = 1 << 3, 
AVS_CS_UPLANEFIRST = 1 << 4, 

AVS_CS_SUB_HEIGHT_MASK = 7 << AVS_CS_SHIFT_SUB_HEIGHT,
AVS_CS_SUB_HEIGHT_1 = 3 << AVS_CS_SHIFT_SUB_HEIGHT, 
AVS_CS_SUB_HEIGHT_2 = 0 << AVS_CS_SHIFT_SUB_HEIGHT, 
AVS_CS_SUB_HEIGHT_4 = 1 << AVS_CS_SHIFT_SUB_HEIGHT, 

AVS_CS_SAMPLE_BITS_MASK = 7 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_8 = 0 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_10 = 5 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_12 = 6 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_14 = 7 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_16 = 1 << AVS_CS_SHIFT_SAMPLE_BITS,
AVS_CS_SAMPLE_BITS_32 = 2 << AVS_CS_SHIFT_SAMPLE_BITS,

AVS_CS_PLANAR_MASK = AVS_CS_PLANAR | AVS_CS_INTERLEAVED | AVS_CS_YUV | AVS_CS_BGR | AVS_CS_YUVA | AVS_CS_SAMPLE_BITS_MASK | AVS_CS_SUB_HEIGHT_MASK | AVS_CS_SUB_WIDTH_MASK,
AVS_CS_PLANAR_FILTER = ~(AVS_CS_VPLANEFIRST | AVS_CS_UPLANEFIRST),

AVS_CS_RGB_TYPE = 1 << 0,
AVS_CS_RGBA_TYPE = 1 << 1,

AVS_CS_GENERIC_YUV420 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_2 | AVS_CS_SUB_WIDTH_2, 
AVS_CS_GENERIC_YUV422 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_1 | AVS_CS_SUB_WIDTH_2, 
AVS_CS_GENERIC_YUV444 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_1 | AVS_CS_SUB_WIDTH_1, 
AVS_CS_GENERIC_Y = AVS_CS_PLANAR | AVS_CS_INTERLEAVED | AVS_CS_YUV, 
AVS_CS_GENERIC_RGBP = AVS_CS_PLANAR | AVS_CS_BGR | AVS_CS_RGB_TYPE, 
AVS_CS_GENERIC_RGBAP = AVS_CS_PLANAR | AVS_CS_BGR | AVS_CS_RGBA_TYPE, 
AVS_CS_GENERIC_YUVA420 = AVS_CS_PLANAR | AVS_CS_YUVA | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_2 | AVS_CS_SUB_WIDTH_2, 
AVS_CS_GENERIC_YUVA422 = AVS_CS_PLANAR | AVS_CS_YUVA | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_1 | AVS_CS_SUB_WIDTH_2, 
AVS_CS_GENERIC_YUVA444 = AVS_CS_PLANAR | AVS_CS_YUVA | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_1 | AVS_CS_SUB_WIDTH_1 }; 



enum {
AVS_CS_UNKNOWN = 0,
AVS_CS_BGR24 = AVS_CS_RGB_TYPE | AVS_CS_BGR | AVS_CS_INTERLEAVED,
AVS_CS_BGR32 = AVS_CS_RGBA_TYPE | AVS_CS_BGR | AVS_CS_INTERLEAVED,
AVS_CS_YUY2 = 1<<2 | AVS_CS_YUV | AVS_CS_INTERLEAVED,


AVS_CS_RAW32 = 1<<5 | AVS_CS_INTERLEAVED,

AVS_CS_YV24 = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_YV16 = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_YV12 = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_I420 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_SAMPLE_BITS_8 | AVS_CS_UPLANEFIRST | AVS_CS_SUB_HEIGHT_2 | AVS_CS_SUB_WIDTH_2, 
AVS_CS_IYUV = AVS_CS_I420,
AVS_CS_YV411 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_SAMPLE_BITS_8 | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_1 | AVS_CS_SUB_WIDTH_4, 
AVS_CS_YUV9 = AVS_CS_PLANAR | AVS_CS_YUV | AVS_CS_SAMPLE_BITS_8 | AVS_CS_VPLANEFIRST | AVS_CS_SUB_HEIGHT_4 | AVS_CS_SUB_WIDTH_4, 
AVS_CS_Y8 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_8, 




AVS_CS_YUV444P10 = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_YUV422P10 = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_YUV420P10 = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_Y10 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_10, 

AVS_CS_YUV444P12 = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_YUV422P12 = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_YUV420P12 = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_Y12 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_12, 

AVS_CS_YUV444P14 = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_YUV422P14 = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_YUV420P14 = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_Y14 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_14, 

AVS_CS_YUV444P16 = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_YUV422P16 = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_YUV420P16 = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_Y16 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_16, 


AVS_CS_YUV444PS = AVS_CS_GENERIC_YUV444 | AVS_CS_SAMPLE_BITS_32, 
AVS_CS_YUV422PS = AVS_CS_GENERIC_YUV422 | AVS_CS_SAMPLE_BITS_32, 
AVS_CS_YUV420PS = AVS_CS_GENERIC_YUV420 | AVS_CS_SAMPLE_BITS_32, 
AVS_CS_Y32 = AVS_CS_GENERIC_Y | AVS_CS_SAMPLE_BITS_32, 


AVS_CS_BGR48 = AVS_CS_RGB_TYPE | AVS_CS_BGR | AVS_CS_INTERLEAVED | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_BGR64 = AVS_CS_RGBA_TYPE | AVS_CS_BGR | AVS_CS_INTERLEAVED | AVS_CS_SAMPLE_BITS_16, 



AVS_CS_RGBP = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_RGBP10 = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_RGBP12 = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_RGBP14 = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_RGBP16 = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_RGBPS = AVS_CS_GENERIC_RGBP | AVS_CS_SAMPLE_BITS_32, 


AVS_CS_RGBAP = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_RGBAP10 = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_RGBAP12 = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_RGBAP14 = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_RGBAP16 = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_RGBAPS = AVS_CS_GENERIC_RGBAP | AVS_CS_SAMPLE_BITS_32, 


AVS_CS_YUVA444 = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_YUVA422 = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_8, 
AVS_CS_YUVA420 = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_8, 

AVS_CS_YUVA444P10 = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_YUVA422P10 = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_10, 
AVS_CS_YUVA420P10 = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_10, 

AVS_CS_YUVA444P12 = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_YUVA422P12 = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_12, 
AVS_CS_YUVA420P12 = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_12, 

AVS_CS_YUVA444P14 = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_YUVA422P14 = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_14, 
AVS_CS_YUVA420P14 = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_14, 

AVS_CS_YUVA444P16 = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_YUVA422P16 = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_16, 
AVS_CS_YUVA420P16 = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_16, 

AVS_CS_YUVA444PS = AVS_CS_GENERIC_YUVA444 | AVS_CS_SAMPLE_BITS_32, 
AVS_CS_YUVA422PS = AVS_CS_GENERIC_YUVA422 | AVS_CS_SAMPLE_BITS_32, 
AVS_CS_YUVA420PS = AVS_CS_GENERIC_YUVA420 | AVS_CS_SAMPLE_BITS_32, 

};

enum {
AVS_IT_BFF = 1<<0,
AVS_IT_TFF = 1<<1,
AVS_IT_FIELDBASED = 1<<2};

enum {
AVS_FILTER_TYPE=1,
AVS_FILTER_INPUT_COLORSPACE=2,
AVS_FILTER_OUTPUT_TYPE=9,
AVS_FILTER_NAME=4,
AVS_FILTER_AUTHOR=5,
AVS_FILTER_VERSION=6,
AVS_FILTER_ARGS=7,
AVS_FILTER_ARGS_INFO=8,
AVS_FILTER_ARGS_DESCRIPTION=10,
AVS_FILTER_DESCRIPTION=11};

enum { 
AVS_FILTER_TYPE_AUDIO=1,
AVS_FILTER_TYPE_VIDEO=2,
AVS_FILTER_OUTPUT_TYPE_SAME=3,
AVS_FILTER_OUTPUT_TYPE_DIFFERENT=4};

enum {

AVS_CACHE_NOTHING=10, 
AVS_CACHE_WINDOW=11, 
AVS_CACHE_GENERIC=12, 
AVS_CACHE_FORCE_GENERIC=13, 

AVS_CACHE_GET_POLICY=30, 
AVS_CACHE_GET_WINDOW=31, 
AVS_CACHE_GET_RANGE=32, 

AVS_CACHE_AUDIO=50, 
AVS_CACHE_AUDIO_NOTHING=51, 
AVS_CACHE_AUDIO_NONE=52, 
AVS_CACHE_AUDIO_AUTO=53, 

AVS_CACHE_GET_AUDIO_POLICY=70, 
AVS_CACHE_GET_AUDIO_SIZE=71, 

AVS_CACHE_PREFETCH_FRAME=100, 
AVS_CACHE_PREFETCH_GO=101, 

AVS_CACHE_PREFETCH_AUDIO_BEGIN=120, 
AVS_CACHE_PREFETCH_AUDIO_STARTLO=121, 
AVS_CACHE_PREFETCH_AUDIO_STARTHI=122, 
AVS_CACHE_PREFETCH_AUDIO_COUNT=123, 
AVS_CACHE_PREFETCH_AUDIO_COMMIT=124, 
AVS_CACHE_PREFETCH_AUDIO_GO=125, 

AVS_CACHE_GETCHILD_CACHE_MODE=200, 
AVS_CACHE_GETCHILD_CACHE_SIZE=201, 
AVS_CACHE_GETCHILD_AUDIO_MODE=202, 
AVS_CACHE_GETCHILD_AUDIO_SIZE=203, 

AVS_CACHE_GETCHILD_COST=220, 
AVS_CACHE_COST_ZERO=221, 
AVS_CACHE_COST_UNIT=222, 
AVS_CACHE_COST_LOW=223, 
AVS_CACHE_COST_MED=224, 
AVS_CACHE_COST_HI=225, 

AVS_CACHE_GETCHILD_THREAD_MODE=240, 
AVS_CACHE_THREAD_UNSAFE=241, 
AVS_CACHE_THREAD_CLASS=242, 
AVS_CACHE_THREAD_SAFE=243, 
AVS_CACHE_THREAD_OWN=244, 

AVS_CACHE_GETCHILD_ACCESS_COST=260, 
AVS_CACHE_ACCESS_RAND=261, 
AVS_CACHE_ACCESS_SEQ0=262, 
AVS_CACHE_ACCESS_SEQ1=263, 
};

#if defined(BUILDING_AVSCORE)
AVSValue create_c_video_filter(AVSValue args, void * user_data, IScriptEnvironment * e0);

struct AVS_ScriptEnvironment {
IScriptEnvironment * env;
const char * error;
AVS_ScriptEnvironment(IScriptEnvironment * e = 0)
: env(e), error(0) {}
};
#endif

typedef struct AVS_Clip AVS_Clip;
typedef struct AVS_ScriptEnvironment AVS_ScriptEnvironment;







typedef struct AVS_VideoInfo {
int width, height; 
unsigned fps_numerator, fps_denominator;
int num_frames;

int pixel_type;

int audio_samples_per_second; 
int sample_type;
INT64 num_audio_samples;
int nchannels;



int image_type;
} AVS_VideoInfo;


AVSC_INLINE int avs_has_video(const AVS_VideoInfo * p)
{ return (p->width!=0); }

AVSC_INLINE int avs_has_audio(const AVS_VideoInfo * p)
{ return (p->audio_samples_per_second!=0); }

AVSC_INLINE int avs_is_rgb(const AVS_VideoInfo * p)
{ return !!(p->pixel_type&AVS_CS_BGR); }

AVSC_INLINE int avs_is_rgb24(const AVS_VideoInfo * p)
{ return ((p->pixel_type&AVS_CS_BGR24)==AVS_CS_BGR24) && ((p->pixel_type & AVS_CS_SAMPLE_BITS_MASK) == AVS_CS_SAMPLE_BITS_8); }

AVSC_INLINE int avs_is_rgb32(const AVS_VideoInfo * p)
{ return ((p->pixel_type&AVS_CS_BGR32)==AVS_CS_BGR32) && ((p->pixel_type & AVS_CS_SAMPLE_BITS_MASK) == AVS_CS_SAMPLE_BITS_8); }

AVSC_INLINE int avs_is_yuv(const AVS_VideoInfo * p)
{ return !!(p->pixel_type&AVS_CS_YUV ); }

AVSC_INLINE int avs_is_yuy2(const AVS_VideoInfo * p)
{ return (p->pixel_type & AVS_CS_YUY2) == AVS_CS_YUY2; }

AVSC_API(int, avs_is_yv24)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yv16)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yv12)(const AVS_VideoInfo * p) ; 

AVSC_API(int, avs_is_yv411)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_y8)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_get_plane_width_subsampling)(const AVS_VideoInfo * p, int plane);

AVSC_API(int, avs_get_plane_height_subsampling)(const AVS_VideoInfo * p, int plane);

AVSC_API(int, avs_bits_per_pixel)(const AVS_VideoInfo * p);

AVSC_API(int, avs_bytes_from_pixels)(const AVS_VideoInfo * p, int pixels);

AVSC_API(int, avs_row_size)(const AVS_VideoInfo * p, int plane);

AVSC_API(int, avs_bmp_size)(const AVS_VideoInfo * vi);

AVSC_API(int, avs_is_color_space)(const AVS_VideoInfo * p, int c_space);


AVSC_INLINE int avs_is_property(const AVS_VideoInfo * p, int property)
{
return ((p->image_type & property) == property);
}

AVSC_INLINE int avs_is_planar(const AVS_VideoInfo * p)
{
return !!(p->pixel_type & AVS_CS_PLANAR);
}

AVSC_INLINE int avs_is_field_based(const AVS_VideoInfo * p)
{
return !!(p->image_type & AVS_IT_FIELDBASED);
}

AVSC_INLINE int avs_is_parity_known(const AVS_VideoInfo * p)
{
return ((p->image_type & AVS_IT_FIELDBASED) && (p->image_type & (AVS_IT_BFF | AVS_IT_TFF)));
}

AVSC_INLINE int avs_is_bff(const AVS_VideoInfo * p)
{
return !!(p->image_type & AVS_IT_BFF);
}

AVSC_INLINE int avs_is_tff(const AVS_VideoInfo * p)
{
return !!(p->image_type & AVS_IT_TFF);
}

AVSC_INLINE int avs_samples_per_second(const AVS_VideoInfo * p)
{ return p->audio_samples_per_second; }

AVSC_INLINE int avs_bytes_per_channel_sample(const AVS_VideoInfo * p)
{
switch (p->sample_type) {
case AVS_SAMPLE_INT8: return sizeof(signed char);
case AVS_SAMPLE_INT16: return sizeof(signed short);
case AVS_SAMPLE_INT24: return 3;
case AVS_SAMPLE_INT32: return sizeof(signed int);
case AVS_SAMPLE_FLOAT: return sizeof(float);
default: return 0;
}
}

AVSC_INLINE int avs_bytes_per_audio_sample(const AVS_VideoInfo * p)
{ return p->nchannels*avs_bytes_per_channel_sample(p);}

AVSC_INLINE INT64 avs_audio_samples_from_frames(const AVS_VideoInfo * p, INT64 frames)
{ return ((INT64)(frames) * p->audio_samples_per_second * p->fps_denominator / p->fps_numerator); }

AVSC_INLINE int avs_frames_from_audio_samples(const AVS_VideoInfo * p, INT64 samples)
{ return (int)(samples * (INT64)p->fps_numerator / (INT64)p->fps_denominator / (INT64)p->audio_samples_per_second); }

AVSC_INLINE INT64 avs_audio_samples_from_bytes(const AVS_VideoInfo * p, INT64 bytes)
{ return bytes / avs_bytes_per_audio_sample(p); }

AVSC_INLINE INT64 avs_bytes_from_audio_samples(const AVS_VideoInfo * p, INT64 samples)
{ return samples * avs_bytes_per_audio_sample(p); }

AVSC_INLINE int avs_audio_channels(const AVS_VideoInfo * p)
{ return p->nchannels; }

AVSC_INLINE int avs_sample_type(const AVS_VideoInfo * p)
{ return p->sample_type;}


AVSC_INLINE void avs_set_property(AVS_VideoInfo * p, int property)
{ p->image_type|=property; }

AVSC_INLINE void avs_clear_property(AVS_VideoInfo * p, int property)
{ p->image_type&=~property; }

AVSC_INLINE void avs_set_field_based(AVS_VideoInfo * p, int isfieldbased)
{ if (isfieldbased) p->image_type|=AVS_IT_FIELDBASED; else p->image_type&=~AVS_IT_FIELDBASED; }

AVSC_INLINE void avs_set_fps(AVS_VideoInfo * p, unsigned numerator, unsigned denominator)
{
unsigned x=numerator, y=denominator;
while (y) { 
unsigned t = x%y; x = y; y = t;
}
p->fps_numerator = numerator/x;
p->fps_denominator = denominator/x;
}

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE int avs_is_same_colorspace(const AVS_VideoInfo * x, const AVS_VideoInfo * y)
{
return (x->pixel_type == y->pixel_type)
|| (avs_is_yv12(x) && avs_is_yv12(y));
}
#endif


AVSC_API(int, avs_is_rgb48)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_rgb64)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_yuv444p16)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yuv422p16)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yuv420p16)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_y16)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yuv444ps)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yuv422ps)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_yuv420ps)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_y32)(const AVS_VideoInfo * p); 

AVSC_API(int, avs_is_444)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_422)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_420)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_y)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_yuva)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_planar_rgb)(const AVS_VideoInfo * p);

AVSC_API(int, avs_is_planar_rgba)(const AVS_VideoInfo * p);

AVSC_API(int, avs_num_components)(const AVS_VideoInfo * p);

AVSC_API(int, avs_component_size)(const AVS_VideoInfo * p);

AVSC_API(int, avs_bits_per_component)(const AVS_VideoInfo * p);















typedef struct AVS_VideoFrameBuffer {
BYTE * data;
#if defined(SIZETMOD)
size_t data_size;
#else
int data_size;
#endif


volatile long sequence_number;

volatile long refcount;
} AVS_VideoFrameBuffer;





typedef struct AVS_VideoFrame {
volatile long refcount;
AVS_VideoFrameBuffer * vfb;
#if defined(SIZETMOD)
size_t offset;
#else
int offset;
#endif
int pitch, row_size, height;
#if defined(SIZETMOD)
size_t offsetU, offsetV;
#else
int offsetU, offsetV;
#endif
int pitchUV; 
int row_sizeUV, heightUV; 



#if defined(SIZETMOD)
size_t offsetA;
#else
int offsetA;
#endif
int pitchA, row_sizeA; 
} AVS_VideoFrame;


AVSC_API(int, avs_get_pitch_p)(const AVS_VideoFrame * p, int plane);

AVSC_API(int, avs_get_row_size_p)(const AVS_VideoFrame * p, int plane);

AVSC_API(int, avs_get_height_p)(const AVS_VideoFrame * p, int plane);

AVSC_API(const BYTE *, avs_get_read_ptr_p)(const AVS_VideoFrame * p, int plane);

AVSC_API(int, avs_is_writable)(const AVS_VideoFrame * p);

AVSC_API(BYTE *, avs_get_write_ptr_p)(const AVS_VideoFrame * p, int plane);

AVSC_API(void, avs_release_video_frame)(AVS_VideoFrame *);

AVSC_API(AVS_VideoFrame *, avs_copy_video_frame)(AVS_VideoFrame *);


#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE int avs_get_pitch(const AVS_VideoFrame * p) {
return avs_get_pitch_p(p, 0);
}
#endif

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE int avs_get_row_size(const AVS_VideoFrame * p) {
return avs_get_row_size_p(p, 0); }
#endif


#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE int avs_get_height(const AVS_VideoFrame * p) {
return avs_get_height_p(p, 0);
}
#endif

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE const BYTE* avs_get_read_ptr(const AVS_VideoFrame * p) {
return avs_get_read_ptr_p(p, 0);}
#endif

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE BYTE* avs_get_write_ptr(const AVS_VideoFrame * p) {
return avs_get_write_ptr_p(p, 0);}
#endif

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE void avs_release_frame(AVS_VideoFrame * f)
{avs_release_video_frame(f);}
#endif

#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE AVS_VideoFrame * avs_copy_frame(AVS_VideoFrame * f)
{return avs_copy_video_frame(f);}
#endif















typedef struct AVS_Value AVS_Value;
struct AVS_Value {
short type; 

short array_size;
union {
void * clip; 
char boolean;
int integer;
float floating_pt;
const char * string;
const AVS_Value * array;
} d;
};





static const AVS_Value avs_void = {'v'};

AVSC_API(void, avs_copy_value)(AVS_Value * dest, AVS_Value src);
AVSC_API(void, avs_release_value)(AVS_Value);
AVSC_API(AVS_Clip *, avs_take_clip)(AVS_Value, AVS_ScriptEnvironment *);
AVSC_API(void, avs_set_to_clip)(AVS_Value *, AVS_Clip *);



AVSC_INLINE int avs_defined(AVS_Value v) { return v.type != 'v'; }
AVSC_INLINE int avs_is_clip(AVS_Value v) { return v.type == 'c'; }
AVSC_INLINE int avs_is_bool(AVS_Value v) { return v.type == 'b'; }
AVSC_INLINE int avs_is_int(AVS_Value v) { return v.type == 'i'; }
AVSC_INLINE int avs_is_float(AVS_Value v) { return v.type == 'f' || v.type == 'i'; }
AVSC_INLINE int avs_is_string(AVS_Value v) { return v.type == 's'; }
AVSC_INLINE int avs_is_array(AVS_Value v) { return v.type == 'a'; }
AVSC_INLINE int avs_is_error(AVS_Value v) { return v.type == 'e'; }

AVSC_INLINE int avs_as_bool(AVS_Value v)
{ return v.d.boolean; }
AVSC_INLINE int avs_as_int(AVS_Value v)
{ return v.d.integer; }
AVSC_INLINE const char * avs_as_string(AVS_Value v)
{ return avs_is_error(v) || avs_is_string(v) ? v.d.string : 0; }
AVSC_INLINE double avs_as_float(AVS_Value v)
{ return avs_is_int(v) ? v.d.integer : v.d.floating_pt; }
AVSC_INLINE const char * avs_as_error(AVS_Value v)
{ return avs_is_error(v) ? v.d.string : 0; }
AVSC_INLINE const AVS_Value * avs_as_array(AVS_Value v)
{ return v.d.array; }
AVSC_INLINE int avs_array_size(AVS_Value v)
{ return avs_is_array(v) ? v.array_size : 1; }
AVSC_INLINE AVS_Value avs_array_elt(AVS_Value v, int index)
{ return avs_is_array(v) ? v.d.array[index] : v; }



AVSC_INLINE AVS_Value avs_new_value_bool(int v0)
{ AVS_Value v; v.type = 'b'; v.d.boolean = v0 == 0 ? 0 : 1; return v; }
AVSC_INLINE AVS_Value avs_new_value_int(int v0)
{ AVS_Value v; v.type = 'i'; v.d.integer = v0; return v; }
AVSC_INLINE AVS_Value avs_new_value_string(const char * v0)
{ AVS_Value v; v.type = 's'; v.d.string = v0; return v; }
AVSC_INLINE AVS_Value avs_new_value_float(float v0)
{ AVS_Value v; v.type = 'f'; v.d.floating_pt = v0; return v;}
AVSC_INLINE AVS_Value avs_new_value_error(const char * v0)
{ AVS_Value v; v.type = 'e'; v.d.string = v0; return v; }
#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE AVS_Value avs_new_value_clip(AVS_Clip * v0)
{ AVS_Value v; avs_set_to_clip(&v, v0); return v; }
#endif
AVSC_INLINE AVS_Value avs_new_value_array(AVS_Value * v0, int size)
{ AVS_Value v; v.type = 'a'; v.d.array = v0; v.array_size = (short)size; return v; }







AVSC_API(void, avs_release_clip)(AVS_Clip *);
AVSC_API(AVS_Clip *, avs_copy_clip)(AVS_Clip *);

AVSC_API(const char *, avs_clip_get_error)(AVS_Clip *); 

AVSC_API(const AVS_VideoInfo *, avs_get_video_info)(AVS_Clip *);

AVSC_API(int, avs_get_version)(AVS_Clip *);

AVSC_API(AVS_VideoFrame *, avs_get_frame)(AVS_Clip *, int n);


AVSC_API(int, avs_get_parity)(AVS_Clip *, int n);


AVSC_API(int, avs_get_audio)(AVS_Clip *, void * buf,
INT64 start, INT64 count);


AVSC_API(int, avs_set_cache_hints)(AVS_Clip *,
int cachehints, int frame_range);


typedef AVS_Value (AVSC_CC * AVS_ApplyFunc)
(AVS_ScriptEnvironment *, AVS_Value args, void * user_data);

typedef struct AVS_FilterInfo AVS_FilterInfo;
struct AVS_FilterInfo
{

AVS_Clip * child;
AVS_VideoInfo vi;
AVS_ScriptEnvironment * env;
AVS_VideoFrame * (AVSC_CC * get_frame)(AVS_FilterInfo *, int n);
int (AVSC_CC * get_parity)(AVS_FilterInfo *, int n);
int (AVSC_CC * get_audio)(AVS_FilterInfo *, void * buf,
INT64 start, INT64 count);
int (AVSC_CC * set_cache_hints)(AVS_FilterInfo *, int cachehints,
int frame_range);
void (AVSC_CC * free_filter)(AVS_FilterInfo *);



const char * error;

void * user_data;
};








AVSC_API(AVS_Clip *, avs_new_c_filter)(AVS_ScriptEnvironment * e,
AVS_FilterInfo * * fi,
AVS_Value child, int store_child);







enum {

AVS_CPU_FORCE = 0x01, 
AVS_CPU_FPU = 0x02, 
AVS_CPU_MMX = 0x04, 
AVS_CPU_INTEGER_SSE = 0x08, 
AVS_CPU_SSE = 0x10, 
AVS_CPU_SSE2 = 0x20, 
AVS_CPU_3DNOW = 0x40, 
AVS_CPU_3DNOW_EXT = 0x80, 
AVS_CPU_X86_64 = 0xA0, 

AVS_CPUF_SSE3 = 0x100, 
AVS_CPUF_SSSE3 = 0x200, 
AVS_CPUF_SSE4 = 0x400, 
AVS_CPUF_SSE4_1 = 0x400,
AVS_CPUF_AVX = 0x800, 
AVS_CPUF_SSE4_2 = 0x1000, 

AVS_CPUF_AVX2 = 0x2000, 
AVS_CPUF_FMA3 = 0x4000,
AVS_CPUF_F16C = 0x8000,
AVS_CPUF_MOVBE = 0x10000, 
AVS_CPUF_POPCNT = 0x20000,
AVS_CPUF_AES = 0x40000,
AVS_CPUF_FMA4 = 0x80000,

AVS_CPUF_AVX512F = 0x100000, 
AVS_CPUF_AVX512DQ = 0x200000, 
AVS_CPUF_AVX512PF = 0x400000, 
AVS_CPUF_AVX512ER = 0x800000, 
AVS_CPUF_AVX512CD = 0x1000000, 
AVS_CPUF_AVX512BW = 0x2000000, 
AVS_CPUF_AVX512VL = 0x4000000, 
AVS_CPUF_AVX512IFMA = 0x8000000, 
AVS_CPUF_AVX512VBMI = 0x10000000 
};


AVSC_API(const char *, avs_get_error)(AVS_ScriptEnvironment *); 

AVSC_API(int, avs_get_cpu_flags)(AVS_ScriptEnvironment *);
AVSC_API(int, avs_check_version)(AVS_ScriptEnvironment *, int version);

AVSC_API(char *, avs_save_string)(AVS_ScriptEnvironment *, const char* s, int length);
AVSC_API(char *, avs_sprintf)(AVS_ScriptEnvironment *, const char * fmt, ...);

AVSC_API(char *, avs_vsprintf)(AVS_ScriptEnvironment *, const char * fmt, void* val);


AVSC_API(int, avs_add_function)(AVS_ScriptEnvironment *,
const char * name, const char * params,
AVS_ApplyFunc apply, void * user_data);

AVSC_API(int, avs_function_exists)(AVS_ScriptEnvironment *, const char * name);

AVSC_API(AVS_Value, avs_invoke)(AVS_ScriptEnvironment *, const char * name,
AVS_Value args, const char** arg_names);


AVSC_API(AVS_Value, avs_get_var)(AVS_ScriptEnvironment *, const char* name);


AVSC_API(int, avs_set_var)(AVS_ScriptEnvironment *, const char* name, AVS_Value val);

AVSC_API(int, avs_set_global_var)(AVS_ScriptEnvironment *, const char* name, const AVS_Value val);




AVSC_API(AVS_VideoFrame *, avs_new_video_frame_a)(AVS_ScriptEnvironment *,
const AVS_VideoInfo * vi, int align);




#if !defined(AVSC_NO_DECLSPEC)

AVSC_INLINE AVS_VideoFrame * avs_new_video_frame(AVS_ScriptEnvironment * env,
const AVS_VideoInfo * vi)
{return avs_new_video_frame_a(env,vi,AVS_FRAME_ALIGN);}



AVSC_INLINE AVS_VideoFrame * avs_new_frame(AVS_ScriptEnvironment * env,
const AVS_VideoInfo * vi)
{return avs_new_video_frame_a(env,vi,AVS_FRAME_ALIGN);}
#endif


AVSC_API(int, avs_make_writable)(AVS_ScriptEnvironment *, AVS_VideoFrame * * pvf);

AVSC_API(void, avs_bit_blt)(AVS_ScriptEnvironment *, BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);

typedef void (AVSC_CC *AVS_ShutdownFunc)(void* user_data, AVS_ScriptEnvironment * env);
AVSC_API(void, avs_at_exit)(AVS_ScriptEnvironment *, AVS_ShutdownFunc function, void * user_data);

AVSC_API(AVS_VideoFrame *, avs_subframe)(AVS_ScriptEnvironment *, AVS_VideoFrame * src, int rel_offset, int new_pitch, int new_row_size, int new_height);


AVSC_API(int, avs_set_memory_max)(AVS_ScriptEnvironment *, int mem);

AVSC_API(int, avs_set_working_dir)(AVS_ScriptEnvironment *, const char * newdir);



AVSC_API(AVS_ScriptEnvironment *, avs_create_script_environment)(int version);



AVSC_EXPORT
const char * AVSC_CC avisynth_c_plugin_init(AVS_ScriptEnvironment* env);


AVSC_API(void, avs_delete_script_environment)(AVS_ScriptEnvironment *);


AVSC_API(AVS_VideoFrame *, avs_subframe_planar)(AVS_ScriptEnvironment *, AVS_VideoFrame * src, int rel_offset, int new_pitch, int new_row_size, int new_height, int rel_offsetU, int rel_offsetV, int new_pitchUV);


#if defined(AVSC_NO_DECLSPEC)
















typedef struct AVS_Library AVS_Library;

#define AVSC_DECLARE_FUNC(name) name##_func name









struct AVS_Library {
HMODULE handle;

AVSC_DECLARE_FUNC(avs_add_function);
AVSC_DECLARE_FUNC(avs_at_exit);
AVSC_DECLARE_FUNC(avs_bit_blt);
AVSC_DECLARE_FUNC(avs_check_version);
AVSC_DECLARE_FUNC(avs_clip_get_error);
AVSC_DECLARE_FUNC(avs_copy_clip);
AVSC_DECLARE_FUNC(avs_copy_value);
AVSC_DECLARE_FUNC(avs_copy_video_frame);
AVSC_DECLARE_FUNC(avs_create_script_environment);
AVSC_DECLARE_FUNC(avs_delete_script_environment);
AVSC_DECLARE_FUNC(avs_function_exists);
AVSC_DECLARE_FUNC(avs_get_audio);
AVSC_DECLARE_FUNC(avs_get_cpu_flags);
AVSC_DECLARE_FUNC(avs_get_frame);
AVSC_DECLARE_FUNC(avs_get_parity);
AVSC_DECLARE_FUNC(avs_get_var);
AVSC_DECLARE_FUNC(avs_get_version);
AVSC_DECLARE_FUNC(avs_get_video_info);
AVSC_DECLARE_FUNC(avs_invoke);
AVSC_DECLARE_FUNC(avs_make_writable);
AVSC_DECLARE_FUNC(avs_new_c_filter);
AVSC_DECLARE_FUNC(avs_new_video_frame_a);
AVSC_DECLARE_FUNC(avs_release_clip);
AVSC_DECLARE_FUNC(avs_release_value);
AVSC_DECLARE_FUNC(avs_release_video_frame);
AVSC_DECLARE_FUNC(avs_save_string);
AVSC_DECLARE_FUNC(avs_set_cache_hints);
AVSC_DECLARE_FUNC(avs_set_global_var);
AVSC_DECLARE_FUNC(avs_set_memory_max);
AVSC_DECLARE_FUNC(avs_set_to_clip);
AVSC_DECLARE_FUNC(avs_set_var);
AVSC_DECLARE_FUNC(avs_set_working_dir);
AVSC_DECLARE_FUNC(avs_sprintf);
AVSC_DECLARE_FUNC(avs_subframe);
AVSC_DECLARE_FUNC(avs_subframe_planar);
AVSC_DECLARE_FUNC(avs_take_clip);
AVSC_DECLARE_FUNC(avs_vsprintf);

AVSC_DECLARE_FUNC(avs_get_error);
AVSC_DECLARE_FUNC(avs_is_yv24);
AVSC_DECLARE_FUNC(avs_is_yv16);
AVSC_DECLARE_FUNC(avs_is_yv12);
AVSC_DECLARE_FUNC(avs_is_yv411);
AVSC_DECLARE_FUNC(avs_is_y8);
AVSC_DECLARE_FUNC(avs_is_color_space);

AVSC_DECLARE_FUNC(avs_get_plane_width_subsampling);
AVSC_DECLARE_FUNC(avs_get_plane_height_subsampling);
AVSC_DECLARE_FUNC(avs_bits_per_pixel);
AVSC_DECLARE_FUNC(avs_bytes_from_pixels);
AVSC_DECLARE_FUNC(avs_row_size);
AVSC_DECLARE_FUNC(avs_bmp_size);
AVSC_DECLARE_FUNC(avs_get_pitch_p);
AVSC_DECLARE_FUNC(avs_get_row_size_p);
AVSC_DECLARE_FUNC(avs_get_height_p);
AVSC_DECLARE_FUNC(avs_get_read_ptr_p);
AVSC_DECLARE_FUNC(avs_is_writable);
AVSC_DECLARE_FUNC(avs_get_write_ptr_p);



AVSC_DECLARE_FUNC(avs_is_rgb48);
AVSC_DECLARE_FUNC(avs_is_rgb64);
AVSC_DECLARE_FUNC(avs_is_yuv444p16);
AVSC_DECLARE_FUNC(avs_is_yuv422p16);
AVSC_DECLARE_FUNC(avs_is_yuv420p16);
AVSC_DECLARE_FUNC(avs_is_y16);
AVSC_DECLARE_FUNC(avs_is_yuv444ps);
AVSC_DECLARE_FUNC(avs_is_yuv422ps);
AVSC_DECLARE_FUNC(avs_is_yuv420ps);
AVSC_DECLARE_FUNC(avs_is_y32);
AVSC_DECLARE_FUNC(avs_is_444);
AVSC_DECLARE_FUNC(avs_is_422);
AVSC_DECLARE_FUNC(avs_is_420);
AVSC_DECLARE_FUNC(avs_is_y);
AVSC_DECLARE_FUNC(avs_is_yuva);
AVSC_DECLARE_FUNC(avs_is_planar_rgb);
AVSC_DECLARE_FUNC(avs_is_planar_rgba);
AVSC_DECLARE_FUNC(avs_num_components);
AVSC_DECLARE_FUNC(avs_component_size);
AVSC_DECLARE_FUNC(avs_bits_per_component);


};

#undef AVSC_DECLARE_FUNC



AVSC_INLINE int avs_is_xx_fallback_return_false(const AVS_VideoInfo * p)
{
return 0;
}


AVSC_INLINE int avs_num_components_fallback(const AVS_VideoInfo * p)
{
switch (p->pixel_type) {
case AVS_CS_UNKNOWN:
return 0;
case AVS_CS_RAW32:
case AVS_CS_Y8:
return 1;
case AVS_CS_BGR32:
return 4; 
default:
return 3;
}
}


AVSC_INLINE int avs_component_size_fallback(const AVS_VideoInfo * p)
{
return 1;
}


AVSC_INLINE int avs_bits_per_component_fallback(const AVS_VideoInfo * p)
{
return 8;
}






AVSC_INLINE AVS_Library * avs_load_library() {
AVS_Library *library = (AVS_Library *)malloc(sizeof(AVS_Library));
if (library == NULL)
return NULL;
library->handle = LoadLibraryA("avisynth");
if (library->handle == NULL)
goto fail;

#define __AVSC_STRINGIFY(x) #x
#define AVSC_STRINGIFY(x) __AVSC_STRINGIFY(x)
#define AVSC_LOAD_FUNC(name) {library->name = (name##_func) GetProcAddress(library->handle, AVSC_STRINGIFY(name));if (library->name == NULL)goto fail;}





#if 0






























#define AVSC_LOAD_FUNC_FALLBACK(name,name2) {library->name = (name##_func) GetProcAddress(library->handle, AVSC_STRINGIFY(name));if (library->name == NULL)library->name = (name##_func) GetProcAddress(library->handle, AVSC_STRINGIFY(name2));if (library->name == NULL)goto fail;}








#define AVSC_LOAD_FUNC_FALLBACK_SIMULATED(name,name2) {library->name = (name##_func) GetProcAddress(library->handle, AVSC_STRINGIFY(name));if (library->name == NULL)library->name = name2;if (library->name == NULL)goto fail;}






#endif

AVSC_LOAD_FUNC(avs_add_function);
AVSC_LOAD_FUNC(avs_at_exit);
AVSC_LOAD_FUNC(avs_bit_blt);
AVSC_LOAD_FUNC(avs_check_version);
AVSC_LOAD_FUNC(avs_clip_get_error);
AVSC_LOAD_FUNC(avs_copy_clip);
AVSC_LOAD_FUNC(avs_copy_value);
AVSC_LOAD_FUNC(avs_copy_video_frame);
AVSC_LOAD_FUNC(avs_create_script_environment);
AVSC_LOAD_FUNC(avs_delete_script_environment);
AVSC_LOAD_FUNC(avs_function_exists);
AVSC_LOAD_FUNC(avs_get_audio);
AVSC_LOAD_FUNC(avs_get_cpu_flags);
AVSC_LOAD_FUNC(avs_get_frame);
AVSC_LOAD_FUNC(avs_get_parity);
AVSC_LOAD_FUNC(avs_get_var);
AVSC_LOAD_FUNC(avs_get_version);
AVSC_LOAD_FUNC(avs_get_video_info);
AVSC_LOAD_FUNC(avs_invoke);
AVSC_LOAD_FUNC(avs_make_writable);
AVSC_LOAD_FUNC(avs_new_c_filter);
AVSC_LOAD_FUNC(avs_new_video_frame_a);
AVSC_LOAD_FUNC(avs_release_clip);
AVSC_LOAD_FUNC(avs_release_value);
AVSC_LOAD_FUNC(avs_release_video_frame);
AVSC_LOAD_FUNC(avs_save_string);
AVSC_LOAD_FUNC(avs_set_cache_hints);
AVSC_LOAD_FUNC(avs_set_global_var);
AVSC_LOAD_FUNC(avs_set_memory_max);
AVSC_LOAD_FUNC(avs_set_to_clip);
AVSC_LOAD_FUNC(avs_set_var);
AVSC_LOAD_FUNC(avs_set_working_dir);
AVSC_LOAD_FUNC(avs_sprintf);
AVSC_LOAD_FUNC(avs_subframe);
AVSC_LOAD_FUNC(avs_subframe_planar);
AVSC_LOAD_FUNC(avs_take_clip);
AVSC_LOAD_FUNC(avs_vsprintf);

AVSC_LOAD_FUNC(avs_get_error);
AVSC_LOAD_FUNC(avs_is_yv24);
AVSC_LOAD_FUNC(avs_is_yv16);
AVSC_LOAD_FUNC(avs_is_yv12);
AVSC_LOAD_FUNC(avs_is_yv411);
AVSC_LOAD_FUNC(avs_is_y8);
AVSC_LOAD_FUNC(avs_is_color_space);

AVSC_LOAD_FUNC(avs_get_plane_width_subsampling);
AVSC_LOAD_FUNC(avs_get_plane_height_subsampling);
AVSC_LOAD_FUNC(avs_bits_per_pixel);
AVSC_LOAD_FUNC(avs_bytes_from_pixels);
AVSC_LOAD_FUNC(avs_row_size);
AVSC_LOAD_FUNC(avs_bmp_size);
AVSC_LOAD_FUNC(avs_get_pitch_p);
AVSC_LOAD_FUNC(avs_get_row_size_p);
AVSC_LOAD_FUNC(avs_get_height_p);
AVSC_LOAD_FUNC(avs_get_read_ptr_p);
AVSC_LOAD_FUNC(avs_is_writable);
AVSC_LOAD_FUNC(avs_get_write_ptr_p);

#if 0

AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_rgb48, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_rgb64, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv444p16, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv422p16, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv420p16, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_y16, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv444ps, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv422ps, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuv420ps, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_y32, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK(avs_is_444, avs_is_yv24);
AVSC_LOAD_FUNC_FALLBACK(avs_is_422, avs_is_yv16);
AVSC_LOAD_FUNC_FALLBACK(avs_is_420, avs_is_yv12);
AVSC_LOAD_FUNC_FALLBACK(avs_is_y, avs_is_y8);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_yuva, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_planar_rgb, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_is_planar_rgba, avs_is_xx_fallback_return_false);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_num_components, avs_num_components_fallback);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_component_size, avs_component_size_fallback);
AVSC_LOAD_FUNC_FALLBACK_SIMULATED(avs_bits_per_component, avs_bits_per_component_fallback);
#endif

#undef __AVSC_STRINGIFY
#undef AVSC_STRINGIFY
#undef AVSC_LOAD_FUNC
#undef AVSC_LOAD_FUNC_FALLBACK
#undef AVSC_LOAD_FUNC_FALLBACK_SIMULATED

return library;

fail:
free(library);
return NULL;
}

AVSC_INLINE void avs_free_library(AVS_Library *library) {
if (library == NULL)
return;
FreeLibrary(library->handle);
free(library);
}
#endif

#endif
