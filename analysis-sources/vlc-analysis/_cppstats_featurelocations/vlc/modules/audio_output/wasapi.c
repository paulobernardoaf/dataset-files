



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#define INITGUID
#define COBJMACROS
#define CONST_VTABLE
#define NONEWWAVE

#include <stdatomic.h>
#include <stdlib.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_codecs.h>
#include <vlc_aout.h>
#include <vlc_plugin.h>

#include <audioclient.h>
#include "audio_output/mmdevice.h"


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL,
WAVE_FORMAT_DOLBY_AC3_SPDIF, 0x0000, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_WAVEFORMATEX,
WAVE_FORMAT_PCM, 0x0000, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEC61937_DTS,
WAVE_FORMAT_DTS_MS, 0x0000, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD,
0x000b, 0x0cea, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS,
0x000a, 0x0cea, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


DEFINE_GUID(_KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP,
0x000c, 0x0cea, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

static BOOL CALLBACK InitFreq(INIT_ONCE *once, void *param, void **context)
{
(void) once; (void) context;
return QueryPerformanceFrequency(param);
}

static LARGE_INTEGER freq; 

static msftime_t GetQPC(void)
{
LARGE_INTEGER counter;

if (!QueryPerformanceCounter(&counter))
abort();

lldiv_t d = lldiv(counter.QuadPart, freq.QuadPart);
return (d.quot * 10000000) + ((d.rem * 10000000) / freq.QuadPart);
}

typedef struct aout_stream_sys
{
IAudioClient *client;
HANDLE hTimer;

#define STARTED_STATE_INIT 0
#define STARTED_STATE_OK 1
#define STARTED_STATE_ERROR 2
atomic_char started_state;

uint8_t chans_table[AOUT_CHAN_MAX];
uint8_t chans_to_reorder;

vlc_fourcc_t format; 
unsigned rate; 
unsigned block_align;
UINT64 written; 
UINT32 frames; 
bool s24s32; 
} aout_stream_sys_t;

static void ResetTimer(aout_stream_t *s)
{
aout_stream_sys_t *sys = s->sys;
if (sys->hTimer != NULL)
{
DeleteTimerQueueTimer(NULL, sys->hTimer, INVALID_HANDLE_VALUE);
sys->hTimer = NULL;
}
}


static HRESULT TimeGet(aout_stream_t *s, vlc_tick_t *restrict delay)
{
aout_stream_sys_t *sys = s->sys;
void *pv;
UINT64 pos, qpcpos, freq;
HRESULT hr;

if (atomic_load(&sys->started_state) != STARTED_STATE_OK)
return E_FAIL;

hr = IAudioClient_GetService(sys->client, &IID_IAudioClock, &pv);
if (FAILED(hr))
{
msg_Err(s, "cannot get clock (error 0x%lX)", hr);
return hr;
}

IAudioClock *clock = pv;

hr = IAudioClock_GetPosition(clock, &pos, &qpcpos);
if (SUCCEEDED(hr))
hr = IAudioClock_GetFrequency(clock, &freq);
IAudioClock_Release(clock);
if (FAILED(hr))
{
msg_Err(s, "cannot get position (error 0x%lX)", hr);
return hr;
}

vlc_tick_t written = vlc_tick_from_frac(sys->written, sys->rate);
vlc_tick_t tick_pos = vlc_tick_from_frac(pos, freq);

static_assert((10000000 % CLOCK_FREQ) == 0, "Frequency conversion broken");

*delay = written - tick_pos
- VLC_TICK_FROM_MSFTIME(GetQPC() - qpcpos);

return hr;
}

static void CALLBACK StartDeferredCallback(void *val, BOOLEAN timeout)
{
aout_stream_t *s = val;
aout_stream_sys_t *sys = s->sys;

HRESULT hr = IAudioClient_Start(sys->client);
atomic_store(&sys->started_state,
SUCCEEDED(hr) ? STARTED_STATE_OK : STARTED_STATE_ERROR);
(void) timeout;
}

static HRESULT StartDeferred(aout_stream_t *s, vlc_tick_t date)
{
aout_stream_sys_t *sys = s->sys;
vlc_tick_t written = vlc_tick_from_frac(sys->written, sys->rate);
vlc_tick_t start_delay = date - vlc_tick_now() - written;
DWORD start_delay_ms = start_delay > 0 ? MS_FROM_VLC_TICK(start_delay) : 0;
BOOL timer_updated = false;


if (start_delay_ms > 0)
{
if (sys->hTimer == NULL)
timer_updated =
CreateTimerQueueTimer(&sys->hTimer, NULL, StartDeferredCallback,
s, start_delay_ms, 0,
WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
else
timer_updated =
ChangeTimerQueueTimer(NULL, sys->hTimer, start_delay_ms, 0);
if (!timer_updated)
msg_Warn(s, "timer update failed, starting now");
}
else
ResetTimer(s);

if (!timer_updated)
{
HRESULT hr = IAudioClient_Start(sys->client);
if (FAILED(hr))
{
atomic_store(&sys->started_state, STARTED_STATE_ERROR);
return hr;
}
atomic_store(&sys->started_state, STARTED_STATE_OK);
}
else
msg_Dbg(s, "deferring start (%"PRId64" us)", start_delay);

return S_OK;
}

static HRESULT Play(aout_stream_t *s, block_t *block, vlc_tick_t date)
{
(void) date;
aout_stream_sys_t *sys = s->sys;
void *pv;
HRESULT hr;

char started_state = atomic_load(&sys->started_state);
if (unlikely(started_state == STARTED_STATE_ERROR))
{
hr = E_FAIL;
goto out;
}
else if (started_state == STARTED_STATE_INIT)
{
hr = StartDeferred(s, date);
if (FAILED(hr))
goto out;
}

if (sys->chans_to_reorder)
aout_ChannelReorder(block->p_buffer, block->i_buffer,
sys->chans_to_reorder, sys->chans_table, sys->format);

hr = IAudioClient_GetService(sys->client, &IID_IAudioRenderClient, &pv);
if (FAILED(hr))
{
msg_Err(s, "cannot get render client (error 0x%lX)", hr);
goto out;
}

IAudioRenderClient *render = pv;
for (;;)
{
UINT32 frames;
hr = IAudioClient_GetCurrentPadding(sys->client, &frames);
if (FAILED(hr))
{
msg_Err(s, "cannot get current padding (error 0x%lX)", hr);
break;
}

assert(frames <= sys->frames);
frames = sys->frames - frames;
if (frames > block->i_nb_samples)
frames = block->i_nb_samples;

BYTE *dst;
hr = IAudioRenderClient_GetBuffer(render, frames, &dst);
if (FAILED(hr))
{
msg_Err(s, "cannot get buffer (error 0x%lX)", hr);
break;
}

const size_t copy = frames * sys->block_align;

if (!sys->s24s32)
{
memcpy(dst, block->p_buffer, copy);
block->p_buffer += copy;
block->i_buffer -= copy;
}
else
{


BYTE *end = dst + copy;
while (dst < end)
{
dst[0] = block->p_buffer[1];
dst[1] = block->p_buffer[2];
dst[2] = block->p_buffer[3];
dst += 3;
block->p_buffer += 4;
block->i_buffer -= 4;
}

}
hr = IAudioRenderClient_ReleaseBuffer(render, frames, 0);
if (FAILED(hr))
{
msg_Err(s, "cannot release buffer (error 0x%lX)", hr);
break;
}

block->i_nb_samples -= frames;
sys->written += frames;
if (block->i_nb_samples == 0)
break; 


vlc_tick_sleep(sys->frames * VLC_TICK_FROM_MS(500) / sys->rate);
}
IAudioRenderClient_Release(render);
out:
block_Release(block);

return hr;
}

static HRESULT Pause(aout_stream_t *s, bool paused)
{
aout_stream_sys_t *sys = s->sys;
HRESULT hr;

if (paused)
{
ResetTimer(s);
if (atomic_load(&sys->started_state) == STARTED_STATE_OK)
hr = IAudioClient_Stop(sys->client);
else
hr = S_OK;

}
else
hr = IAudioClient_Start(sys->client);
if (FAILED(hr))
msg_Warn(s, "cannot %s stream (error 0x%lX)",
paused ? "stop" : "start", hr);
return hr;
}

static HRESULT Flush(aout_stream_t *s)
{
aout_stream_sys_t *sys = s->sys;
HRESULT hr;

ResetTimer(s);

if (atomic_exchange(&sys->started_state, STARTED_STATE_INIT) == STARTED_STATE_OK)
{
IAudioClient_Stop(sys->client);
hr = IAudioClient_Reset(sys->client);
}
else
hr = S_OK;

if (SUCCEEDED(hr))
{
msg_Dbg(s, "reset");
sys->written = 0;
}
else
msg_Warn(s, "cannot reset stream (error 0x%lX)", hr);
return hr;
}



static const uint32_t chans_out[] = {
SPEAKER_FRONT_LEFT, SPEAKER_FRONT_RIGHT,
SPEAKER_FRONT_CENTER, SPEAKER_LOW_FREQUENCY,
SPEAKER_BACK_LEFT, SPEAKER_BACK_RIGHT, SPEAKER_BACK_CENTER,
SPEAKER_SIDE_LEFT, SPEAKER_SIDE_RIGHT, 0
};
static const uint32_t chans_in[] = {
SPEAKER_FRONT_LEFT, SPEAKER_FRONT_RIGHT,
SPEAKER_SIDE_LEFT, SPEAKER_SIDE_RIGHT,
SPEAKER_BACK_LEFT, SPEAKER_BACK_RIGHT, SPEAKER_BACK_CENTER,
SPEAKER_FRONT_CENTER, SPEAKER_LOW_FREQUENCY, 0
};

static void vlc_HdmiToWave(WAVEFORMATEXTENSIBLE_IEC61937 *restrict wf_iec61937,
audio_sample_format_t *restrict audio)
{
WAVEFORMATEXTENSIBLE *wf = &wf_iec61937->FormatExt;

switch (audio->i_format)
{
case VLC_CODEC_DTSHD:
wf->SubFormat = _KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD;
wf->Format.nChannels = 8;
wf->dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
audio->i_rate = 768000;
break;
case VLC_CODEC_EAC3:
wf->SubFormat = _KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS;
wf->Format.nChannels = 2;
wf->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
break;
case VLC_CODEC_TRUEHD:
case VLC_CODEC_MLP:
wf->SubFormat = _KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP;
wf->Format.nChannels = 8;
wf->dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
audio->i_rate = 768000;
break;
default:
vlc_assert_unreachable();
}
wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
wf->Format.nSamplesPerSec = 192000;
wf->Format.wBitsPerSample = 16;
wf->Format.nBlockAlign = wf->Format.wBitsPerSample / 8 * wf->Format.nChannels;
wf->Format.nAvgBytesPerSec = wf->Format.nSamplesPerSec * wf->Format.nBlockAlign;
wf->Format.cbSize = sizeof (*wf_iec61937) - sizeof (wf->Format);

wf->Samples.wValidBitsPerSample = wf->Format.wBitsPerSample;

wf_iec61937->dwEncodedSamplesPerSec = audio->i_rate;
wf_iec61937->dwEncodedChannelCount = audio->i_channels;
wf_iec61937->dwAverageBytesPerSec = 0;

audio->i_format = VLC_CODEC_SPDIFL;
audio->i_bytes_per_frame = wf->Format.nBlockAlign;
audio->i_frame_length = 1;
}

static void vlc_SpdifToWave(WAVEFORMATEXTENSIBLE *restrict wf,
audio_sample_format_t *restrict audio)
{
switch (audio->i_format)
{
case VLC_CODEC_DTS:
if (audio->i_rate < 48000)
{

wf->SubFormat = _KSDATAFORMAT_SUBTYPE_WAVEFORMATEX;
}
else
wf->SubFormat = _KSDATAFORMAT_SUBTYPE_IEC61937_DTS;
break;
case VLC_CODEC_SPDIFL:
case VLC_CODEC_SPDIFB:
case VLC_CODEC_A52:
wf->SubFormat = _KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL;
break;
default:
vlc_assert_unreachable();
}

wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
wf->Format.nChannels = 2; 
wf->Format.nSamplesPerSec = audio->i_rate;
wf->Format.wBitsPerSample = 16;
wf->Format.nBlockAlign = 4; 
wf->Format.nAvgBytesPerSec = wf->Format.nSamplesPerSec * wf->Format.nBlockAlign;
wf->Format.cbSize = sizeof (*wf) - sizeof (wf->Format);

wf->Samples.wValidBitsPerSample = wf->Format.wBitsPerSample;

wf->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;

audio->i_format = VLC_CODEC_SPDIFL;
audio->i_bytes_per_frame = wf->Format.nBlockAlign;
audio->i_frame_length = 1;
}

static void vlc_ToWave(WAVEFORMATEXTENSIBLE *restrict wf,
audio_sample_format_t *restrict audio)
{
switch (audio->i_format)
{
case VLC_CODEC_FL64:
audio->i_format = VLC_CODEC_FL32;

case VLC_CODEC_FL32:
wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
break;

case VLC_CODEC_U8:
audio->i_format = VLC_CODEC_S16N;

case VLC_CODEC_S16N:
wf->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
break;

default:
audio->i_format = VLC_CODEC_FL32;
wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
break;
}
aout_FormatPrepare (audio);

wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
wf->Format.nChannels = audio->i_channels;
wf->Format.nSamplesPerSec = audio->i_rate;
wf->Format.nAvgBytesPerSec = audio->i_bytes_per_frame * audio->i_rate;
wf->Format.nBlockAlign = audio->i_bytes_per_frame;
wf->Format.wBitsPerSample = audio->i_bitspersample;
wf->Format.cbSize = sizeof (*wf) - sizeof (wf->Format);

wf->Samples.wValidBitsPerSample = audio->i_bitspersample;

wf->dwChannelMask = 0;
for (unsigned i = 0; pi_vlc_chan_order_wg4[i]; i++)
if (audio->i_physical_channels & pi_vlc_chan_order_wg4[i])
wf->dwChannelMask |= chans_in[i];
}

static int vlc_FromWave(const WAVEFORMATEX *restrict wf,
audio_sample_format_t *restrict audio)
{
audio->i_rate = wf->nSamplesPerSec;
audio->i_physical_channels = 0;

if (wf->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
{
const WAVEFORMATEXTENSIBLE *wfe = (void *)wf;

if (IsEqualIID(&wfe->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
{
switch (wf->wBitsPerSample)
{
case 64:
audio->i_format = VLC_CODEC_FL64;
break;
case 32:
audio->i_format = VLC_CODEC_FL32;
break;
default:
return -1;
}
}
else if (IsEqualIID(&wfe->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM))
{
switch (wf->wBitsPerSample)
{
case 32:
case 24:
audio->i_format = VLC_CODEC_S32N;
break;
case 16:
audio->i_format = VLC_CODEC_S16N;
break;
default:
return -1;
}
}

if (wfe->Samples.wValidBitsPerSample != wf->wBitsPerSample)
return -1;

for (unsigned i = 0; chans_in[i]; i++)
if (wfe->dwChannelMask & chans_in[i])
audio->i_physical_channels |= pi_vlc_chan_order_wg4[i];
}
else
return -1;

aout_FormatPrepare (audio);

if (wf->nChannels != audio->i_channels)
return -1;
return 0;
}

static unsigned vlc_CheckWaveOrder (const WAVEFORMATEX *restrict wf,
uint8_t *restrict table)
{
uint32_t mask = 0;

if (wf->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
{
const WAVEFORMATEXTENSIBLE *wfe = (void *)wf;

mask = wfe->dwChannelMask;
}
return aout_CheckChannelReorder(chans_in, chans_out, mask, table);
}

static void Stop(aout_stream_t *s)
{
aout_stream_sys_t *sys = s->sys;

ResetTimer(s);

if (atomic_load(&sys->started_state) == STARTED_STATE_OK)
IAudioClient_Stop(sys->client);

IAudioClient_Release(sys->client);

free(sys);
}







static HRESULT GetExclusivePCMFormat(IAudioClient *c, const WAVEFORMATEX *pwf,
WAVEFORMATEX **ppwf_closest)
{
HRESULT hr;
const AUDCLNT_SHAREMODE exclusive = AUDCLNT_SHAREMODE_EXCLUSIVE;

*ppwf_closest = NULL;


hr = IAudioClient_IsFormatSupported(c, exclusive, pwf, NULL);

if (hr != AUDCLNT_E_UNSUPPORTED_FORMAT)
{
assert(hr != S_FALSE); 
return hr;
}


assert(pwf->wFormatTag == WAVE_FORMAT_EXTENSIBLE);
const WAVEFORMATEXTENSIBLE *pwfe = (void *) pwf;
assert(IsEqualIID(&pwfe->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
|| IsEqualIID(&pwfe->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM));


WAVEFORMATEXTENSIBLE *pwfe_closest =
CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE));
if (!pwfe_closest)
return E_FAIL;
WAVEFORMATEX *pwf_closest = &pwfe_closest->Format;









static const uint16_t bits_pcm8_candidates[] = { 8, 16, 24, 32 };
static const uint16_t bits_pcm16_candidates[] = { 16, 24, 32, 8 };
static const uint16_t bits_pcm24_candidates[] = { 24, 32, 16, 8 };
static const uint16_t bits_pcm32_candidates[] = { 32, 24, 16, 8 };

static const size_t bits_candidates_size = ARRAY_SIZE(bits_pcm8_candidates);

const uint16_t *bits_candidates;
switch (pwf->wBitsPerSample)
{
case 64: 
case 32: bits_candidates = bits_pcm32_candidates; break;
case 24: bits_candidates = bits_pcm24_candidates; break;
case 16: bits_candidates = bits_pcm16_candidates; break;
case 8: bits_candidates = bits_pcm8_candidates; break;
default: vlc_assert_unreachable();
}


const uint32_t samplerate_candidates[] = {
pwf->nSamplesPerSec,
pwf->nSamplesPerSec == 48000 ? 0 : 48000,
pwf->nSamplesPerSec == 44100 ? 0 : 44100,
};
const size_t samplerate_candidates_size = ARRAY_SIZE(samplerate_candidates);


const uint16_t channels_candidates[] = {
pwf->nChannels,
pwf->nChannels == 2 ? 0 : 2,
};
const size_t channels_candidates_size = ARRAY_SIZE(channels_candidates);


for (size_t bits_idx = 0; bits_idx < bits_candidates_size; ++bits_idx)
{
uint16_t bits = bits_candidates[bits_idx];

for (size_t samplerate_idx = 0;
samplerate_idx < samplerate_candidates_size;
++samplerate_idx)
{
const uint32_t samplerate = samplerate_candidates[samplerate_idx];

if (samplerate == 0)
continue;

for (size_t channels_idx = 0;
channels_idx < channels_candidates_size;
++channels_idx)
{
const uint16_t channels = channels_candidates[channels_idx];
if (channels == 0)
continue;

pwfe_closest->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
pwfe_closest->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
pwfe_closest->Format.nSamplesPerSec = samplerate;
pwfe_closest->Format.wBitsPerSample = bits;
pwfe_closest->Samples.wValidBitsPerSample = bits;
pwfe_closest->Format.nChannels = channels;
pwfe_closest->Format.nBlockAlign = bits / 8 * channels;
pwfe_closest->Format.nAvgBytesPerSec =
pwfe_closest->Format.nBlockAlign * samplerate;

if (channels == pwf->nChannels)
{

pwfe_closest->dwChannelMask = pwfe->dwChannelMask;
}
else
{
assert(channels == 2);
pwfe_closest->dwChannelMask =
SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
}
pwfe_closest->Format.cbSize = pwfe->Format.cbSize;

hr = IAudioClient_IsFormatSupported(c, exclusive,
pwf_closest, NULL);

if (hr != AUDCLNT_E_UNSUPPORTED_FORMAT)
{
if (hr == S_OK)
{
*ppwf_closest = pwf_closest;


return S_FALSE;
}
assert(hr != S_FALSE); 
CoTaskMemFree(pwfe_closest);
return hr; 
}
}
}
}


CoTaskMemFree(pwfe_closest);
return AUDCLNT_E_UNSUPPORTED_FORMAT;
}

static HRESULT Start(aout_stream_t *s, audio_sample_format_t *restrict pfmt,
const GUID *sid)
{
static INIT_ONCE freq_once = INIT_ONCE_STATIC_INIT;

if (!InitOnceExecuteOnce(&freq_once, InitFreq, &freq, NULL))
return E_FAIL;

aout_stream_sys_t *sys = malloc(sizeof (*sys));
if (unlikely(sys == NULL))
return E_OUTOFMEMORY;
sys->client = NULL;
sys->hTimer = NULL;
atomic_init(&sys->started_state, STARTED_STATE_INIT);


WAVEFORMATEXTENSIBLE_IEC61937 wf_iec61937;
WAVEFORMATEXTENSIBLE *pwfe = &wf_iec61937.FormatExt;
WAVEFORMATEX *pwf = &pwfe->Format, *pwf_closest, *pwf_mix = NULL;
AUDCLNT_SHAREMODE shared_mode;
REFERENCE_TIME buffer_duration;
audio_sample_format_t fmt = *pfmt;
bool b_spdif = AOUT_FMT_SPDIF(&fmt);
bool b_hdmi = AOUT_FMT_HDMI(&fmt);

void *pv;
HRESULT hr = aout_stream_Activate(s, &IID_IAudioClient, NULL, &pv);
if (FAILED(hr))
{
msg_Err(s, "cannot activate client (error 0x%lX)", hr);
goto error;
}
sys->client = pv;


if (b_spdif || b_hdmi)
{
if (b_spdif)
vlc_SpdifToWave(pwfe, &fmt);
else
vlc_HdmiToWave(&wf_iec61937, &fmt);

shared_mode = AUDCLNT_SHAREMODE_EXCLUSIVE;

buffer_duration = MSFTIME_FROM_MS(200);

hr = IAudioClient_IsFormatSupported(sys->client, shared_mode,
pwf, NULL);
pwf_closest = NULL;
}
else if (AOUT_FMT_LINEAR(&fmt))
{
if (fmt.channel_type == AUDIO_CHANNEL_TYPE_AMBISONICS)
{
fmt.channel_type = AUDIO_CHANNEL_TYPE_BITMAP;


hr = IAudioClient_GetMixFormat(sys->client, &pwf_mix);
if (FAILED(hr) || vlc_FromWave(pwf_mix, &fmt))
vlc_ToWave(pwfe, &fmt); 
else
pwf = pwf_mix;



buffer_duration = MSFTIME_FROM_MS(200);
}
else
{
vlc_ToWave(pwfe, &fmt);
buffer_duration = MSFTIME_FROM_VLC_TICK(AOUT_MAX_PREPARE_TIME);
}


if (var_CreateGetBool(vlc_object_parent(s), "wasapi-exclusive"))
{
shared_mode = AUDCLNT_SHAREMODE_EXCLUSIVE;
buffer_duration = MSFTIME_FROM_MS(200);
hr = GetExclusivePCMFormat(sys->client, pwf, &pwf_closest);
}
else
{
shared_mode = AUDCLNT_SHAREMODE_SHARED;
hr = IAudioClient_IsFormatSupported(sys->client, shared_mode,
pwf, &pwf_closest);
}
}
else
hr = E_FAIL;


if (FAILED(hr))
{
msg_Err(s, "cannot negotiate audio format (error 0x%lX)%s", hr,
hr == AUDCLNT_E_UNSUPPORTED_FORMAT
&& fmt.i_format == VLC_CODEC_SPDIFL ?
": digital pass-through not supported" : "");
goto error;
}

if (hr == S_FALSE)
{
assert(pwf_closest != NULL);
if (vlc_FromWave(pwf_closest, &fmt))
{
CoTaskMemFree(pwf_closest);
msg_Err(s, "unsupported audio format");
hr = E_INVALIDARG;
goto error;
}
msg_Dbg(s, "modified format");
pwf = pwf_closest;
}
else
assert(pwf_closest == NULL);

sys->chans_to_reorder = fmt.i_format != VLC_CODEC_SPDIFL ?
vlc_CheckWaveOrder(pwf, sys->chans_table) : 0;
sys->format = fmt.i_format;
sys->block_align = pwf->nBlockAlign;
sys->rate = pwf->nSamplesPerSec;
sys->s24s32 = pwf->wBitsPerSample == 24 && fmt.i_format == VLC_CODEC_S32N;
if (sys->s24s32)
msg_Dbg(s, "audio device configured as s24");

hr = IAudioClient_Initialize(sys->client, shared_mode, 0, buffer_duration,
0, pwf, sid);
CoTaskMemFree(pwf_closest);
if (FAILED(hr))
{
msg_Err(s, "cannot initialize audio client (error 0x%lX)", hr);
goto error;
}

hr = IAudioClient_GetBufferSize(sys->client, &sys->frames);
if (FAILED(hr))
{
msg_Err(s, "cannot get buffer size (error 0x%lX)", hr);
goto error;
}
msg_Dbg(s, "buffer size : %"PRIu32" frames", sys->frames);

REFERENCE_TIME latT, defT, minT;
if (SUCCEEDED(IAudioClient_GetStreamLatency(sys->client, &latT))
&& SUCCEEDED(IAudioClient_GetDevicePeriod(sys->client, &defT, &minT)))
{
msg_Dbg(s, "maximum latency: %"PRIu64"00 ns", latT);
msg_Dbg(s, "default period : %"PRIu64"00 ns", defT);
msg_Dbg(s, "minimum period : %"PRIu64"00 ns", minT);
}

CoTaskMemFree(pwf_mix);
*pfmt = fmt;
sys->written = 0;
s->sys = sys;
s->time_get = TimeGet;
s->play = Play;
s->pause = Pause;
s->flush = Flush;
s->stop = Stop;
return S_OK;
error:
CoTaskMemFree(pwf_mix);
if (sys->client != NULL)
IAudioClient_Release(sys->client);
free(sys);
return hr;
}

#define WASAPI_EXCLUSIVE_TEXT N_("Use exclusive mode")
#define WASAPI_EXCLUSIVE_LONGTEXT N_( "VLC will have a direct connection of the audio endpoint device. " "This mode can be used to reduce the audio latency or " "to assure that the audio stream won't be modified by the OS. " "This mode is more likely to fail if the soundcard format is not " "handled by VLC.")






vlc_module_begin()
set_shortname("WASAPI")
set_description(N_("Windows Audio Session API output"))
set_capability("aout stream", 50)
set_category(CAT_AUDIO)
add_bool("wasapi-exclusive", false, WASAPI_EXCLUSIVE_TEXT,
WASAPI_EXCLUSIVE_LONGTEXT, true)
set_subcategory(SUBCAT_AUDIO_AOUT)
set_callback(Start)
vlc_module_end()
