#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdlib.h>
#include <assert.h>
#include <vlc_common.h>
#include <vlc_aout.h>
#include <vlc_modules.h>
#include <vlc_atomic.h>
#include "libvlc.h"
#include "aout_internal.h"
struct aout_dev
{
aout_dev_t *next;
char *name;
char id[1];
};
static int var_Copy (vlc_object_t *src, const char *name, vlc_value_t prev,
vlc_value_t value, void *data)
{
vlc_object_t *dst = data;
(void) src; (void) prev;
return var_Set (dst, name, value);
}
static int var_CopyDevice (vlc_object_t *src, const char *name,
vlc_value_t prev, vlc_value_t value, void *data)
{
vlc_object_t *dst = data;
(void) src; (void) name; (void) prev;
return var_Set (dst, "audio-device", value);
}
static void aout_TimingNotify(audio_output_t *aout, vlc_tick_t system_ts,
vlc_tick_t audio_ts)
{
aout_RequestRetiming(aout, system_ts, audio_ts);
}
static void aout_VolumeNotify (audio_output_t *aout, float volume)
{
var_SetFloat (aout, "volume", volume);
}
static void aout_MuteNotify (audio_output_t *aout, bool mute)
{
var_SetBool (aout, "mute", mute);
}
static void aout_PolicyNotify (audio_output_t *aout, bool cork)
{
(cork ? var_IncInteger : var_DecInteger)(vlc_object_parent(aout), "corks");
}
static void aout_DeviceNotify (audio_output_t *aout, const char *id)
{
var_SetString (aout, "device", (id != NULL) ? id : "");
}
static void aout_HotplugNotify (audio_output_t *aout,
const char *id, const char *name)
{
aout_owner_t *owner = aout_owner (aout);
aout_dev_t *dev, **pp = &owner->dev.list;
vlc_mutex_lock (&owner->dev.lock);
while ((dev = *pp) != NULL)
{
if (!strcmp (id, dev->id))
break;
pp = &dev->next;
}
if (name != NULL)
{
if (dev == NULL) 
{
dev = malloc (sizeof (*dev) + strlen (id));
if (unlikely(dev == NULL))
goto out;
dev->next = NULL;
strcpy (dev->id, id);
*pp = dev;
owner->dev.count++;
}
else 
free (dev->name);
dev->name = strdup (name);
}
else
{
if (dev != NULL) 
{
owner->dev.count--;
*pp = dev->next;
free (dev->name);
free (dev);
}
}
out:
vlc_mutex_unlock (&owner->dev.lock);
}
static void aout_RestartNotify (audio_output_t *aout, unsigned mode)
{
aout_RequestRestart (aout, mode);
}
static int aout_GainNotify (audio_output_t *aout, float gain)
{
aout_owner_t *owner = aout_owner (aout);
vlc_mutex_assert(&owner->lock);
aout_volume_SetVolume (owner->volume, gain);
return 0;
}
static const struct vlc_audio_output_events aout_events = {
aout_TimingNotify,
aout_VolumeNotify,
aout_MuteNotify,
aout_PolicyNotify,
aout_DeviceNotify,
aout_HotplugNotify,
aout_RestartNotify,
aout_GainNotify,
};
static int FilterCallback (vlc_object_t *obj, const char *var,
vlc_value_t prev, vlc_value_t cur, void *data)
{
if (strcmp(prev.psz_string, cur.psz_string))
aout_InputRequestRestart ((audio_output_t *)obj);
(void) var; (void) data;
return VLC_SUCCESS;
}
static int StereoModeCallback (vlc_object_t *obj, const char *varname,
vlc_value_t oldval, vlc_value_t newval, void *data)
{
audio_output_t *aout = (audio_output_t *)obj;
(void)varname; (void)oldval; (void)newval; (void)data;
aout_owner_t *owner = aout_owner (aout);
vlc_mutex_lock (&owner->lock);
owner->requested_stereo_mode = newval.i_int;
vlc_mutex_unlock (&owner->lock);
aout_RestartRequest (aout, AOUT_RESTART_STEREOMODE);
return 0;
}
static void aout_ChangeViewpoint(audio_output_t *, const vlc_viewpoint_t *);
static int ViewpointCallback (vlc_object_t *obj, const char *var,
vlc_value_t prev, vlc_value_t cur, void *data)
{
if( cur.p_address != NULL )
aout_ChangeViewpoint((audio_output_t *)obj, cur.p_address );
(void) var; (void) data; (void) prev;
return VLC_SUCCESS;
}
#undef aout_New
audio_output_t *aout_New (vlc_object_t *parent)
{
vlc_value_t val;
audio_output_t *aout = vlc_custom_create (parent, sizeof (aout_instance_t),
"audio output");
if (unlikely(aout == NULL))
return NULL;
aout_owner_t *owner = aout_owner (aout);
vlc_mutex_init (&owner->lock);
vlc_mutex_init (&owner->dev.lock);
vlc_mutex_init (&owner->vp.lock);
vlc_viewpoint_init (&owner->vp.value);
atomic_init (&owner->vp.update, false);
vlc_atomic_rc_init(&owner->rc);
var_Create (aout, "volume", VLC_VAR_FLOAT);
var_AddCallback (aout, "volume", var_Copy, parent);
var_Create (aout, "mute", VLC_VAR_BOOL | VLC_VAR_DOINHERIT);
var_AddCallback (aout, "mute", var_Copy, parent);
var_Create (aout, "device", VLC_VAR_STRING);
var_AddCallback (aout, "device", var_CopyDevice, parent);
var_Create (aout, "dtshd", VLC_VAR_BOOL);
aout->events = &aout_events;
aout->start = NULL;
aout->stop = NULL;
aout->volume_set = NULL;
aout->mute_set = NULL;
aout->device_select = NULL;
owner->module = module_need_var(aout, "audio output", "aout");
if (owner->module == NULL)
{
msg_Err (aout, "no suitable audio output module");
vlc_object_delete(aout);
return NULL;
}
assert(aout->start && aout->stop);
module_config_t *cfg;
char *str;
var_Create (aout, "visual", VLC_VAR_STRING);
var_Change(aout, "visual", VLC_VAR_SETTEXT, _("Visualizations"));
val.psz_string = (char *)"";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, _("Disable"));
val.psz_string = (char *)"spectrometer";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, _("Spectrometer"));
val.psz_string = (char *)"scope";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, _("Scope"));
val.psz_string = (char *)"spectrum";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, _("Spectrum"));
val.psz_string = (char *)"vuMeter";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, _("VU meter"));
if (module_exists ("goom"))
{
val.psz_string = (char *)"goom";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, "Goom");
}
if (module_exists ("projectm"))
{
val.psz_string = (char *)"projectm";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, "projectM");
}
if (module_exists ("vsxu"))
{
val.psz_string = (char *)"vsxu";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, "Vovoid VSXU");
}
if (module_exists ("glspectrum"))
{
val.psz_string = (char *)"glspectrum";
var_Change(aout, "visual", VLC_VAR_ADDCHOICE, val, "3D spectrum");
}
str = var_GetNonEmptyString (aout, "effect-list");
if (str != NULL)
{
var_SetString (aout, "visual", str);
free (str);
}
var_Create (aout, "audio-filter", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
var_AddCallback (aout, "audio-filter", FilterCallback, NULL);
var_Change(aout, "audio-filter", VLC_VAR_SETTEXT, _("Audio filters"));
var_Create (aout, "viewpoint", VLC_VAR_ADDRESS );
var_AddCallback (aout, "viewpoint", ViewpointCallback, NULL);
var_Create (aout, "audio-visual", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
var_Change(aout, "audio-visual", VLC_VAR_SETTEXT,
_("Audio visualizations"));
var_Create (aout, "audio-replay-gain-mode",
VLC_VAR_STRING | VLC_VAR_DOINHERIT );
var_Change(aout, "audio-replay-gain-mode", VLC_VAR_SETTEXT,
_("Replay gain"));
cfg = config_FindConfig("audio-replay-gain-mode");
if (likely(cfg != NULL))
for (unsigned i = 0; i < cfg->list_count; i++)
{
val.psz_string = (char *)cfg->list.psz[i];
var_Change(aout, "audio-replay-gain-mode", VLC_VAR_ADDCHOICE,
val, vlc_gettext(cfg->list_text[i]));
}
var_Create (aout, "stereo-mode", VLC_VAR_INTEGER | VLC_VAR_DOINHERIT);
owner->requested_stereo_mode = var_GetInteger (aout, "stereo-mode");
var_AddCallback (aout, "stereo-mode", StereoModeCallback, NULL);
var_Change(aout, "stereo-mode", VLC_VAR_SETTEXT, _("Stereo audio mode"));
var_Create (aout, "equalizer-preamp", VLC_VAR_FLOAT | VLC_VAR_DOINHERIT);
var_Create (aout, "equalizer-bands", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
var_Create (aout, "equalizer-preset", VLC_VAR_STRING | VLC_VAR_DOINHERIT);
owner->bitexact = var_InheritBool (aout, "audio-bitexact");
return aout;
}
audio_output_t *aout_Hold(audio_output_t *aout)
{
aout_owner_t *owner = aout_owner(aout);
vlc_atomic_rc_inc(&owner->rc);
return aout;
}
void aout_Destroy (audio_output_t *aout)
{
aout_owner_t *owner = aout_owner (aout);
vlc_mutex_lock(&owner->lock);
module_unneed (aout, owner->module);
aout->volume_set = NULL;
aout->mute_set = NULL;
aout->device_select = NULL;
vlc_mutex_unlock(&owner->lock);
var_DelCallback (aout, "viewpoint", ViewpointCallback, NULL);
var_DelCallback (aout, "audio-filter", FilterCallback, NULL);
var_DelCallback(aout, "device", var_CopyDevice, vlc_object_parent(aout));
var_DelCallback(aout, "mute", var_Copy, vlc_object_parent(aout));
var_SetFloat (aout, "volume", -1.f);
var_DelCallback(aout, "volume", var_Copy, vlc_object_parent(aout));
var_DelCallback (aout, "stereo-mode", StereoModeCallback, NULL);
aout_Release(aout);
}
void aout_Release(audio_output_t *aout)
{
aout_owner_t *owner = aout_owner(aout);
if (!vlc_atomic_rc_dec(&owner->rc))
return;
atomic_thread_fence(memory_order_acquire);
for (aout_dev_t *dev = owner->dev.list, *next; dev != NULL; dev = next)
{
next = dev->next;
free (dev->name);
free (dev);
}
vlc_object_delete(VLC_OBJECT(aout));
}
static void aout_PrepareStereoMode (audio_output_t *aout,
audio_sample_format_t *restrict fmt,
aout_filters_cfg_t *filters_cfg,
audio_channel_type_t input_chan_type,
unsigned i_nb_input_channels)
{
aout_owner_t *owner = aout_owner (aout);
var_Change(aout, "stereo-mode", VLC_VAR_CLEARCHOICES);
vlc_value_t val;
const char *txt;
val.i_int = 0;
if (!AOUT_FMT_LINEAR(fmt) || i_nb_input_channels == 1)
return;
int i_output_mode = owner->requested_stereo_mode;
int i_default_mode = AOUT_VAR_CHAN_UNSET;
val.i_int = AOUT_VAR_CHAN_MONO;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val, _("Mono"));
if (i_nb_input_channels != 2)
{
val.i_int = AOUT_VAR_CHAN_UNSET;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val, _("Original"));
}
if (fmt->i_chan_mode & AOUT_CHANMODE_DOLBYSTEREO)
{
val.i_int = AOUT_VAR_CHAN_DOLBYS;
txt = _("Dolby Surround");
}
else
{
val.i_int = AOUT_VAR_CHAN_STEREO;
txt = _("Stereo");
}
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val, txt);
if (i_nb_input_channels == 2)
{
if (fmt->i_chan_mode & AOUT_CHANMODE_DUALMONO)
i_default_mode = AOUT_VAR_CHAN_LEFT;
else
i_default_mode = val.i_int; 
val.i_int = AOUT_VAR_CHAN_LEFT;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val, _("Left"));
val.i_int = AOUT_VAR_CHAN_RIGHT;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val, _("Right"));
val.i_int = AOUT_VAR_CHAN_RSTEREO;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val,
_("Reverse stereo"));
}
if (input_chan_type == AUDIO_CHANNEL_TYPE_AMBISONICS
|| i_nb_input_channels > 2)
{
val.i_int = AOUT_VAR_CHAN_HEADPHONES;
var_Change(aout, "stereo-mode", VLC_VAR_ADDCHOICE, val,
_("Headphones"));
if (aout->current_sink_info.headphones)
i_default_mode = AOUT_VAR_CHAN_HEADPHONES;
}
bool mode_available = false;
vlc_value_t *vals;
size_t count;
if (!var_Change(aout, "stereo-mode", VLC_VAR_GETCHOICES,
&count, &vals, (char ***)NULL))
{
for (size_t i = 0; !mode_available && i < count; ++i)
{
if (vals[i].i_int == i_output_mode)
mode_available = true;
}
free(vals);
}
if (!mode_available)
i_output_mode = i_default_mode;
switch (i_output_mode)
{
case AOUT_VAR_CHAN_RSTEREO:
filters_cfg->remap[AOUT_CHANIDX_LEFT] = AOUT_CHANIDX_RIGHT;
filters_cfg->remap[AOUT_CHANIDX_RIGHT] = AOUT_CHANIDX_LEFT;
break;
case AOUT_VAR_CHAN_STEREO:
break;
case AOUT_VAR_CHAN_LEFT:
filters_cfg->remap[AOUT_CHANIDX_RIGHT] = AOUT_CHANIDX_DISABLE;
break;
case AOUT_VAR_CHAN_RIGHT:
filters_cfg->remap[AOUT_CHANIDX_LEFT] = AOUT_CHANIDX_DISABLE;
break;
case AOUT_VAR_CHAN_DOLBYS:
fmt->i_chan_mode = AOUT_CHANMODE_DOLBYSTEREO;
break;
case AOUT_VAR_CHAN_HEADPHONES:
filters_cfg->headphones = true;
break;
case AOUT_VAR_CHAN_MONO:
for (size_t i = 0; i < AOUT_CHANIDX_MAX; ++ i)
filters_cfg->remap[i] = AOUT_CHANIDX_LEFT;
break;
default:
break;
}
var_Change(aout, "stereo-mode", VLC_VAR_SETVALUE,
(vlc_value_t) { .i_int = i_output_mode });
}
int aout_OutputNew (audio_output_t *aout)
{
aout_owner_t *owner = aout_owner (aout);
audio_sample_format_t *fmt = &owner->mixer_format;
audio_sample_format_t *filter_fmt = &owner->filter_format;
aout_filters_cfg_t *filters_cfg = &owner->filters_cfg;
audio_channel_type_t input_chan_type = fmt->channel_type;
unsigned i_nb_input_channels = fmt->i_channels;
vlc_fourcc_t formats[] = {
fmt->i_format, 0, 0
};
if (AOUT_FMT_LINEAR(fmt))
{
if (fmt->channel_type == AUDIO_CHANNEL_TYPE_BITMAP
&& aout_FormatNbChannels(fmt) == 0)
{
assert(fmt->i_channels > 0);
aout_SetWavePhysicalChannels(fmt);
}
if (fmt->channel_type == AUDIO_CHANNEL_TYPE_AMBISONICS)
{
fmt->i_physical_channels = AOUT_CHANS_7_1;
}
fmt->i_format = (fmt->i_bitspersample > 16) ? VLC_CODEC_FL32
: VLC_CODEC_S16N;
if (fmt->i_physical_channels == AOUT_CHANS_STEREO
&& (owner->requested_stereo_mode == AOUT_VAR_CHAN_LEFT
|| owner->requested_stereo_mode == AOUT_VAR_CHAN_RIGHT))
fmt->i_physical_channels = AOUT_CHAN_CENTER;
aout_FormatPrepare (fmt);
assert (aout_FormatNbChannels(fmt) > 0);
}
else
{
switch (fmt->i_format)
{
case VLC_CODEC_DTS:
if (owner->input_profile > 0)
{
assert(ARRAY_SIZE(formats) >= 3);
formats[0] = VLC_CODEC_DTSHD;
formats[1] = VLC_CODEC_DTS;
}
break;
case VLC_CODEC_A52:
if (owner->input_profile > 0)
{
assert(ARRAY_SIZE(formats) >= 3);
formats[0] = VLC_CODEC_EAC3;
formats[1] = VLC_CODEC_A52;
}
break;
default:
break;
}
}
aout->current_sink_info.headphones = false;
vlc_mutex_lock(&owner->lock);
int ret = VLC_EGENERIC;
for (size_t i = 0; formats[i] != 0 && ret != VLC_SUCCESS; ++i)
{
filter_fmt->i_format = fmt->i_format = formats[i];
ret = aout->start(aout, fmt);
}
vlc_mutex_unlock(&owner->lock);
if (ret)
{
msg_Err (aout, "module not functional");
return -1;
}
assert(aout->flush && aout->play && aout->time_get && aout->pause);
aout_PrepareStereoMode (aout, fmt, filters_cfg, input_chan_type,
i_nb_input_channels);
aout_FormatPrepare (fmt);
assert (fmt->i_bytes_per_frame > 0 && fmt->i_frame_length > 0);
aout_FormatPrint (aout, "output", fmt);
return 0;
}
void aout_OutputDelete (audio_output_t *aout)
{
aout_owner_t *owner = aout_owner(aout);
vlc_mutex_lock(&owner->lock);
aout->stop (aout);
vlc_mutex_unlock(&owner->lock);
}
float aout_VolumeGet (audio_output_t *aout)
{
return var_GetFloat (aout, "volume");
}
int aout_VolumeSet (audio_output_t *aout, float vol)
{
aout_owner_t *owner = aout_owner(aout);
int ret;
vlc_mutex_lock(&owner->lock);
ret = aout->volume_set ? aout->volume_set(aout, vol) : -1;
vlc_mutex_unlock(&owner->lock);
return ret ? -1 : 0;
}
int aout_VolumeUpdate (audio_output_t *aout, int value, float *volp)
{
int ret = -1;
float stepSize = var_InheritFloat (aout, "volume-step") / (float)AOUT_VOLUME_DEFAULT;
float delta = value * stepSize;
float vol = aout_VolumeGet (aout);
if (vol >= 0.f)
{
vol += delta;
if (vol < 0.f)
vol = 0.f;
if (vol > 2.f)
vol = 2.f;
vol = (roundf (vol / stepSize)) * stepSize;
if (volp != NULL)
*volp = vol;
ret = aout_VolumeSet (aout, vol);
}
return ret;
}
int aout_MuteGet (audio_output_t *aout)
{
return var_InheritBool (aout, "mute");
}
int aout_MuteSet (audio_output_t *aout, bool mute)
{
aout_owner_t *owner = aout_owner(aout);
int ret;
vlc_mutex_lock(&owner->lock);
ret = aout->mute_set ? aout->mute_set(aout, mute) : -1;
vlc_mutex_unlock(&owner->lock);
return ret ? -1 : 0;
}
char *aout_DeviceGet (audio_output_t *aout)
{
return var_GetNonEmptyString (aout, "device");
}
int aout_DeviceSet (audio_output_t *aout, const char *id)
{
aout_owner_t *owner = aout_owner(aout);
int ret;
vlc_mutex_lock(&owner->lock);
ret = aout->device_select ? aout->device_select(aout, id) : -1;
vlc_mutex_unlock(&owner->lock);
return ret ? -1 : 0;
}
int aout_DevicesList (audio_output_t *aout, char ***ids, char ***names)
{
aout_owner_t *owner = aout_owner (aout);
char **tabid, **tabname;
unsigned i = 0;
vlc_mutex_lock (&owner->dev.lock);
tabid = vlc_alloc (owner->dev.count, sizeof (*tabid));
tabname = vlc_alloc (owner->dev.count, sizeof (*tabname));
if (unlikely(tabid == NULL || tabname == NULL))
goto error;
*ids = tabid;
*names = tabname;
for (aout_dev_t *dev = owner->dev.list; dev != NULL; dev = dev->next)
{
tabid[i] = strdup(dev->id);
if (unlikely(tabid[i] == NULL))
goto error;
tabname[i] = strdup(dev->name);
if (unlikely(tabname[i] == NULL))
{
free(tabid[i]);
goto error;
}
i++;
}
vlc_mutex_unlock (&owner->dev.lock);
return i;
error:
vlc_mutex_unlock(&owner->dev.lock);
while (i > 0)
{
i--;
free(tabname[i]);
free(tabid[i]);
}
free(tabname);
free(tabid);
return -1;
}
static void aout_ChangeViewpoint(audio_output_t *aout,
const vlc_viewpoint_t *p_viewpoint)
{
aout_owner_t *owner = aout_owner(aout);
vlc_mutex_lock(&owner->vp.lock);
owner->vp.value = *p_viewpoint;
atomic_store_explicit(&owner->vp.update, true, memory_order_relaxed);
vlc_mutex_unlock(&owner->vp.lock);
}
