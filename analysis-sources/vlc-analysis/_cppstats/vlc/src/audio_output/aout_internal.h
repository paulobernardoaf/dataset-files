#include <stdatomic.h>
#include <vlc_atomic.h>
#include <vlc_viewpoint.h>
#include "../clock/clock.h"
#define AOUT_MAX_INPUT_RATE (4)
enum {
AOUT_RESAMPLING_NONE=0,
AOUT_RESAMPLING_UP,
AOUT_RESAMPLING_DOWN
};
typedef struct aout_volume aout_volume_t;
typedef struct aout_dev aout_dev_t;
typedef struct
{
vlc_mutex_t lock;
module_t *module; 
aout_filters_t *filters;
aout_volume_t *volume;
bool bitexact;
struct
{
vlc_mutex_t lock;
aout_dev_t *list;
unsigned count;
} dev;
struct
{
atomic_bool update;
vlc_mutex_t lock;
vlc_viewpoint_t value;
} vp;
struct
{
struct vlc_clock_t *clock;
float rate; 
vlc_tick_t resamp_start_drift; 
int resamp_type; 
bool discontinuity;
vlc_tick_t request_delay;
vlc_tick_t delay;
vlc_tick_t first_pts;
} sync;
vlc_tick_t original_pts;
int requested_stereo_mode; 
int input_profile;
audio_sample_format_t input_format;
audio_sample_format_t filter_format;
audio_sample_format_t mixer_format;
aout_filters_cfg_t filters_cfg;
atomic_uint buffers_lost;
atomic_uint buffers_played;
atomic_uchar restart;
vlc_atomic_rc_t rc;
} aout_owner_t;
typedef struct
{
audio_output_t output;
aout_owner_t owner;
} aout_instance_t;
static inline aout_owner_t *aout_owner (audio_output_t *aout)
{
return &((aout_instance_t *)aout)->owner;
}
aout_volume_t *aout_volume_New(vlc_object_t *, const audio_replay_gain_t *);
#define aout_volume_New(o, g) aout_volume_New(VLC_OBJECT(o), g)
int aout_volume_SetFormat(aout_volume_t *, vlc_fourcc_t);
void aout_volume_SetVolume(aout_volume_t *, float);
int aout_volume_Amplify(aout_volume_t *, block_t *);
void aout_volume_Delete(aout_volume_t *);
audio_output_t *aout_New (vlc_object_t *);
#define aout_New(a) aout_New(VLC_OBJECT(a))
void aout_Destroy (audio_output_t *);
int aout_OutputNew(audio_output_t *);
void aout_OutputDelete( audio_output_t * p_aout );
void aout_FormatsPrint(vlc_object_t *, const char *,
const audio_sample_format_t *,
const audio_sample_format_t *);
#define aout_FormatsPrint(o, t, a, b) aout_FormatsPrint(VLC_OBJECT(o), t, a, b)
#define AOUT_DEC_SUCCESS 0
#define AOUT_DEC_CHANGED 1
#define AOUT_DEC_FAILED VLC_EGENERIC
int aout_DecNew(audio_output_t *, const audio_sample_format_t *, int profile,
struct vlc_clock_t *clock, const audio_replay_gain_t *);
void aout_DecDelete(audio_output_t *);
int aout_DecPlay(audio_output_t *aout, block_t *block);
void aout_DecGetResetStats(audio_output_t *, unsigned *, unsigned *);
void aout_DecChangePause(audio_output_t *, bool b_paused, vlc_tick_t i_date);
void aout_DecChangeRate(audio_output_t *aout, float rate);
void aout_DecChangeDelay(audio_output_t *aout, vlc_tick_t delay);
void aout_DecFlush(audio_output_t *);
void aout_DecDrain(audio_output_t *);
void aout_RequestRestart (audio_output_t *, unsigned);
void aout_RequestRetiming(audio_output_t *aout, vlc_tick_t system_ts,
vlc_tick_t audio_ts);
static inline void aout_InputRequestRestart(audio_output_t *aout)
{
aout_RequestRestart(aout, AOUT_RESTART_FILTERS);
}
static inline void aout_SetWavePhysicalChannels(audio_sample_format_t *fmt)
{
static const uint32_t wave_channels[] = {
AOUT_CHAN_LEFT, AOUT_CHAN_RIGHT, AOUT_CHAN_CENTER,
AOUT_CHAN_LFE, AOUT_CHAN_REARLEFT, AOUT_CHAN_REARRIGHT,
AOUT_CHAN_MIDDLELEFT, AOUT_CHAN_MIDDLERIGHT, AOUT_CHAN_REARCENTER };
fmt->i_physical_channels = 0;
for (int i = 0; i < fmt->i_channels && i < AOUT_CHAN_MAX; ++i)
fmt->i_physical_channels |= wave_channels[i];
aout_FormatPrepare(fmt);
}
aout_filters_t *aout_FiltersNewWithClock(vlc_object_t *, const vlc_clock_t *,
const audio_sample_format_t *,
const audio_sample_format_t *,
const aout_filters_cfg_t *cfg) VLC_USED;
void aout_FiltersResetClock(aout_filters_t *filters);
void aout_FiltersSetClockDelay(aout_filters_t *filters, vlc_tick_t delay);
bool aout_FiltersCanResample (aout_filters_t *filters);
