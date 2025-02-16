




















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdatomic.h>
#include <stddef.h>
#include <math.h>

#include <vlc_common.h>
#include <libvlc.h>
#include <vlc_modules.h>
#include <vlc_aout.h>
#include <vlc_aout_volume.h>
#include "aout_internal.h"

struct aout_volume
{
audio_volume_t object;
audio_replay_gain_t replay_gain;
_Atomic float gain_factor;
float output_factor;
module_t *module;
};

static int ReplayGainCallback (vlc_object_t *, char const *,
vlc_value_t, vlc_value_t, void *);

#undef aout_volume_New



aout_volume_t *aout_volume_New(vlc_object_t *parent,
const audio_replay_gain_t *gain)
{
aout_volume_t *vol = vlc_custom_create(parent, sizeof (aout_volume_t),
"volume");
if (unlikely(vol == NULL))
return NULL;
vol->module = NULL;
vol->output_factor = 1.f;




if (gain != NULL)
memcpy(&vol->replay_gain, gain, sizeof (vol->replay_gain));
else
memset(&vol->replay_gain, 0, sizeof (vol->replay_gain));

var_AddCallback(parent, "audio-replay-gain-mode",
ReplayGainCallback, vol);
var_TriggerCallback(parent, "audio-replay-gain-mode");

return vol;
}




int aout_volume_SetFormat(aout_volume_t *vol, vlc_fourcc_t format)
{
if (unlikely(vol == NULL))
return -1;

audio_volume_t *obj = &vol->object;
if (vol->module != NULL)
{
if (obj->format == format)
{
msg_Dbg (obj, "retaining sample format");
return 0;
}
msg_Dbg (obj, "changing sample format");
module_unneed(obj, vol->module);
}

obj->format = format;
vol->module = module_need(obj, "audio volume", NULL, false);
if (vol->module == NULL)
return -1;
return 0;
}




void aout_volume_Delete(aout_volume_t *vol)
{
if (vol == NULL)
return;

audio_volume_t *obj = &vol->object;

if (vol->module != NULL)
module_unneed(obj, vol->module);
var_DelCallback(vlc_object_parent(obj), "audio-replay-gain-mode",
ReplayGainCallback, vol);
vlc_object_delete(obj);
}

void aout_volume_SetVolume(aout_volume_t *vol, float factor)
{
if (unlikely(vol == NULL))
return;

vol->output_factor = factor;
}




int aout_volume_Amplify(aout_volume_t *vol, block_t *block)
{
if (unlikely(vol == NULL) || vol->module == NULL)
return -1;

float amp = vol->output_factor * atomic_load(&vol->gain_factor);

vol->object.amplify(&vol->object, block, amp);
return 0;
}


static float aout_ReplayGainSelect(vlc_object_t *obj, const char *str,
const audio_replay_gain_t *replay_gain)
{
unsigned mode = AUDIO_REPLAY_GAIN_MAX;

if (likely(str != NULL))
{ 
if (!strcmp (str, "track"))
mode = AUDIO_REPLAY_GAIN_TRACK;
else
if (!strcmp (str, "album"))
mode = AUDIO_REPLAY_GAIN_ALBUM;
}


float multiplier;

if (mode == AUDIO_REPLAY_GAIN_MAX)
{
multiplier = 1.f;
}
else
{
float gain;


if (!replay_gain->pb_gain[mode] && replay_gain->pb_gain[!mode])
mode = !mode;

if (replay_gain->pb_gain[mode])
gain = replay_gain->pf_gain[mode]
+ var_InheritFloat (obj, "audio-replay-gain-preamp");
else
gain = var_InheritFloat (obj, "audio-replay-gain-default");

multiplier = powf (10.f, gain / 20.f);

if (var_InheritBool (obj, "audio-replay-gain-peak-protection"))
multiplier = fminf (multiplier, replay_gain->pb_peak[mode]
? 1.f / replay_gain->pf_peak[mode]
: 1.f);
}


multiplier *= var_InheritFloat (obj, "gain");

return multiplier;
}

static int ReplayGainCallback (vlc_object_t *obj, char const *var,
vlc_value_t oldval, vlc_value_t val, void *data)
{
aout_volume_t *vol = data;
float multiplier = aout_ReplayGainSelect(obj, val.psz_string,
&vol->replay_gain);
atomic_store(&vol->gain_factor, multiplier);
VLC_UNUSED(var); VLC_UNUSED(oldval);
return VLC_SUCCESS;
}
