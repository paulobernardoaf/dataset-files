#if defined(__cplusplus)
extern "C" {
#endif
#include "allegro5/allegro.h"
#if (defined ALLEGRO_MINGW32) || (defined ALLEGRO_MSVC) || (defined ALLEGRO_BCC32)
#if !defined(ALLEGRO_STATICLINK)
#if defined(ALLEGRO_KCM_AUDIO_SRC)
#define _ALLEGRO_KCM_AUDIO_DLL __declspec(dllexport)
#else
#define _ALLEGRO_KCM_AUDIO_DLL __declspec(dllimport)
#endif
#else
#define _ALLEGRO_KCM_AUDIO_DLL
#endif
#endif
#if defined ALLEGRO_MSVC
#define ALLEGRO_KCM_AUDIO_FUNC(type, name, args) _ALLEGRO_KCM_AUDIO_DLL type __cdecl name args
#elif defined ALLEGRO_MINGW32
#define ALLEGRO_KCM_AUDIO_FUNC(type, name, args) extern type name args
#elif defined ALLEGRO_BCC32
#define ALLEGRO_KCM_AUDIO_FUNC(type, name, args) extern _ALLEGRO_KCM_AUDIO_DLL type name args
#else
#define ALLEGRO_KCM_AUDIO_FUNC AL_FUNC
#endif
enum ALLEGRO_AUDIO_EVENT_TYPE
{
_KCM_STREAM_FEEDER_QUIT_EVENT_TYPE = 512,
ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT = 513,
ALLEGRO_EVENT_AUDIO_STREAM_FINISHED = 514,
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
ALLEGRO_EVENT_AUDIO_RECORDER_FRAGMENT = 515,
#endif
};
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
typedef struct ALLEGRO_AUDIO_RECORDER_EVENT ALLEGRO_AUDIO_RECORDER_EVENT;
struct ALLEGRO_AUDIO_RECORDER_EVENT
{
_AL_EVENT_HEADER(struct ALLEGRO_AUDIO_RECORDER)
struct ALLEGRO_USER_EVENT_DESCRIPTOR *__internal__descr;
void *buffer;
unsigned int samples;
};
#endif
enum ALLEGRO_AUDIO_DEPTH
{
ALLEGRO_AUDIO_DEPTH_INT8 = 0x00,
ALLEGRO_AUDIO_DEPTH_INT16 = 0x01,
ALLEGRO_AUDIO_DEPTH_INT24 = 0x02,
ALLEGRO_AUDIO_DEPTH_FLOAT32 = 0x03,
ALLEGRO_AUDIO_DEPTH_UNSIGNED = 0x08,
ALLEGRO_AUDIO_DEPTH_UINT8 = ALLEGRO_AUDIO_DEPTH_INT8 |
ALLEGRO_AUDIO_DEPTH_UNSIGNED,
ALLEGRO_AUDIO_DEPTH_UINT16 = ALLEGRO_AUDIO_DEPTH_INT16 |
ALLEGRO_AUDIO_DEPTH_UNSIGNED,
ALLEGRO_AUDIO_DEPTH_UINT24 = ALLEGRO_AUDIO_DEPTH_INT24 |
ALLEGRO_AUDIO_DEPTH_UNSIGNED
};
enum ALLEGRO_CHANNEL_CONF
{
ALLEGRO_CHANNEL_CONF_1 = 0x10,
ALLEGRO_CHANNEL_CONF_2 = 0x20,
ALLEGRO_CHANNEL_CONF_3 = 0x30,
ALLEGRO_CHANNEL_CONF_4 = 0x40,
ALLEGRO_CHANNEL_CONF_5_1 = 0x51,
ALLEGRO_CHANNEL_CONF_6_1 = 0x61,
ALLEGRO_CHANNEL_CONF_7_1 = 0x71
#define ALLEGRO_MAX_CHANNELS 8
};
enum ALLEGRO_PLAYMODE
{
ALLEGRO_PLAYMODE_ONCE = 0x100,
ALLEGRO_PLAYMODE_LOOP = 0x101,
ALLEGRO_PLAYMODE_BIDIR = 0x102,
_ALLEGRO_PLAYMODE_STREAM_ONCE = 0x103, 
_ALLEGRO_PLAYMODE_STREAM_ONEDIR = 0x104 
};
enum ALLEGRO_MIXER_QUALITY
{
ALLEGRO_MIXER_QUALITY_POINT = 0x110,
ALLEGRO_MIXER_QUALITY_LINEAR = 0x111,
ALLEGRO_MIXER_QUALITY_CUBIC = 0x112
};
#define ALLEGRO_AUDIO_PAN_NONE (-1000.0f)
typedef struct ALLEGRO_SAMPLE ALLEGRO_SAMPLE;
typedef struct ALLEGRO_SAMPLE_ID ALLEGRO_SAMPLE_ID;
struct ALLEGRO_SAMPLE_ID {
int _index;
int _id;
};
typedef struct ALLEGRO_SAMPLE_INSTANCE ALLEGRO_SAMPLE_INSTANCE;
typedef struct ALLEGRO_AUDIO_STREAM ALLEGRO_AUDIO_STREAM;
typedef struct ALLEGRO_MIXER ALLEGRO_MIXER;
typedef struct ALLEGRO_VOICE ALLEGRO_VOICE;
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
typedef struct ALLEGRO_AUDIO_RECORDER ALLEGRO_AUDIO_RECORDER;
#endif
#if !defined(__cplusplus)
typedef enum ALLEGRO_AUDIO_DEPTH ALLEGRO_AUDIO_DEPTH;
typedef enum ALLEGRO_CHANNEL_CONF ALLEGRO_CHANNEL_CONF;
typedef enum ALLEGRO_PLAYMODE ALLEGRO_PLAYMODE;
typedef enum ALLEGRO_MIXER_QUALITY ALLEGRO_MIXER_QUALITY;
#endif
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE *, al_create_sample, (void *buf,
unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth,
ALLEGRO_CHANNEL_CONF chan_conf, bool free_buf));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_sample, (ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE_INSTANCE*, al_create_sample_instance, (
ALLEGRO_SAMPLE *data));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_sample_instance, (
ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_sample_frequency, (const ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_sample_length, (const ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, al_get_sample_depth, (const ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, al_get_sample_channels, (const ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(void *, al_get_sample_data, (const ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_sample_instance_frequency, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_sample_instance_length, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_sample_instance_position, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_sample_instance_speed, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_sample_instance_gain, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_sample_instance_pan, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_sample_instance_time, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, al_get_sample_instance_depth, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, al_get_sample_instance_channels, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_PLAYMODE, al_get_sample_instance_playmode, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_sample_instance_playing, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_sample_instance_attached, (const ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_position, (ALLEGRO_SAMPLE_INSTANCE *spl, unsigned int val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_length, (ALLEGRO_SAMPLE_INSTANCE *spl, unsigned int val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_speed, (ALLEGRO_SAMPLE_INSTANCE *spl, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_gain, (ALLEGRO_SAMPLE_INSTANCE *spl, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_pan, (ALLEGRO_SAMPLE_INSTANCE *spl, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_playmode, (ALLEGRO_SAMPLE_INSTANCE *spl, ALLEGRO_PLAYMODE val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_playing, (ALLEGRO_SAMPLE_INSTANCE *spl, bool val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_detach_sample_instance, (ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample, (ALLEGRO_SAMPLE_INSTANCE *spl, ALLEGRO_SAMPLE *data));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE *, al_get_sample, (ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_play_sample_instance, (ALLEGRO_SAMPLE_INSTANCE *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_stop_sample_instance, (ALLEGRO_SAMPLE_INSTANCE *spl));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_sample_instance_channel_matrix, (ALLEGRO_SAMPLE_INSTANCE *spl, const float *matrix));
#endif
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_STREAM*, al_create_audio_stream, (size_t buffer_count,
unsigned int samples, unsigned int freq,
ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_audio_stream, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(void, al_drain_audio_stream, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_audio_stream_frequency, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_audio_stream_length, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_audio_stream_fragments, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_available_audio_stream_fragments, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_audio_stream_speed, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_audio_stream_gain, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_audio_stream_pan, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, al_get_audio_stream_channels, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, al_get_audio_stream_depth, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_PLAYMODE, al_get_audio_stream_playmode, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_audio_stream_playing, (const ALLEGRO_AUDIO_STREAM *spl));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_audio_stream_attached, (const ALLEGRO_AUDIO_STREAM *spl));
ALLEGRO_KCM_AUDIO_FUNC(uint64_t, al_get_audio_stream_played_samples, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(void *, al_get_audio_stream_fragment, (const ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_speed, (ALLEGRO_AUDIO_STREAM *stream, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_gain, (ALLEGRO_AUDIO_STREAM *stream, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_pan, (ALLEGRO_AUDIO_STREAM *stream, float val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_playmode, (ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_PLAYMODE val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_playing, (ALLEGRO_AUDIO_STREAM *stream, bool val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_detach_audio_stream, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_fragment, (ALLEGRO_AUDIO_STREAM *stream, void *val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_rewind_audio_stream, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_seek_audio_stream_secs, (ALLEGRO_AUDIO_STREAM *stream, double time));
ALLEGRO_KCM_AUDIO_FUNC(double, al_get_audio_stream_position_secs, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(double, al_get_audio_stream_length_secs, (ALLEGRO_AUDIO_STREAM *stream));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_loop_secs, (ALLEGRO_AUDIO_STREAM *stream, double start, double end));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_audio_stream_event_source, (ALLEGRO_AUDIO_STREAM *stream));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_audio_stream_channel_matrix, (ALLEGRO_AUDIO_STREAM *stream, const float *matrix));
#endif
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_MIXER*, al_create_mixer, (unsigned int freq,
ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_mixer, (ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_sample_instance_to_mixer, (
ALLEGRO_SAMPLE_INSTANCE *stream, ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_audio_stream_to_mixer, (ALLEGRO_AUDIO_STREAM *stream,
ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_mixer_to_mixer, (ALLEGRO_MIXER *stream,
ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_mixer_postprocess_callback, (
ALLEGRO_MIXER *mixer,
void (*cb)(void *buf, unsigned int samples, void *data),
void *data));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_mixer_frequency, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, al_get_mixer_channels, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, al_get_mixer_depth, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_MIXER_QUALITY, al_get_mixer_quality, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(float, al_get_mixer_gain, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_mixer_playing, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_mixer_attached, (const ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_mixer_frequency, (ALLEGRO_MIXER *mixer, unsigned int val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_mixer_quality, (ALLEGRO_MIXER *mixer, ALLEGRO_MIXER_QUALITY val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_mixer_gain, (ALLEGRO_MIXER *mixer, float gain));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_mixer_playing, (ALLEGRO_MIXER *mixer, bool val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_detach_mixer, (ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_VOICE*, al_create_voice, (unsigned int freq,
ALLEGRO_AUDIO_DEPTH depth,
ALLEGRO_CHANNEL_CONF chan_conf));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_voice, (ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_sample_instance_to_voice, (
ALLEGRO_SAMPLE_INSTANCE *stream, ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_audio_stream_to_voice, (
ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_VOICE *voice ));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_attach_mixer_to_voice, (ALLEGRO_MIXER *mixer,
ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(void, al_detach_voice, (ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_voice_frequency, (const ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(unsigned int, al_get_voice_position, (const ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, al_get_voice_channels, (const ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, al_get_voice_depth, (const ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_get_voice_playing, (const ALLEGRO_VOICE *voice));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_voice_position, (ALLEGRO_VOICE *voice, unsigned int val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_voice_playing, (ALLEGRO_VOICE *voice, bool val));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_install_audio, (void));
ALLEGRO_KCM_AUDIO_FUNC(void, al_uninstall_audio, (void));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_is_audio_installed, (void));
ALLEGRO_KCM_AUDIO_FUNC(uint32_t, al_get_allegro_audio_version, (void));
ALLEGRO_KCM_AUDIO_FUNC(size_t, al_get_channel_count, (ALLEGRO_CHANNEL_CONF conf));
ALLEGRO_KCM_AUDIO_FUNC(size_t, al_get_audio_depth_size, (ALLEGRO_AUDIO_DEPTH conf));
ALLEGRO_KCM_AUDIO_FUNC(void, al_fill_silence, (void *buf, unsigned int samples,
ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_reserve_samples, (int reserve_samples));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_MIXER *, al_get_default_mixer, (void));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_set_default_mixer, (ALLEGRO_MIXER *mixer));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_restore_default_mixer, (void));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_play_sample, (ALLEGRO_SAMPLE *data,
float gain, float pan, float speed, ALLEGRO_PLAYMODE loop, ALLEGRO_SAMPLE_ID *ret_id));
ALLEGRO_KCM_AUDIO_FUNC(void, al_stop_sample, (ALLEGRO_SAMPLE_ID *spl_id));
ALLEGRO_KCM_AUDIO_FUNC(void, al_stop_samples, (void));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_VOICE *, al_get_default_voice, (void));
ALLEGRO_KCM_AUDIO_FUNC(void, al_set_default_voice, (ALLEGRO_VOICE *voice));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE_INSTANCE*, al_lock_sample_id, (ALLEGRO_SAMPLE_ID *spl_id));
ALLEGRO_KCM_AUDIO_FUNC(void, al_unlock_sample_id, (ALLEGRO_SAMPLE_ID *spl_id));
#endif
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_sample_loader, (const char *ext,
ALLEGRO_SAMPLE *(*loader)(const char *filename)));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_sample_saver, (const char *ext,
bool (*saver)(const char *filename, ALLEGRO_SAMPLE *spl)));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_audio_stream_loader, (const char *ext,
ALLEGRO_AUDIO_STREAM *(*stream_loader)(const char *filename,
size_t buffer_count, unsigned int samples)));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_sample_loader_f, (const char *ext,
ALLEGRO_SAMPLE *(*loader)(ALLEGRO_FILE *fp)));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_sample_saver_f, (const char *ext,
bool (*saver)(ALLEGRO_FILE *fp, ALLEGRO_SAMPLE *spl)));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_register_audio_stream_loader_f, (const char *ext,
ALLEGRO_AUDIO_STREAM *(*stream_loader)(ALLEGRO_FILE *fp,
size_t buffer_count, unsigned int samples)));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE *, al_load_sample, (const char *filename));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_save_sample, (const char *filename,
ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_STREAM *, al_load_audio_stream, (const char *filename,
size_t buffer_count, unsigned int samples));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_SAMPLE *, al_load_sample_f, (ALLEGRO_FILE* fp, const char *ident));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_save_sample_f, (ALLEGRO_FILE* fp, const char *ident,
ALLEGRO_SAMPLE *spl));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_STREAM *, al_load_audio_stream_f, (ALLEGRO_FILE* fp, const char *ident,
size_t buffer_count, unsigned int samples));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_KCM_AUDIO_SRC)
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_RECORDER *, al_create_audio_recorder, (size_t fragment_count,
unsigned int samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_start_audio_recorder, (ALLEGRO_AUDIO_RECORDER *r));
ALLEGRO_KCM_AUDIO_FUNC(void, al_stop_audio_recorder, (ALLEGRO_AUDIO_RECORDER *r));
ALLEGRO_KCM_AUDIO_FUNC(bool, al_is_audio_recorder_recording, (ALLEGRO_AUDIO_RECORDER *r));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_EVENT_SOURCE *, al_get_audio_recorder_event_source,
(ALLEGRO_AUDIO_RECORDER *r));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_RECORDER_EVENT *, al_get_audio_recorder_event, (ALLEGRO_EVENT *event));
ALLEGRO_KCM_AUDIO_FUNC(void, al_destroy_audio_recorder, (ALLEGRO_AUDIO_RECORDER *r));
#endif
#if defined(__cplusplus)
} 
#endif
