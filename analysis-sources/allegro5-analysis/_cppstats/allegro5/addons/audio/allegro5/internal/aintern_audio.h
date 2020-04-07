#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_list.h"
#include "allegro5/internal/aintern_vector.h"
#include "../allegro_audio.h"
struct ALLEGRO_AUDIO_RECORDER {
ALLEGRO_EVENT_SOURCE source;
ALLEGRO_THREAD *thread;
ALLEGRO_MUTEX *mutex;
ALLEGRO_COND *cond;
ALLEGRO_AUDIO_DEPTH depth;
ALLEGRO_CHANNEL_CONF chan_conf;
unsigned int frequency;
void **fragments;
unsigned int fragment_count;
unsigned int samples;
size_t fragment_size;
unsigned int sample_size;
volatile bool is_recording; 
void *extra;
};
typedef enum ALLEGRO_AUDIO_DRIVER_ENUM
{
ALLEGRO_AUDIO_DRIVER_AUTODETECT = 0x20000,
ALLEGRO_AUDIO_DRIVER_OPENAL = 0x20001,
ALLEGRO_AUDIO_DRIVER_ALSA = 0x20002,
ALLEGRO_AUDIO_DRIVER_DSOUND = 0x20003,
ALLEGRO_AUDIO_DRIVER_OSS = 0x20004,
ALLEGRO_AUDIO_DRIVER_AQUEUE = 0x20005,
ALLEGRO_AUDIO_DRIVER_PULSEAUDIO = 0x20006,
ALLEGRO_AUDIO_DRIVER_OPENSL = 0x20007,
ALLEGRO_AUDIO_DRIVER_SDL = 0x20008
} ALLEGRO_AUDIO_DRIVER_ENUM;
typedef struct ALLEGRO_AUDIO_DRIVER ALLEGRO_AUDIO_DRIVER;
struct ALLEGRO_AUDIO_DRIVER {
const char *specifier;
int (*open)(void);
void (*close)(void);
int (*allocate_voice)(ALLEGRO_VOICE*);
void (*deallocate_voice)(ALLEGRO_VOICE*);
int (*load_voice)(ALLEGRO_VOICE*, const void*);
void (*unload_voice)(ALLEGRO_VOICE*);
int (*start_voice)(ALLEGRO_VOICE*);
int (*stop_voice)(ALLEGRO_VOICE*);
bool (*voice_is_playing)(const ALLEGRO_VOICE*);
unsigned int (*get_voice_position)(const ALLEGRO_VOICE*);
int (*set_voice_position)(ALLEGRO_VOICE*, unsigned int);
int (*allocate_recorder)(struct ALLEGRO_AUDIO_RECORDER *);
void (*deallocate_recorder)(struct ALLEGRO_AUDIO_RECORDER *);
};
extern ALLEGRO_AUDIO_DRIVER *_al_kcm_driver;
const void *_al_voice_update(ALLEGRO_VOICE *voice, ALLEGRO_MUTEX *mutex,
unsigned int *samples);
bool _al_kcm_set_voice_playing(ALLEGRO_VOICE *voice, ALLEGRO_MUTEX *mutex,
bool val);
struct ALLEGRO_VOICE {
ALLEGRO_AUDIO_DEPTH depth;
ALLEGRO_CHANNEL_CONF chan_conf;
unsigned int frequency;
size_t buffer_size;
size_t num_buffers;
ALLEGRO_SAMPLE_INSTANCE *attached_stream;
bool is_streaming;
ALLEGRO_MUTEX *mutex;
ALLEGRO_COND *cond;
_AL_LIST_ITEM *dtor_item;
ALLEGRO_AUDIO_DRIVER *driver;
void *extra;
};
typedef union {
float *f32;
uint32_t *u24;
int32_t *s24;
uint16_t *u16;
int16_t *s16;
uint8_t *u8;
int8_t *s8;
void *ptr;
} any_buffer_t;
struct ALLEGRO_SAMPLE {
ALLEGRO_AUDIO_DEPTH depth;
ALLEGRO_CHANNEL_CONF chan_conf;
unsigned int frequency;
int len;
any_buffer_t buffer;
bool free_buf;
_AL_LIST_ITEM *dtor_item;
};
typedef void (*stream_reader_t)(void *source, void **vbuf,
unsigned int *samples, ALLEGRO_AUDIO_DEPTH buffer_depth, size_t dest_maxc);
typedef struct {
union {
ALLEGRO_MIXER *mixer;
ALLEGRO_VOICE *voice;
void *ptr;
} u;
bool is_voice;
} sample_parent_t;
struct ALLEGRO_SAMPLE_INSTANCE {
ALLEGRO_EVENT_SOURCE es;
ALLEGRO_SAMPLE spl_data;
volatile bool is_playing;
ALLEGRO_PLAYMODE loop;
float speed;
float gain;
float pan;
int pos;
int pos_bresenham_error;
int loop_start;
int loop_end;
int step;
int step_denom;
float *matrix;
bool is_mixer;
stream_reader_t spl_read;
ALLEGRO_MUTEX *mutex;
sample_parent_t parent;
_AL_LIST_ITEM *dtor_item;
};
void _al_kcm_destroy_sample(ALLEGRO_SAMPLE_INSTANCE *sample, bool unregister);
void _al_kcm_stream_set_mutex(ALLEGRO_SAMPLE_INSTANCE *stream, ALLEGRO_MUTEX *mutex);
void _al_kcm_detach_from_parent(ALLEGRO_SAMPLE_INSTANCE *spl);
typedef size_t (*stream_callback_t)(ALLEGRO_AUDIO_STREAM *, void *, size_t);
typedef void (*unload_feeder_t)(ALLEGRO_AUDIO_STREAM *);
typedef bool (*rewind_feeder_t)(ALLEGRO_AUDIO_STREAM *);
typedef bool (*seek_feeder_t)(ALLEGRO_AUDIO_STREAM *, double);
typedef double (*get_feeder_position_t)(ALLEGRO_AUDIO_STREAM *);
typedef double (*get_feeder_length_t)(ALLEGRO_AUDIO_STREAM *);
typedef bool (*set_feeder_loop_t)(ALLEGRO_AUDIO_STREAM *, double, double);
struct ALLEGRO_AUDIO_STREAM {
ALLEGRO_SAMPLE_INSTANCE spl;
unsigned int buf_count;
void *main_buffer;
void **pending_bufs;
void **used_bufs;
volatile bool is_draining;
uint64_t consumed_fragments;
ALLEGRO_THREAD *feed_thread;
ALLEGRO_MUTEX *feed_thread_started_mutex;
ALLEGRO_COND *feed_thread_started_cond;
bool feed_thread_started;
volatile bool quit_feed_thread;
unload_feeder_t unload_feeder;
rewind_feeder_t rewind_feeder;
seek_feeder_t seek_feeder;
get_feeder_position_t get_feeder_position;
get_feeder_length_t get_feeder_length;
set_feeder_loop_t set_feeder_loop;
stream_callback_t feeder;
_AL_LIST_ITEM *dtor_item;
void *extra;
};
bool _al_kcm_refill_stream(ALLEGRO_AUDIO_STREAM *stream);
typedef void (*postprocess_callback_t)(void *buf, unsigned int samples,
void *userdata);
struct ALLEGRO_MIXER {
ALLEGRO_SAMPLE_INSTANCE ss;
ALLEGRO_MIXER_QUALITY quality;
postprocess_callback_t postprocess_callback;
void *pp_callback_userdata;
_AL_VECTOR streams;
_AL_LIST_ITEM *dtor_item;
};
extern void _al_kcm_mixer_rejig_sample_matrix(ALLEGRO_MIXER *mixer,
ALLEGRO_SAMPLE_INSTANCE *spl);
extern void _al_kcm_mixer_read(void *source, void **buf, unsigned int *samples,
ALLEGRO_AUDIO_DEPTH buffer_depth, size_t dest_maxc);
typedef enum {
ALLEGRO_NO_ERROR = 0,
ALLEGRO_INVALID_PARAM = 1,
ALLEGRO_INVALID_OBJECT = 2,
ALLEGRO_GENERIC_ERROR = 255
} AL_ERROR_ENUM;
extern void _al_set_error(int error, char* string);
ALLEGRO_KCM_AUDIO_FUNC(void*, _al_kcm_feed_stream, (ALLEGRO_THREAD *self, void *vstream));
void _al_kcm_emit_stream_events(ALLEGRO_AUDIO_STREAM *stream);
void _al_kcm_init_destructors(void);
void _al_kcm_shutdown_destructors(void);
_AL_LIST_ITEM *_al_kcm_register_destructor(char const *name, void *object,
void (*func)(void*));
void _al_kcm_unregister_destructor(_AL_LIST_ITEM *dtor_item);
void _al_kcm_foreach_destructor(
void (*callback)(void *object, void (*func)(void *), void *udata),
void *userdata);
ALLEGRO_KCM_AUDIO_FUNC(void, _al_kcm_shutdown_default_mixer, (void));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_CHANNEL_CONF, _al_count_to_channel_conf, (int num_channels));
ALLEGRO_KCM_AUDIO_FUNC(ALLEGRO_AUDIO_DEPTH, _al_word_size_to_depth_conf, (int word_size));
ALLEGRO_KCM_AUDIO_FUNC(void, _al_emit_audio_event, (int event_type));
