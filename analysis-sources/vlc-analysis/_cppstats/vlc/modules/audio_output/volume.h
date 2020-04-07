#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdio.h>
#include <math.h>
#include <vlc_common.h>
#include <vlc_aout.h>
#define add_sw_gain() add_float(MODULE_STRING"-gain", 1., N_("Software gain"), N_("This linear gain will be applied in software."), true) change_float_range(0., 8.)
static int aout_SoftVolumeSet(audio_output_t *aout, float volume)
{
aout_sys_t *sys = aout->sys;
float gain = volume * volume * volume;
if (!sys->soft_mute && aout_GainRequest(aout, gain))
return -1;
sys->soft_gain = gain;
if (var_InheritBool(aout, "volume-save"))
config_PutFloat(MODULE_STRING"-gain", gain);
aout_VolumeReport(aout, volume);
return 0;
}
static int aout_SoftMuteSet (audio_output_t *aout, bool mute)
{
aout_sys_t *sys = aout->sys;
if (aout_GainRequest(aout, mute ? 0.f : sys->soft_gain))
return -1;
sys->soft_mute = mute;
aout_MuteReport(aout, mute);
return 0;
}
static void aout_SoftVolumeInit(audio_output_t *aout)
{
aout_sys_t *sys = aout->sys;
float gain = var_InheritFloat(aout, MODULE_STRING"-gain");
bool mute = var_InheritBool(aout, "mute");
aout->volume_set = aout_SoftVolumeSet;
aout->mute_set = aout_SoftMuteSet;
sys->soft_gain = gain;
sys->soft_mute = mute;
aout_MuteReport(aout, mute);
aout_VolumeReport(aout, cbrtf(gain));
}
static void aout_SoftVolumeStart (audio_output_t *aout)
{
aout_sys_t *sys = aout->sys;
if (aout_GainRequest(aout, sys->soft_mute ? 0.f : sys->soft_gain))
{
aout_MuteReport(aout, false);
aout_VolumeReport(aout, 1.f);
}
}
