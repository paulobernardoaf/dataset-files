#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <assert.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_dialog.h>
#include <vlc_aout.h>
#include <vlc_cpu.h>
#include <alsa/asoundlib.h>
#include <alsa/version.h>
typedef struct
{
snd_pcm_t *pcm;
unsigned rate; 
vlc_fourcc_t format; 
uint8_t chans_table[AOUT_CHAN_MAX]; 
uint8_t chans_to_reorder; 
bool soft_mute;
float soft_gain;
char *device;
} aout_sys_t;
enum {
PASSTHROUGH_NONE,
PASSTHROUGH_SPDIF,
PASSTHROUGH_HDMI,
};
#include "audio_output/volume.h"
#define A52_FRAME_NB 1536
static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
static int EnumDevices(char const *, char ***, char ***);
#define AUDIO_DEV_TEXT N_("Audio output device")
#define AUDIO_DEV_LONGTEXT N_("Audio output device (using ALSA syntax).")
#define AUDIO_CHAN_TEXT N_("Audio output channels")
#define AUDIO_CHAN_LONGTEXT N_("Channels available for audio output. " "If the input has more channels than the output, it will be down-mixed. " "This parameter is ignored when digital pass-through is active.")
static const int channels[] = {
AOUT_CHAN_CENTER, AOUT_CHANS_STEREO, AOUT_CHANS_4_0, AOUT_CHANS_4_1,
AOUT_CHANS_5_0, AOUT_CHANS_5_1, AOUT_CHANS_7_1,
};
static const char *const channels_text[] = {
N_("Mono"), N_("Stereo"), N_("Surround 4.0"), N_("Surround 4.1"),
N_("Surround 5.0"), N_("Surround 5.1"), N_("Surround 7.1"),
};
#define PASSTHROUGH_TEXT N_("Audio passthrough mode")
static const int passthrough_modes[] = {
PASSTHROUGH_NONE, PASSTHROUGH_SPDIF, PASSTHROUGH_HDMI,
};
static const char *const passthrough_modes_text[] = {
N_("None"), N_("S/PDIF"), N_("HDMI"),
};
vlc_module_begin ()
set_shortname( "ALSA" )
set_description( N_("ALSA audio output") )
set_category( CAT_AUDIO )
set_subcategory( SUBCAT_AUDIO_AOUT )
add_string ("alsa-audio-device", "default",
AUDIO_DEV_TEXT, AUDIO_DEV_LONGTEXT, false)
add_integer ("alsa-audio-channels", AOUT_CHANS_FRONT,
AUDIO_CHAN_TEXT, AUDIO_CHAN_LONGTEXT, false)
change_integer_list (channels, channels_text)
add_integer("alsa-passthrough", PASSTHROUGH_NONE, PASSTHROUGH_TEXT,
PASSTHROUGH_TEXT, false)
change_integer_list(passthrough_modes, passthrough_modes_text)
add_sw_gain ()
set_capability( "audio output", 150 )
set_callbacks( Open, Close )
vlc_module_end ()
static void Dump (vlc_object_t *obj, const char *msg,
int (*cb)(void *, snd_output_t *), void *p)
{
snd_output_t *output;
char *str;
if (unlikely(snd_output_buffer_open (&output)))
return;
int val = cb (p, output);
if (val)
{
msg_Warn (obj, "cannot get info: %s", snd_strerror (val));
return;
}
size_t len = snd_output_buffer_string (output, &str);
if (len > 0 && str[len - 1])
len--; 
msg_Dbg (obj, "%s%.*s", msg, (int)len, str);
snd_output_close (output);
}
#define Dump(o, m, cb, p) Dump(VLC_OBJECT(o), m, (int (*)(void *, snd_output_t *))(cb), p)
static void DumpDevice (vlc_object_t *obj, snd_pcm_t *pcm)
{
snd_pcm_info_t *info;
Dump (obj, " ", snd_pcm_dump, pcm);
snd_pcm_info_alloca (&info);
if (snd_pcm_info (pcm, info) == 0)
{
msg_Dbg (obj, " device name : %s", snd_pcm_info_get_name (info));
msg_Dbg (obj, " device ID : %s", snd_pcm_info_get_id (info));
msg_Dbg (obj, " subdevice name: %s",
snd_pcm_info_get_subdevice_name (info));
}
}
static void DumpDeviceStatus (vlc_object_t *obj, snd_pcm_t *pcm)
{
snd_pcm_status_t *status;
snd_pcm_status_alloca (&status);
snd_pcm_status (pcm, status);
Dump (obj, "current status:\n", snd_pcm_status_dump, status);
}
#define DumpDeviceStatus(o, p) DumpDeviceStatus(VLC_OBJECT(o), p)
#if (SND_LIB_VERSION >= 0x01001B)
static const uint16_t vlc_chans[] = {
[SND_CHMAP_MONO] = AOUT_CHAN_CENTER,
[SND_CHMAP_FL] = AOUT_CHAN_LEFT,
[SND_CHMAP_FR] = AOUT_CHAN_RIGHT,
[SND_CHMAP_RL] = AOUT_CHAN_REARLEFT,
[SND_CHMAP_RR] = AOUT_CHAN_REARRIGHT,
[SND_CHMAP_FC] = AOUT_CHAN_CENTER,
[SND_CHMAP_LFE] = AOUT_CHAN_LFE,
[SND_CHMAP_SL] = AOUT_CHAN_MIDDLELEFT,
[SND_CHMAP_SR] = AOUT_CHAN_MIDDLERIGHT,
[SND_CHMAP_RC] = AOUT_CHAN_REARCENTER,
};
static_assert(AOUT_CHAN_MAX == 9, "Missing channel entries");
static int Map2Mask (vlc_object_t *obj, const snd_pcm_chmap_t *restrict map)
{
uint16_t mask = 0;
for (unsigned i = 0; i < map->channels; i++)
{
const unsigned pos = map->pos[i];
uint_fast16_t vlc_chan = 0;
if (pos < sizeof (vlc_chans) / sizeof (vlc_chans[0]))
vlc_chan = vlc_chans[pos];
if (vlc_chan == 0)
{
msg_Dbg (obj, " %s channel %u position %u", "unsupported", i, pos);
return -1;
}
if (mask & vlc_chan)
{
msg_Dbg (obj, " %s channel %u position %u", "duplicate", i, pos);
return -1;
}
mask |= vlc_chan;
}
return mask;
}
static unsigned SetupChannelsFixed(const snd_pcm_chmap_t *restrict map,
uint16_t *restrict maskp, uint8_t *restrict tab)
{
uint32_t chans_out[AOUT_CHAN_MAX];
uint16_t mask = 0;
for (unsigned i = 0; i < map->channels; i++)
{
uint_fast16_t vlc_chan = vlc_chans[map->pos[i]];
chans_out[i] = vlc_chan;
mask |= vlc_chan;
}
*maskp = mask;
return aout_CheckChannelReorder(NULL, chans_out, mask, tab);
}
static unsigned SetupChannels (vlc_object_t *obj, snd_pcm_t *pcm,
uint16_t *restrict mask, uint8_t *restrict tab)
{
snd_pcm_chmap_query_t **maps = snd_pcm_query_chmaps (pcm);
if (maps == NULL)
{ 
msg_Dbg(obj, "channels map not provided");
return 0;
}
unsigned best_offset, best_score = 0, to_reorder = 0;
for (snd_pcm_chmap_query_t *const *p = maps; *p != NULL; p++)
{
snd_pcm_chmap_query_t *map = *p;
switch (map->type)
{
case SND_CHMAP_TYPE_FIXED:
case SND_CHMAP_TYPE_PAIRED:
case SND_CHMAP_TYPE_VAR:
break;
default:
msg_Err (obj, "unknown channels map type %u", map->type);
continue;
}
int chans = Map2Mask (obj, &map->map);
if (chans == -1)
continue;
unsigned score = (vlc_popcount (chans & *mask) << 8)
| (255 - vlc_popcount (chans));
if (score > best_score)
{
best_offset = p - maps;
best_score = score;
}
}
if (best_score == 0)
{
msg_Err (obj, "cannot find supported channels map");
goto out;
}
const snd_pcm_chmap_t *map = &maps[best_offset]->map;
msg_Dbg (obj, "using channels map %u, type %u, %u channel(s)", best_offset,
maps[best_offset]->type, map->channels);
to_reorder = SetupChannelsFixed(map, mask, tab);
out:
snd_pcm_free_chmaps (maps);
return to_reorder;
}
#else 
#define SetupChannels(obj, pcm, mask, tab) (0)
#endif
static int TimeGet (audio_output_t *aout, vlc_tick_t *);
static void Play(audio_output_t *, block_t *, vlc_tick_t);
static void Pause (audio_output_t *, bool, vlc_tick_t);
static void PauseDummy (audio_output_t *, bool, vlc_tick_t);
static void Flush (audio_output_t *);
static void Drain (audio_output_t *);
static int Start (audio_output_t *aout, audio_sample_format_t *restrict fmt)
{
aout_sys_t *sys = aout->sys;
snd_pcm_format_t pcm_format; 
unsigned channels;
int passthrough = PASSTHROUGH_NONE;
if (aout_FormatNbChannels(fmt) == 0)
return VLC_EGENERIC;
switch (fmt->i_format)
{
case VLC_CODEC_FL64:
pcm_format = SND_PCM_FORMAT_FLOAT64;
break;
case VLC_CODEC_FL32:
pcm_format = SND_PCM_FORMAT_FLOAT;
break;
case VLC_CODEC_S32N:
pcm_format = SND_PCM_FORMAT_S32;
break;
case VLC_CODEC_S16N:
pcm_format = SND_PCM_FORMAT_S16;
break;
case VLC_CODEC_U8:
pcm_format = SND_PCM_FORMAT_U8;
break;
default:
if (AOUT_FMT_SPDIF(fmt))
{
passthrough = var_InheritInteger(aout, "alsa-passthrough");
channels = 2;
}
if (AOUT_FMT_HDMI(fmt))
{
passthrough = var_InheritInteger(aout, "alsa-passthrough");
if (passthrough == PASSTHROUGH_SPDIF)
passthrough = PASSTHROUGH_NONE; 
channels = 8;
}
if (passthrough != PASSTHROUGH_NONE)
{
fmt->i_format = VLC_CODEC_SPDIFL;
pcm_format = SND_PCM_FORMAT_S16;
}
else
if (HAVE_FPU)
{
fmt->i_format = VLC_CODEC_FL32;
pcm_format = SND_PCM_FORMAT_FLOAT;
}
else
{
fmt->i_format = VLC_CODEC_S16N;
pcm_format = SND_PCM_FORMAT_S16;
}
}
const char *device = sys->device;
char sep = '\0';
if (passthrough != PASSTHROUGH_NONE)
{
const char *opt = NULL;
if (!strcmp (device, "default"))
device = (passthrough == PASSTHROUGH_HDMI) ? "hdmi" : "iec958";
if (!strncmp (device, "iec958", 6))
opt = device + 6;
if (!strncmp (device, "hdmi", 4))
opt = device + 4;
if (opt != NULL)
switch (*opt)
{
case ':': sep = ','; break;
case '\0': sep = ':'; break;
}
}
char *devbuf = NULL;
if (sep != '\0')
{
unsigned aes3;
switch (fmt->i_rate)
{
#define FS(freq) case freq: aes3 = IEC958_AES3_CON_FS_ ##freq; break;
FS( 44100) FS( 48000) FS( 32000)
FS( 22050) FS( 24000)
FS( 88200) FS(768000) FS( 96000)
FS(176400) FS(192000)
#undef FS
default:
aes3 = IEC958_AES3_CON_FS_NOTID;
break;
}
if (asprintf (&devbuf, "%s%cAES0=0x%x,AES1=0x%x,AES2=0x%x,AES3=0x%x",
device, sep,
IEC958_AES0_CON_EMPHASIS_NONE | IEC958_AES0_NONAUDIO,
IEC958_AES1_CON_ORIGINAL | IEC958_AES1_CON_PCM_CODER,
0, aes3) == -1)
return VLC_ENOMEM;
device = devbuf;
}
snd_pcm_t *pcm;
const int mode = SND_PCM_NO_AUTO_RESAMPLE;
int val = snd_pcm_open (&pcm, device, SND_PCM_STREAM_PLAYBACK, mode);
if (val != 0)
{
msg_Err (aout, "cannot open ALSA device \"%s\": %s", device,
snd_strerror (val));
vlc_dialog_display_error (aout, _("Audio output failed"),
_("The audio device \"%s\" could not be used:\n%s."),
sys->device, snd_strerror (val));
free (devbuf);
return VLC_EGENERIC;
}
sys->pcm = pcm;
msg_Dbg (aout, "using ALSA device: %s", device);
free (devbuf);
DumpDevice (VLC_OBJECT(aout), pcm);
snd_pcm_hw_params_t *hw;
unsigned param;
snd_pcm_hw_params_alloca (&hw);
snd_pcm_hw_params_any (pcm, hw);
Dump (aout, "initial hardware setup:\n", snd_pcm_hw_params_dump, hw);
val = snd_pcm_hw_params_set_rate_resample(pcm, hw, 0);
if (val)
{
msg_Err (aout, "cannot disable resampling: %s", snd_strerror (val));
goto error;
}
val = snd_pcm_hw_params_set_access (pcm, hw,
SND_PCM_ACCESS_RW_INTERLEAVED);
if (val)
{
msg_Err (aout, "cannot set access mode: %s", snd_strerror (val));
goto error;
}
if (snd_pcm_hw_params_test_format (pcm, hw, pcm_format) == 0)
;
else
if (snd_pcm_hw_params_test_format (pcm, hw, SND_PCM_FORMAT_FLOAT) == 0)
{
fmt->i_format = VLC_CODEC_FL32;
pcm_format = SND_PCM_FORMAT_FLOAT;
}
else
if (snd_pcm_hw_params_test_format (pcm, hw, SND_PCM_FORMAT_S32) == 0)
{
fmt->i_format = VLC_CODEC_S32N;
pcm_format = SND_PCM_FORMAT_S32;
}
else
if (snd_pcm_hw_params_test_format (pcm, hw, SND_PCM_FORMAT_S16) == 0)
{
fmt->i_format = VLC_CODEC_S16N;
pcm_format = SND_PCM_FORMAT_S16;
}
else
{
msg_Err (aout, "no supported sample format");
goto error;
}
val = snd_pcm_hw_params_set_format (pcm, hw, pcm_format);
if (val)
{
msg_Err (aout, "cannot set sample format: %s", snd_strerror (val));
goto error;
}
if (passthrough == PASSTHROUGH_NONE)
{
uint16_t map = var_InheritInteger (aout, "alsa-audio-channels");
sys->chans_to_reorder = SetupChannels (VLC_OBJECT(aout), pcm, &map,
sys->chans_table);
fmt->i_physical_channels = map;
channels = vlc_popcount (map);
}
else
sys->chans_to_reorder = 0;
val = snd_pcm_hw_params_set_channels (pcm, hw, channels);
if (val)
{
msg_Err (aout, "cannot set %u channels: %s", channels,
snd_strerror (val));
goto error;
}
val = snd_pcm_hw_params_set_rate_near (pcm, hw, &fmt->i_rate, NULL);
if (val)
{
msg_Err (aout, "cannot set sample rate: %s", snd_strerror (val));
goto error;
}
sys->rate = fmt->i_rate;
#if 1 
param = AOUT_MIN_PREPARE_TIME;
val = snd_pcm_hw_params_set_period_time_near (pcm, hw, &param, NULL);
if (val)
{
msg_Err (aout, "cannot set period: %s", snd_strerror (val));
goto error;
}
#endif
param = AOUT_MAX_ADVANCE_TIME;
val = snd_pcm_hw_params_set_buffer_time_near (pcm, hw, &param, NULL);
if (val)
{
msg_Err (aout, "cannot set buffer duration: %s", snd_strerror (val));
goto error;
}
#if 0
val = snd_pcm_hw_params_get_buffer_time (hw, &param, NULL);
if (val)
{
msg_Warn (aout, "cannot get buffer time: %s", snd_strerror(val));
param = AOUT_MIN_PREPARE_TIME;
}
else
param /= 2;
val = snd_pcm_hw_params_set_period_time_near (pcm, hw, &param, NULL);
if (val)
{
msg_Err (aout, "cannot set period: %s", snd_strerror (val));
goto error;
}
#endif
val = snd_pcm_hw_params (pcm, hw);
if (val < 0)
{
msg_Err (aout, "cannot commit hardware parameters: %s",
snd_strerror (val));
goto error;
}
Dump (aout, "final HW setup:\n", snd_pcm_hw_params_dump, hw);
snd_pcm_sw_params_t *sw;
snd_pcm_sw_params_alloca (&sw);
snd_pcm_sw_params_current (pcm, sw);
Dump (aout, "initial software parameters:\n", snd_pcm_sw_params_dump, sw);
val = snd_pcm_sw_params_set_start_threshold (pcm, sw, 1);
if( val < 0 )
{
msg_Err( aout, "unable to set start threshold (%s)",
snd_strerror( val ) );
goto error;
}
val = snd_pcm_sw_params (pcm, sw);
if (val)
{
msg_Err (aout, "cannot commit software parameters: %s",
snd_strerror (val));
goto error;
}
Dump (aout, "final software parameters:\n", snd_pcm_sw_params_dump, sw);
val = snd_pcm_prepare (pcm);
if (val)
{
msg_Err (aout, "cannot prepare device: %s", snd_strerror (val));
goto error;
}
if (passthrough != PASSTHROUGH_NONE)
{
fmt->i_bytes_per_frame = AOUT_SPDIF_SIZE * (channels / 2);
fmt->i_frame_length = A52_FRAME_NB;
}
fmt->channel_type = AUDIO_CHANNEL_TYPE_BITMAP;
sys->format = fmt->i_format;
if (snd_pcm_hw_params_can_pause (hw))
aout->pause = Pause;
else
{
aout->pause = PauseDummy;
msg_Warn (aout, "device cannot be paused");
}
aout_SoftVolumeStart (aout);
return 0;
error:
snd_pcm_close (pcm);
return VLC_EGENERIC;
}
static int TimeGet (audio_output_t *aout, vlc_tick_t *restrict delay)
{
aout_sys_t *sys = aout->sys;
snd_pcm_sframes_t frames;
int val = snd_pcm_delay (sys->pcm, &frames);
if (val)
{
msg_Err (aout, "cannot estimate delay: %s", snd_strerror (val));
return -1;
}
*delay = vlc_tick_from_samples(frames, sys->rate);
return 0;
}
static void Play(audio_output_t *aout, block_t *block, vlc_tick_t date)
{
aout_sys_t *sys = aout->sys;
if (sys->chans_to_reorder != 0)
aout_ChannelReorder(block->p_buffer, block->i_buffer,
sys->chans_to_reorder, sys->chans_table, sys->format);
snd_pcm_t *pcm = sys->pcm;
while (block->i_nb_samples > 0)
{
snd_pcm_sframes_t frames;
frames = snd_pcm_writei (pcm, block->p_buffer, block->i_nb_samples);
if (frames >= 0)
{
size_t bytes = snd_pcm_frames_to_bytes (pcm, frames);
block->i_nb_samples -= frames;
block->p_buffer += bytes;
block->i_buffer -= bytes;
}
else 
{
int val = snd_pcm_recover (pcm, frames, 1);
if (val)
{
msg_Err (aout, "cannot recover playback stream: %s",
snd_strerror (val));
DumpDeviceStatus (aout, pcm);
break;
}
msg_Warn (aout, "cannot write samples: %s", snd_strerror (frames));
}
}
block_Release (block);
(void) date;
}
static void Pause (audio_output_t *aout, bool pause, vlc_tick_t date)
{
aout_sys_t *p_sys = aout->sys;
snd_pcm_t *pcm = p_sys->pcm;
int val = snd_pcm_pause (pcm, pause);
if (unlikely(val))
PauseDummy (aout, pause, date);
}
static void PauseDummy (audio_output_t *aout, bool pause, vlc_tick_t date)
{
aout_sys_t *p_sys = aout->sys;
snd_pcm_t *pcm = p_sys->pcm;
if (pause)
snd_pcm_drop (pcm);
else
snd_pcm_prepare (pcm);
(void) date;
}
static void Flush (audio_output_t *aout)
{
aout_sys_t *p_sys = aout->sys;
snd_pcm_t *pcm = p_sys->pcm;
snd_pcm_drop (pcm);
snd_pcm_prepare (pcm);
}
static void Drain (audio_output_t *aout)
{
aout_sys_t *p_sys = aout->sys;
snd_pcm_t *pcm = p_sys->pcm;
snd_pcm_drain (pcm);
snd_pcm_prepare (pcm);
}
static void Stop (audio_output_t *aout)
{
aout_sys_t *sys = aout->sys;
snd_pcm_t *pcm = sys->pcm;
snd_pcm_drop (pcm);
snd_pcm_close (pcm);
}
static int EnumDevices(char const *varname,
char ***restrict idp, char ***restrict namep)
{
void **hints;
if (snd_device_name_hint(-1, "pcm", &hints) < 0)
return -1;
char **ids = NULL, **names = NULL;
unsigned n = 0;
bool hinted_default = false;
for (size_t i = 0; hints[i] != NULL; i++)
{
void *hint = hints[i];
char *name = snd_device_name_get_hint(hint, "NAME");
if (unlikely(name == NULL))
continue;
char *desc = snd_device_name_get_hint(hint, "DESC");
if (desc == NULL)
desc = xstrdup (name);
for (char *lf = strchr(desc, '\n'); lf; lf = strchr(lf, '\n'))
*lf = ' ';
ids = xrealloc (ids, (n + 1) * sizeof (*ids));
names = xrealloc (names, (n + 1) * sizeof (*names));
ids[n] = name;
names[n] = desc;
n++;
if (!strcmp(name, "default"))
hinted_default = true;
}
snd_device_name_free_hint(hints);
if (!hinted_default)
{
ids = xrealloc (ids, (n + 1) * sizeof (*ids));
names = xrealloc (names, (n + 1) * sizeof (*names));
ids[n] = xstrdup ("default");
names[n] = xstrdup (_("Default"));
n++;
}
*idp = ids;
*namep = names;
(void) varname;
return n;
}
VLC_CONFIG_STRING_ENUM(EnumDevices)
static int DeviceSelect (audio_output_t *aout, const char *id)
{
aout_sys_t *sys = aout->sys;
char *device = strdup (id ? id : "default");
if (unlikely(device == NULL))
return -1;
free (sys->device);
sys->device = device;
aout_DeviceReport (aout, device);
aout_RestartRequest (aout, AOUT_RESTART_OUTPUT);
return 0;
}
static int Open(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = malloc (sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
sys->device = var_InheritString (aout, "alsa-audio-device");
if (unlikely(sys->device == NULL))
goto error;
aout->sys = sys;
aout->start = Start;
aout->stop = Stop;
aout_SoftVolumeInit (aout);
aout->device_select = DeviceSelect;
aout_DeviceReport (aout, sys->device);
char **ids, **names;
int count = EnumDevices(NULL, &ids, &names);
if (count >= 0)
{
msg_Dbg (obj, "Available ALSA PCM devices:");
for (int i = 0; i < count; i++)
{
msg_Dbg(obj, "%s: %s", ids[i], names[i]);
aout_HotplugReport (aout, ids[i], names[i]);
free (names[i]);
free (ids[i]);
}
free (names);
free (ids);
}
aout->time_get = TimeGet;
aout->play = Play;
aout->flush = Flush;
aout->drain = Drain;
return VLC_SUCCESS;
error:
free (sys);
return VLC_ENOMEM;
}
static void Close(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = aout->sys;
free (sys->device);
free (sys);
}
