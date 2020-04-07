#include "windowsPorts/windows2linux.h"
#include <stdarg.h>
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#define AVSC_USE_STDCALL 1
#if !defined(AVSC_USE_STDCALL)
#define AVSC_CC __cdecl
#else
#define AVSC_CC __stdcall
#endif
#define AVSC_INLINE static __inline
#if defined(AVISYNTH_C_EXPORTS)
#define AVSC_EXPORT EXTERN_C
#define AVSC_API(ret, name) EXTERN_C __declspec(dllexport) ret AVSC_CC name
#else
#define AVSC_EXPORT EXTERN_C __declspec(dllexport)
#if !defined(AVSC_NO_DECLSPEC)
#define AVSC_API(ret, name) EXTERN_C __declspec(dllimport) ret AVSC_CC name
#else
#define AVSC_API(ret, name) typedef ret (AVSC_CC *name##_func)
#endif
#endif
#if defined(__GNUC__)
typedef long long int INT64;
#else
typedef __int64 INT64;
#endif
#if !defined(__AVXSYNTH_H__)
enum { AVISYNTH_INTERFACE_VERSION = 3 };
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
AVS_PLANAR_V_ALIGNED=AVS_PLANAR_V|AVS_PLANAR_ALIGNED};
enum {AVS_CS_BGR = 1<<28,
AVS_CS_YUV = 1<<29,
AVS_CS_INTERLEAVED = 1<<30,
AVS_CS_PLANAR = 1<<31};
enum {
AVS_CS_UNKNOWN = 0,
AVS_CS_BGR24 = 1<<0 | AVS_CS_BGR | AVS_CS_INTERLEAVED,
AVS_CS_BGR32 = 1<<1 | AVS_CS_BGR | AVS_CS_INTERLEAVED,
AVS_CS_YUY2 = 1<<2 | AVS_CS_YUV | AVS_CS_INTERLEAVED,
AVS_CS_YV12 = 1<<3 | AVS_CS_YUV | AVS_CS_PLANAR, 
AVS_CS_I420 = 1<<4 | AVS_CS_YUV | AVS_CS_PLANAR, 
AVS_CS_IYUV = 1<<4 | AVS_CS_YUV | AVS_CS_PLANAR 
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
AVS_CACHE_NOTHING=0,
AVS_CACHE_RANGE=1,
AVS_CACHE_ALL=2,
AVS_CACHE_AUDIO=3,
AVS_CACHE_AUDIO_NONE=4,
AVS_CACHE_AUDIO_AUTO=5
};
#define AVS_FRAME_ALIGN 16
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
{ return (p->pixel_type&AVS_CS_BGR24)==AVS_CS_BGR24; } 
AVSC_INLINE int avs_is_rgb32(const AVS_VideoInfo * p)
{ return (p->pixel_type & AVS_CS_BGR32) == AVS_CS_BGR32 ; }
AVSC_INLINE int avs_is_yuv(const AVS_VideoInfo * p)
{ return !!(p->pixel_type&AVS_CS_YUV ); }
AVSC_INLINE int avs_is_yuy2(const AVS_VideoInfo * p)
{ return (p->pixel_type & AVS_CS_YUY2) == AVS_CS_YUY2; }
AVSC_INLINE int avs_is_yv12(const AVS_VideoInfo * p)
{ return ((p->pixel_type & AVS_CS_YV12) == AVS_CS_YV12)||((p->pixel_type & AVS_CS_I420) == AVS_CS_I420); }
AVSC_INLINE int avs_is_color_space(const AVS_VideoInfo * p, int c_space)
{ return ((p->pixel_type & c_space) == c_space); }
AVSC_INLINE int avs_is_property(const AVS_VideoInfo * p, int property)
{ return ((p->pixel_type & property)==property ); }
AVSC_INLINE int avs_is_planar(const AVS_VideoInfo * p)
{ return !!(p->pixel_type & AVS_CS_PLANAR); }
AVSC_INLINE int avs_is_field_based(const AVS_VideoInfo * p)
{ return !!(p->image_type & AVS_IT_FIELDBASED); }
AVSC_INLINE int avs_is_parity_known(const AVS_VideoInfo * p)
{ return ((p->image_type & AVS_IT_FIELDBASED)&&(p->image_type & (AVS_IT_BFF | AVS_IT_TFF))); }
AVSC_INLINE int avs_is_bff(const AVS_VideoInfo * p)
{ return !!(p->image_type & AVS_IT_BFF); }
AVSC_INLINE int avs_is_tff(const AVS_VideoInfo * p)
{ return !!(p->image_type & AVS_IT_TFF); }
AVSC_INLINE int avs_bits_per_pixel(const AVS_VideoInfo * p)
{
switch (p->pixel_type) {
case AVS_CS_BGR24: return 24;
case AVS_CS_BGR32: return 32;
case AVS_CS_YUY2: return 16;
case AVS_CS_YV12:
case AVS_CS_I420: return 12;
default: return 0;
}
}
AVSC_INLINE int avs_bytes_from_pixels(const AVS_VideoInfo * p, int pixels)
{ return pixels * (avs_bits_per_pixel(p)>>3); } 
AVSC_INLINE int avs_row_size(const AVS_VideoInfo * p)
{ return avs_bytes_from_pixels(p,p->width); } 
AVSC_INLINE int avs_bmp_size(const AVS_VideoInfo * vi)
{ if (avs_is_planar(vi)) {int p = vi->height * ((avs_row_size(vi)+3) & ~3); p+=p>>1; return p; } return vi->height * ((avs_row_size(vi)+3) & ~3); }
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
AVSC_INLINE int avs_is_same_colorspace(AVS_VideoInfo * x, AVS_VideoInfo * y)
{
return (x->pixel_type == y->pixel_type)
|| (avs_is_yv12(x) && avs_is_yv12(y));
}
typedef struct AVS_VideoFrameBuffer {
unsigned char * data;
int data_size;
long sequence_number;
long refcount;
} AVS_VideoFrameBuffer;
typedef struct AVS_VideoFrame {
int refcount;
AVS_VideoFrameBuffer * vfb;
int offset, pitch, row_size, height, offsetU, offsetV, pitchUV; 
} AVS_VideoFrame;
AVSC_INLINE int avs_get_pitch(const AVS_VideoFrame * p) {
return p->pitch;}
AVSC_INLINE int avs_get_pitch_p(const AVS_VideoFrame * p, int plane) {
switch (plane) {
case AVS_PLANAR_U: case AVS_PLANAR_V: return p->pitchUV;}
return p->pitch;}
AVSC_INLINE int avs_get_row_size(const AVS_VideoFrame * p) {
return p->row_size; }
AVSC_INLINE int avs_get_row_size_p(const AVS_VideoFrame * p, int plane) {
int r;
switch (plane) {
case AVS_PLANAR_U: case AVS_PLANAR_V:
if (p->pitchUV) return p->row_size>>1;
else return 0;
case AVS_PLANAR_U_ALIGNED: case AVS_PLANAR_V_ALIGNED:
if (p->pitchUV) {
r = ((p->row_size+AVS_FRAME_ALIGN-1)&(~(AVS_FRAME_ALIGN-1)) )>>1; 
if (r < p->pitchUV)
return r;
return p->row_size>>1;
} else return 0;
case AVS_PLANAR_Y_ALIGNED:
r = (p->row_size+AVS_FRAME_ALIGN-1)&(~(AVS_FRAME_ALIGN-1)); 
if (r <= p->pitch)
return r;
return p->row_size;
}
return p->row_size;
}
AVSC_INLINE int avs_get_height(const AVS_VideoFrame * p) {
return p->height;}
AVSC_INLINE int avs_get_height_p(const AVS_VideoFrame * p, int plane) {
switch (plane) {
case AVS_PLANAR_U: case AVS_PLANAR_V:
if (p->pitchUV) return p->height>>1;
return 0;
}
return p->height;}
AVSC_INLINE const unsigned char* avs_get_read_ptr(const AVS_VideoFrame * p) {
return p->vfb->data + p->offset;}
AVSC_INLINE const unsigned char* avs_get_read_ptr_p(const AVS_VideoFrame * p, int plane)
{
switch (plane) {
case AVS_PLANAR_U: return p->vfb->data + p->offsetU;
case AVS_PLANAR_V: return p->vfb->data + p->offsetV;
default: return p->vfb->data + p->offset;}
}
AVSC_INLINE int avs_is_writable(const AVS_VideoFrame * p) {
return (p->refcount == 1 && p->vfb->refcount == 1);}
AVSC_INLINE unsigned char* avs_get_write_ptr(const AVS_VideoFrame * p)
{
if (avs_is_writable(p)) {
++p->vfb->sequence_number;
return p->vfb->data + p->offset;
} else
return 0;
}
AVSC_INLINE unsigned char* avs_get_write_ptr_p(const AVS_VideoFrame * p, int plane)
{
if (plane==AVS_PLANAR_Y && avs_is_writable(p)) {
++p->vfb->sequence_number;
return p->vfb->data + p->offset;
} else if (plane==AVS_PLANAR_Y) {
return 0;
} else {
switch (plane) {
case AVS_PLANAR_U: return p->vfb->data + p->offsetU;
case AVS_PLANAR_V: return p->vfb->data + p->offsetV;
default: return p->vfb->data + p->offset;
}
}
}
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(void, avs_release_video_frame)(AVS_VideoFrame *);
AVSC_API(AVS_VideoFrame *, avs_copy_video_frame)(AVS_VideoFrame *);
#if defined __cplusplus
}
#endif 
#if !defined(AVSC_NO_DECLSPEC)
AVSC_INLINE void avs_release_frame(AVS_VideoFrame * f)
{avs_release_video_frame(f);}
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
INT64 integer64; 
float floating_pt;
const char * string;
const AVS_Value * array;
} d;
};
static const AVS_Value avs_void = {'v'};
AVSC_API(void, avs_copy_value)(AVS_Value * dest, AVS_Value src);
AVSC_API(void, avs_release_value)(AVS_Value);
AVSC_INLINE int avs_defined(AVS_Value v) { return v.type != 'v'; }
AVSC_INLINE int avs_is_clip(AVS_Value v) { return v.type == 'c'; }
AVSC_INLINE int avs_is_bool(AVS_Value v) { return v.type == 'b'; }
AVSC_INLINE int avs_is_int(AVS_Value v) { return v.type == 'i'; }
AVSC_INLINE int avs_is_float(AVS_Value v) { return v.type == 'f' || v.type == 'i'; }
AVSC_INLINE int avs_is_string(AVS_Value v) { return v.type == 's'; }
AVSC_INLINE int avs_is_array(AVS_Value v) { return v.type == 'a'; }
AVSC_INLINE int avs_is_error(AVS_Value v) { return v.type == 'e'; }
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(AVS_Clip *, avs_take_clip)(AVS_Value, AVS_ScriptEnvironment *);
AVSC_API(void, avs_set_to_clip)(AVS_Value *, AVS_Clip *);
#if defined __cplusplus
}
#endif 
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
{ AVS_Value v = {0}; v.type = 'b'; v.d.boolean = v0 == 0 ? 0 : 1; return v; }
AVSC_INLINE AVS_Value avs_new_value_int(int v0)
{ AVS_Value v = {0}; v.type = 'i'; v.d.integer = v0; return v; }
AVSC_INLINE AVS_Value avs_new_value_string(const char * v0)
{ AVS_Value v = {0}; v.type = 's'; v.d.string = v0; return v; }
AVSC_INLINE AVS_Value avs_new_value_float(float v0)
{ AVS_Value v = {0}; v.type = 'f'; v.d.floating_pt = v0; return v;}
AVSC_INLINE AVS_Value avs_new_value_error(const char * v0)
{ AVS_Value v = {0}; v.type = 'e'; v.d.string = v0; return v; }
#if !defined(AVSC_NO_DECLSPEC)
AVSC_INLINE AVS_Value avs_new_value_clip(AVS_Clip * v0)
{ AVS_Value v = {0}; avs_set_to_clip(&v, v0); return v; }
#endif
AVSC_INLINE AVS_Value avs_new_value_array(AVS_Value * v0, int size)
{ AVS_Value v = {0}; v.type = 'a'; v.d.array = v0; v.array_size = size; return v; }
#if defined __cplusplus
extern "C"
{
#endif 
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
int cachehints, size_t frame_range);
#if defined __cplusplus
}
#endif 
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
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(AVS_Clip *, avs_new_c_filter)(AVS_ScriptEnvironment * e,
AVS_FilterInfo * * fi,
AVS_Value child, int store_child);
#if defined __cplusplus
}
#endif 
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
};
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(const char *, avs_get_error)(AVS_ScriptEnvironment *); 
AVSC_API(long, avs_get_cpu_flags)(AVS_ScriptEnvironment *);
AVSC_API(int, avs_check_version)(AVS_ScriptEnvironment *, int version);
AVSC_API(char *, avs_save_string)(AVS_ScriptEnvironment *, const char* s, int length);
AVSC_API(char *, avs_sprintf)(AVS_ScriptEnvironment *, const char * fmt, ...);
AVSC_API(char *, avs_vsprintf)(AVS_ScriptEnvironment *, const char * fmt, va_list val);
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
#if defined __cplusplus
}
#endif 
#if !defined(AVSC_NO_DECLSPEC)
AVSC_INLINE
AVS_VideoFrame * avs_new_video_frame(AVS_ScriptEnvironment * env,
const AVS_VideoInfo * vi)
{return avs_new_video_frame_a(env,vi,AVS_FRAME_ALIGN);}
AVSC_INLINE
AVS_VideoFrame * avs_new_frame(AVS_ScriptEnvironment * env,
const AVS_VideoInfo * vi)
{return avs_new_video_frame_a(env,vi,AVS_FRAME_ALIGN);}
#endif
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(int, avs_make_writable)(AVS_ScriptEnvironment *, AVS_VideoFrame * * pvf);
AVSC_API(void, avs_bit_blt)(AVS_ScriptEnvironment *, unsigned char* dstp, int dst_pitch, const unsigned char* srcp, int src_pitch, int row_size, int height);
typedef void (AVSC_CC *AVS_ShutdownFunc)(void* user_data, AVS_ScriptEnvironment * env);
AVSC_API(void, avs_at_exit)(AVS_ScriptEnvironment *, AVS_ShutdownFunc function, void * user_data);
AVSC_API(AVS_VideoFrame *, avs_subframe)(AVS_ScriptEnvironment *, AVS_VideoFrame * src, int rel_offset, int new_pitch, int new_row_size, int new_height);
AVSC_API(int, avs_set_memory_max)(AVS_ScriptEnvironment *, int mem);
AVSC_API(int, avs_set_working_dir)(AVS_ScriptEnvironment *, const char * newdir);
AVSC_API(AVS_ScriptEnvironment *, avs_create_script_environment)(int version);
#if defined __cplusplus
}
#endif 
AVSC_EXPORT
const char * AVSC_CC avisynth_c_plugin_init(AVS_ScriptEnvironment* env);
#if defined __cplusplus
extern "C"
{
#endif 
AVSC_API(void, avs_delete_script_environment)(AVS_ScriptEnvironment *);
AVSC_API(AVS_VideoFrame *, avs_subframe_planar)(AVS_ScriptEnvironment *, AVS_VideoFrame * src, int rel_offset, int new_pitch, int new_row_size, int new_height, int rel_offsetU, int rel_offsetV, int new_pitchUV);
#if defined __cplusplus
}
#endif 
