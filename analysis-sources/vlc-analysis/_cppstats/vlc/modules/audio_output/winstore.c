#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#define INITGUID
#define COBJMACROS
#include <stdlib.h>
#include <assert.h>
#include <audiopolicy.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_aout.h>
#include <vlc_modules.h>
#include "audio_output/mmdevice.h"
DEFINE_GUID (GUID_VLC_AUD_OUT, 0x4533f59d, 0x59ee, 0x00c6,
0xad, 0xb2, 0xc6, 0x8b, 0x50, 0x1a, 0x66, 0x55);
static void EnterMTA(void)
{
HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
if (unlikely(FAILED(hr)))
abort();
}
static void LeaveMTA(void)
{
CoUninitialize();
}
typedef struct
{
aout_stream_t *stream; 
module_t *module;
IAudioClient *client;
} aout_sys_t;
static int vlc_FromHR(audio_output_t *aout, HRESULT hr)
{
aout_sys_t* sys = aout->sys;
if (unlikely(hr == AUDCLNT_E_DEVICE_INVALIDATED ||
hr == AUDCLNT_E_RESOURCES_INVALIDATED))
{
sys->client = NULL;
}
return SUCCEEDED(hr) ? 0 : -1;
}
static int VolumeSet(audio_output_t *aout, float vol)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return VLC_EGENERIC;
HRESULT hr;
ISimpleAudioVolume *pc_AudioVolume = NULL;
float gain = 1.f;
vol = vol * vol * vol; 
if (vol > 1.f)
{
gain = vol;
vol = 1.f;
}
aout_GainRequest(aout, gain);
hr = IAudioClient_GetService(sys->client, &IID_ISimpleAudioVolume, &pc_AudioVolume);
if (FAILED(hr))
{
msg_Err(aout, "cannot get volume service (error 0x%lX)", hr);
goto done;
}
hr = ISimpleAudioVolume_SetMasterVolume(pc_AudioVolume, vol, NULL);
if (FAILED(hr))
{
msg_Err(aout, "cannot set volume (error 0x%lX)", hr);
goto done;
}
done:
ISimpleAudioVolume_Release(pc_AudioVolume);
return SUCCEEDED(hr) ? 0 : -1;
}
static int MuteSet(audio_output_t *aout, bool mute)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return VLC_EGENERIC;
HRESULT hr;
ISimpleAudioVolume *pc_AudioVolume = NULL;
hr = IAudioClient_GetService(sys->client, &IID_ISimpleAudioVolume, &pc_AudioVolume);
if (FAILED(hr))
{
msg_Err(aout, "cannot get volume service (error 0x%lX)", hr);
goto done;
}
hr = ISimpleAudioVolume_SetMute(pc_AudioVolume, mute, NULL);
if (FAILED(hr))
{
msg_Err(aout, "cannot set mute (error 0x%lX)", hr);
goto done;
}
done:
ISimpleAudioVolume_Release(pc_AudioVolume);
return SUCCEEDED(hr) ? 0 : -1;
}
static int TimeGet(audio_output_t *aout, vlc_tick_t *restrict delay)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return VLC_EGENERIC;
HRESULT hr;
EnterMTA();
hr = aout_stream_TimeGet(sys->stream, delay);
LeaveMTA();
return SUCCEEDED(hr) ? 0 : -1;
}
static void Play(audio_output_t *aout, block_t *block, vlc_tick_t date)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return;
EnterMTA();
HRESULT hr = aout_stream_Play(sys->stream, block);
LeaveMTA();
vlc_FromHR(aout, hr);
(void) date;
}
static void Pause(audio_output_t *aout, bool paused, vlc_tick_t date)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return;
EnterMTA();
HRESULT hr = aout_stream_Pause(sys->stream, paused);
LeaveMTA();
(void) date;
vlc_FromHR(aout, hr);
}
static void Flush(audio_output_t *aout)
{
aout_sys_t *sys = aout->sys;
if( unlikely( sys->client == NULL ) )
return;
EnterMTA();
HRESULT hr = aout_stream_Flush(sys->stream);
LeaveMTA();
vlc_FromHR(aout, hr);
}
static HRESULT ActivateDevice(void *opaque, REFIID iid, PROPVARIANT *actparms,
void **restrict pv)
{
IAudioClient *client = opaque;
if (!IsEqualIID(iid, &IID_IAudioClient))
return E_NOINTERFACE;
if (actparms != NULL || client == NULL )
return E_INVALIDARG;
IAudioClient_AddRef(client);
*pv = opaque;
return S_OK;
}
static int aout_stream_Start(void *func, bool forced, va_list ap)
{
aout_stream_start_t start = func;
aout_stream_t *s = va_arg(ap, aout_stream_t *);
audio_sample_format_t *fmt = va_arg(ap, audio_sample_format_t *);
HRESULT *hr = va_arg(ap, HRESULT *);
(void) forced;
*hr = start(s, fmt, &GUID_VLC_AUD_OUT);
return SUCCEEDED(*hr) ? VLC_SUCCESS : VLC_EGENERIC;
}
static int Start(audio_output_t *aout, audio_sample_format_t *restrict fmt)
{
aout_sys_t *sys = aout->sys;
HRESULT hr;
aout_stream_t *s = vlc_object_create(aout, sizeof (*s));
if (unlikely(s == NULL))
return -1;
s->owner.device = sys->client;
s->owner.activate = ActivateDevice;
EnterMTA();
sys->module = vlc_module_load(s, "aout stream", NULL, false,
aout_stream_Start, s, fmt, &hr);
LeaveMTA();
if (sys->module == NULL)
{
vlc_object_delete(s);
return -1;
}
assert (sys->stream == NULL);
sys->stream = s;
return 0;
}
static void Stop(audio_output_t *aout)
{
aout_sys_t *sys = aout->sys;
assert (sys->stream != NULL);
EnterMTA();
aout_stream_Stop(sys->stream);
LeaveMTA();
vlc_object_delete(sys->stream);
sys->stream = NULL;
}
static int DeviceSelect(audio_output_t *aout, const char* psz_device)
{
if( psz_device == NULL )
return VLC_EGENERIC;
char* psz_end;
aout_sys_t* sys = aout->sys;
intptr_t ptr = strtoll( psz_device, &psz_end, 16 );
if ( *psz_end != 0 )
return VLC_EGENERIC;
if (sys->client == (IAudioClient*)ptr)
return VLC_SUCCESS;
sys->client = (IAudioClient*)ptr;
var_SetAddress( vlc_object_parent(aout), "winstore-client", sys->client );
aout_RestartRequest( aout, AOUT_RESTART_OUTPUT );
return VLC_SUCCESS;
}
static int Open(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
aout->sys = sys;
sys->stream = NULL;
sys->client = var_CreateGetAddress( vlc_object_parent(aout), "winstore-client" );
if (sys->client != NULL)
msg_Dbg( aout, "Reusing previous client: %p", sys->client );
aout->start = Start;
aout->stop = Stop;
aout->time_get = TimeGet;
aout->volume_set = VolumeSet;
aout->mute_set = MuteSet;
aout->play = Play;
aout->pause = Pause;
aout->flush = Flush;
aout->device_select = DeviceSelect;
return VLC_SUCCESS;
}
static void Close(vlc_object_t *obj)
{
audio_output_t *aout = (audio_output_t *)obj;
aout_sys_t *sys = aout->sys;
free(sys);
}
vlc_module_begin()
set_shortname("winstore")
set_description("Windows Store audio output")
set_capability("audio output", 0)
set_category(CAT_AUDIO)
set_subcategory(SUBCAT_AUDIO_AOUT)
add_shortcut("wasapi")
set_callbacks(Open, Close)
vlc_module_end()
