#define MM_PASSTHROUGH_DISABLED 0
#define MM_PASSTHROUGH_ENABLED 1
#define MM_PASSTHROUGH_ENABLED_HD 2
#define MM_PASSTHROUGH_DEFAULT MM_PASSTHROUGH_DISABLED
typedef struct aout_stream aout_stream_t;
struct aout_stream
{
struct vlc_object_t obj;
void *sys;
void (*stop)(aout_stream_t *);
HRESULT (*time_get)(aout_stream_t *, vlc_tick_t *);
HRESULT (*play)(aout_stream_t *, block_t *, vlc_tick_t);
HRESULT (*pause)(aout_stream_t *, bool);
HRESULT (*flush)(aout_stream_t *);
struct
{
void *device;
HRESULT (*activate)(void *device, REFIID, PROPVARIANT *, void **);
} owner;
};
typedef HRESULT (*aout_stream_start_t)(aout_stream_t *s,
audio_sample_format_t *fmt, const GUID *sid);
static inline void aout_stream_Stop(aout_stream_t *s)
{
(s->stop)(s);
}
static inline HRESULT aout_stream_TimeGet(aout_stream_t *s, vlc_tick_t *delay)
{
return (s->time_get)(s, delay);
}
static inline HRESULT aout_stream_Play(aout_stream_t *s, block_t *block,
vlc_tick_t date)
{
return (s->play)(s, block, date);
}
static inline HRESULT aout_stream_Pause(aout_stream_t *s, bool paused)
{
return (s->pause)(s, paused);
}
static inline HRESULT aout_stream_Flush(aout_stream_t *s)
{
return (s->flush)(s);
}
static inline
HRESULT aout_stream_Activate(aout_stream_t *s, REFIID iid,
PROPVARIANT *actparms, void **pv)
{
return s->owner.activate(s->owner.device, iid, actparms, pv);
}
