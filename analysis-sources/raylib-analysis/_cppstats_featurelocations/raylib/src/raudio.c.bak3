


































































#if defined(RAUDIO_STANDALONE)
#include "raudio.h"
#include <stdarg.h> 
#else
#include "raylib.h" 


#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif
#include "utils.h" 
#endif

#if defined(_WIN32)






#define NOGDICAPMASKS 
#define NOVIRTUALKEYCODES 
#define NOWINMESSAGES 
#define NOWINSTYLES 
#define NOSYSMETRICS 
#define NOMENUS 
#define NOICONS 
#define NOKEYSTATES 
#define NOSYSCOMMANDS 
#define NORASTEROPS 
#define NOSHOWWINDOW 
#define OEMRESOURCE 
#define NOATOM 
#define NOCLIPBOARD 
#define NOCOLOR 
#define NOCTLMGR 
#define NODRAWTEXT 
#define NOGDI 
#define NOKERNEL 
#define NOUSER 

#define NOMB 
#define NOMEMMGR 
#define NOMETAFILE 
#define NOMINMAX 
#define NOMSG 
#define NOOPENFILE 
#define NOSCROLL 
#define NOSERVICE 
#define NOSOUND 
#define NOTEXTMETRIC 
#define NOWH 
#define NOWINOFFSETS 
#define NOCOMM 
#define NOKANJI 
#define NOHELP 
#define NOPROFILER 
#define NODEFERWINDOWPOS 
#define NOMCX 


typedef struct tagMSG *LPMSG;

#include <windows.h>


typedef struct tagBITMAPINFOHEADER {
DWORD biSize;
LONG biWidth;
LONG biHeight;
WORD biPlanes;
WORD biBitCount;
DWORD biCompression;
DWORD biSizeImage;
LONG biXPelsPerMeter;
LONG biYPelsPerMeter;
DWORD biClrUsed;
DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>


#if defined(_MSC_VER) || defined(__TINYC__)
#include "propidl.h"
#endif
#endif

#define MA_MALLOC RL_MALLOC
#define MA_FREE RL_FREE

#define MA_NO_JACK
#define MINIAUDIO_IMPLEMENTATION
#include "external/miniaudio.h" 
#undef PlaySound 

#include <stdlib.h> 
#include <stdio.h> 

#if defined(RAUDIO_STANDALONE)
#include <string.h> 

#if !defined(TRACELOG)
#define TRACELOG(level, ...) (void)0
#endif
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)


#define STB_VORBIS_IMPLEMENTATION
#include "external/stb_vorbis.h" 
#endif

#if defined(SUPPORT_FILEFORMAT_XM)
#define JARXM_MALLOC RL_MALLOC
#define JARXM_FREE RL_FREE

#define JAR_XM_IMPLEMENTATION
#include "external/jar_xm.h" 
#endif

#if defined(SUPPORT_FILEFORMAT_MOD)
#define JARMOD_MALLOC RL_MALLOC
#define JARMOD_FREE RL_FREE

#define JAR_MOD_IMPLEMENTATION
#include "external/jar_mod.h" 
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
#define DRFLAC_MALLOC RL_MALLOC
#define DRFLAC_REALLOC RL_REALLOC
#define DRFLAC_FREE RL_FREE

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_WIN32_IO
#include "external/dr_flac.h" 
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
#define DRMP3_MALLOC RL_MALLOC
#define DRMP3_REALLOC RL_REALLOC
#define DRMP3_FREE RL_FREE

#define DR_MP3_IMPLEMENTATION
#include "external/dr_mp3.h" 
#endif

#if defined(_MSC_VER)
#undef bool
#endif




#define AUDIO_DEVICE_FORMAT ma_format_f32
#define AUDIO_DEVICE_CHANNELS 2
#define AUDIO_DEVICE_SAMPLE_RATE 44100

#define MAX_AUDIO_BUFFER_POOL_CHANNELS 16








typedef enum {
MUSIC_AUDIO_WAV = 0,
MUSIC_AUDIO_OGG,
MUSIC_AUDIO_FLAC,
MUSIC_AUDIO_MP3,
MUSIC_MODULE_XM,
MUSIC_MODULE_MOD
} MusicContextType;

#if defined(RAUDIO_STANDALONE)
typedef enum {
LOG_ALL,
LOG_TRACE,
LOG_DEBUG,
LOG_INFO,
LOG_WARNING,
LOG_ERROR,
LOG_FATAL,
LOG_NONE
} TraceLogType;
#endif



typedef enum {
AUDIO_BUFFER_USAGE_STATIC = 0,
AUDIO_BUFFER_USAGE_STREAM
} AudioBufferUsage;


struct rAudioBuffer {
ma_data_converter converter; 

float volume; 
float pitch; 

bool playing; 
bool paused; 
bool looping; 
int usage; 

bool isSubBufferProcessed[2]; 
unsigned int sizeInFrames; 
unsigned int frameCursorPos; 
unsigned int totalFramesProcessed; 

unsigned char *data; 

rAudioBuffer *next; 
rAudioBuffer *prev; 
};

#define AudioBuffer rAudioBuffer 


typedef struct AudioData {
struct {
ma_context context; 
ma_device device; 
ma_mutex lock; 
bool isReady; 
} System;
struct {
AudioBuffer *first; 
AudioBuffer *last; 
int defaultSize; 
} Buffer;
struct {
AudioBuffer *pool[MAX_AUDIO_BUFFER_POOL_CHANNELS]; 
unsigned int poolCounter; 
unsigned int channels[MAX_AUDIO_BUFFER_POOL_CHANNELS]; 
} MultiChannel;
} AudioData;




static AudioData AUDIO = { 





.Buffer.defaultSize = 4096
};




static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message);
static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount);
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume);

static void InitAudioBufferPool(void); 
static void CloseAudioBufferPool(void); 

#if defined(SUPPORT_FILEFORMAT_WAV)
static Wave LoadWAV(const char *fileName); 
static int SaveWAV(Wave wave, const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
static Wave LoadOGG(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
static Wave LoadFLAC(const char *fileName); 
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
static Wave LoadMP3(const char *fileName); 
#endif

#if defined(RAUDIO_STANDALONE)
bool IsFileExtension(const char *fileName, const char *ext);
void TraceLog(int msgType, const char *text, ...); 
#endif





AudioBuffer *LoadAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage);
void UnloadAudioBuffer(AudioBuffer *buffer);

bool IsAudioBufferPlaying(AudioBuffer *buffer);
void PlayAudioBuffer(AudioBuffer *buffer);
void StopAudioBuffer(AudioBuffer *buffer);
void PauseAudioBuffer(AudioBuffer *buffer);
void ResumeAudioBuffer(AudioBuffer *buffer);
void SetAudioBufferVolume(AudioBuffer *buffer, float volume);
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch);
void TrackAudioBuffer(AudioBuffer *buffer);
void UntrackAudioBuffer(AudioBuffer *buffer);





void InitAudioDevice(void)
{



ma_context_config ctxConfig = ma_context_config_init();
ctxConfig.logCallback = OnLog;

ma_result result = ma_context_init(NULL, 0, &ctxConfig, &AUDIO.System.context);
if (result != MA_SUCCESS)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to initialize context");
return;
}



ma_device_config config = ma_device_config_init(ma_device_type_playback);
config.playback.pDeviceID = NULL; 
config.playback.format = AUDIO_DEVICE_FORMAT;
config.playback.channels = AUDIO_DEVICE_CHANNELS;
config.capture.pDeviceID = NULL; 
config.capture.format = ma_format_s16;
config.capture.channels = 1;
config.sampleRate = AUDIO_DEVICE_SAMPLE_RATE;
config.dataCallback = OnSendAudioDataToDevice;
config.pUserData = NULL;

result = ma_device_init(&AUDIO.System.context, &config, &AUDIO.System.device);
if (result != MA_SUCCESS)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to initialize playback device");
ma_context_uninit(&AUDIO.System.context);
return;
}



result = ma_device_start(&AUDIO.System.device);
if (result != MA_SUCCESS)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to start playback device");
ma_device_uninit(&AUDIO.System.device);
ma_context_uninit(&AUDIO.System.context);
return;
}



if (ma_mutex_init(&AUDIO.System.context, &AUDIO.System.lock) != MA_SUCCESS)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to create mutex for mixing");
ma_device_uninit(&AUDIO.System.device);
ma_context_uninit(&AUDIO.System.context);
return;
}

TRACELOG(LOG_INFO, "AUDIO: Device initialized successfully");
TRACELOG(LOG_INFO, " > Backend: miniaudio / %s", ma_get_backend_name(AUDIO.System.context.backend));
TRACELOG(LOG_INFO, " > Format: %s -> %s", ma_get_format_name(AUDIO.System.device.playback.format), ma_get_format_name(AUDIO.System.device.playback.internalFormat));
TRACELOG(LOG_INFO, " > Channels: %d -> %d", AUDIO.System.device.playback.channels, AUDIO.System.device.playback.internalChannels);
TRACELOG(LOG_INFO, " > Sample rate: %d -> %d", AUDIO.System.device.sampleRate, AUDIO.System.device.playback.internalSampleRate);
TRACELOG(LOG_INFO, " > Periods size: %d", AUDIO.System.device.playback.internalPeriodSizeInFrames*AUDIO.System.device.playback.internalPeriods);

InitAudioBufferPool();

AUDIO.System.isReady = true;
}


void CloseAudioDevice(void)
{
if (AUDIO.System.isReady)
{
ma_mutex_uninit(&AUDIO.System.lock);
ma_device_uninit(&AUDIO.System.device);
ma_context_uninit(&AUDIO.System.context);

CloseAudioBufferPool();

TRACELOG(LOG_INFO, "AUDIO: Device closed successfully");
}
else TRACELOG(LOG_WARNING, "AUDIO: Device could not be closed, not currently initialized");
}


bool IsAudioDeviceReady(void)
{
return AUDIO.System.isReady;
}


void SetMasterVolume(float volume)
{
ma_device_set_master_volume(&AUDIO.System.device, volume);
}






AudioBuffer *LoadAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage)
{
AudioBuffer *audioBuffer = (AudioBuffer *)RL_CALLOC(1, sizeof(AudioBuffer));

if (audioBuffer == NULL)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to allocate memory for buffer");
return NULL;
}

audioBuffer->data = RL_CALLOC(sizeInFrames*channels*ma_get_bytes_per_sample(format), 1);


ma_data_converter_config converterConfig = ma_data_converter_config_init(format, AUDIO_DEVICE_FORMAT, channels, AUDIO_DEVICE_CHANNELS, sampleRate, AUDIO_DEVICE_SAMPLE_RATE);
converterConfig.resampling.allowDynamicSampleRate = true; 

ma_result result = ma_data_converter_init(&converterConfig, &audioBuffer->converter);

if (result != MA_SUCCESS)
{
TRACELOG(LOG_ERROR, "AUDIO: Failed to create data conversion pipeline");
RL_FREE(audioBuffer);
return NULL;
}


audioBuffer->volume = 1.0f;
audioBuffer->pitch = 1.0f;
audioBuffer->playing = false;
audioBuffer->paused = false;
audioBuffer->looping = false;
audioBuffer->usage = usage;
audioBuffer->frameCursorPos = 0;
audioBuffer->sizeInFrames = sizeInFrames;



audioBuffer->isSubBufferProcessed[0] = true;
audioBuffer->isSubBufferProcessed[1] = true;


TrackAudioBuffer(audioBuffer);

return audioBuffer;
}


void UnloadAudioBuffer(AudioBuffer *buffer)
{
if (buffer != NULL)
{
ma_data_converter_uninit(&buffer->converter);
UntrackAudioBuffer(buffer);
RL_FREE(buffer->data);
RL_FREE(buffer);
}
}


bool IsAudioBufferPlaying(AudioBuffer *buffer)
{
bool result = false;

if (buffer != NULL) result = (buffer->playing && !buffer->paused);

return result;
}




void PlayAudioBuffer(AudioBuffer *buffer)
{
if (buffer != NULL)
{
buffer->playing = true;
buffer->paused = false;
buffer->frameCursorPos = 0;
}
}


void StopAudioBuffer(AudioBuffer *buffer)
{
if (buffer != NULL)
{
if (IsAudioBufferPlaying(buffer))
{
buffer->playing = false;
buffer->paused = false;
buffer->frameCursorPos = 0;
buffer->totalFramesProcessed = 0;
buffer->isSubBufferProcessed[0] = true;
buffer->isSubBufferProcessed[1] = true;
}
}
}


void PauseAudioBuffer(AudioBuffer *buffer)
{
if (buffer != NULL) buffer->paused = true;
}


void ResumeAudioBuffer(AudioBuffer *buffer)
{
if (buffer != NULL) buffer->paused = false;
}


void SetAudioBufferVolume(AudioBuffer *buffer, float volume)
{
if (buffer != NULL) buffer->volume = volume;
}


void SetAudioBufferPitch(AudioBuffer *buffer, float pitch)
{
if (buffer != NULL)
{
float pitchMul = pitch/buffer->pitch;





ma_uint32 newOutputSampleRate = (ma_uint32)((float)buffer->converter.config.sampleRateOut/pitchMul);
buffer->pitch *= (float)buffer->converter.config.sampleRateOut/newOutputSampleRate;

ma_data_converter_set_rate(&buffer->converter, buffer->converter.config.sampleRateIn, newOutputSampleRate);
}
}


void TrackAudioBuffer(AudioBuffer *buffer)
{
ma_mutex_lock(&AUDIO.System.lock);
{
if (AUDIO.Buffer.first == NULL) AUDIO.Buffer.first = buffer;
else
{
AUDIO.Buffer.last->next = buffer;
buffer->prev = AUDIO.Buffer.last;
}

AUDIO.Buffer.last = buffer;
}
ma_mutex_unlock(&AUDIO.System.lock);
}


void UntrackAudioBuffer(AudioBuffer *buffer)
{
ma_mutex_lock(&AUDIO.System.lock);
{
if (buffer->prev == NULL) AUDIO.Buffer.first = buffer->next;
else buffer->prev->next = buffer->next;

if (buffer->next == NULL) AUDIO.Buffer.last = buffer->prev;
else buffer->next->prev = buffer->prev;

buffer->prev = NULL;
buffer->next = NULL;
}
ma_mutex_unlock(&AUDIO.System.lock);
}






Wave LoadWave(const char *fileName)
{
Wave wave = { 0 };

if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
else if (IsFileExtension(fileName, ".wav")) wave = LoadWAV(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
else if (IsFileExtension(fileName, ".ogg")) wave = LoadOGG(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
else if (IsFileExtension(fileName, ".flac")) wave = LoadFLAC(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
else if (IsFileExtension(fileName, ".mp3")) wave = LoadMP3(fileName);
#endif
else TRACELOG(LOG_WARNING, "FILEIO: [%s] File format not supported", fileName);

return wave;
}



Sound LoadSound(const char *fileName)
{
Wave wave = LoadWave(fileName);

Sound sound = LoadSoundFromWave(wave);

UnloadWave(wave); 

return sound;
}



Sound LoadSoundFromWave(Wave wave)
{
Sound sound = { 0 };

if (wave.data != NULL)
{









ma_format formatIn = ((wave.sampleSize == 8)? ma_format_u8 : ((wave.sampleSize == 16)? ma_format_s16 : ma_format_f32));
ma_uint32 frameCountIn = wave.sampleCount/wave.channels;

ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, 0, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO_DEVICE_SAMPLE_RATE, NULL, frameCountIn, formatIn, wave.channels, wave.sampleRate);
if (frameCount == 0) TRACELOG(LOG_WARNING, "SOUND: Failed to get frame count for format conversion");

AudioBuffer *audioBuffer = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO_DEVICE_SAMPLE_RATE, frameCount, AUDIO_BUFFER_USAGE_STATIC);
if (audioBuffer == NULL) TRACELOG(LOG_WARNING, "SOUND: Failed to create buffer");

frameCount = (ma_uint32)ma_convert_frames(audioBuffer->data, frameCount, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO_DEVICE_SAMPLE_RATE, wave.data, frameCountIn, formatIn, wave.channels, wave.sampleRate);
if (frameCount == 0) TRACELOG(LOG_WARNING, "SOUND: Failed format conversion");

sound.sampleCount = frameCount*AUDIO_DEVICE_CHANNELS;
sound.stream.sampleRate = AUDIO_DEVICE_SAMPLE_RATE;
sound.stream.sampleSize = 32;
sound.stream.channels = AUDIO_DEVICE_CHANNELS;
sound.stream.buffer = audioBuffer;
}

return sound;
}


void UnloadWave(Wave wave)
{
if (wave.data != NULL) RL_FREE(wave.data);

TRACELOG(LOG_INFO, "WAVE: Unloaded wave data from RAM");
}


void UnloadSound(Sound sound)
{
UnloadAudioBuffer(sound.stream.buffer);

TRACELOG(LOG_INFO, "WAVE: Unloaded sound data from RAM");
}


void UpdateSound(Sound sound, const void *data, int samplesCount)
{
if (sound.stream.buffer != NULL)
{
StopAudioBuffer(sound.stream.buffer);


memcpy(sound.stream.buffer->data, data, samplesCount*ma_get_bytes_per_frame(sound.stream.buffer->converter.config.formatIn, sound.stream.buffer->converter.config.channelsIn));
}
}


void ExportWave(Wave wave, const char *fileName)
{
bool success = false;

if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
else if (IsFileExtension(fileName, ".wav")) success = SaveWAV(wave, fileName);
#endif
else if (IsFileExtension(fileName, ".raw"))
{


SaveFileData(fileName, wave.data, wave.sampleCount*wave.channels*wave.sampleSize/8);
success = true;
}

if (success) TRACELOG(LOG_INFO, "FILEIO: [%s] Wave data exported successfully", fileName);
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export wave data", fileName);
}


void ExportWaveAsCode(Wave wave, const char *fileName)
{
#define BYTES_TEXT_PER_LINE 20

char varFileName[256] = { 0 };
int dataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;

FILE *txtFile = fopen(fileName, "wt");

if (txtFile != NULL)
{
fprintf(txtFile, "\n//////////////////////////////////////////////////////////////////////////////////\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// WaveAsCode exporter v1.0 - Wave data exported as an array of bytes //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// more info and bugs-report: github.com/raysan5/raylib //\n");
fprintf(txtFile, "// feedback and support: ray[at]raylib.com //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "// Copyright (c) 2018 Ramon Santamaria (@raysan5) //\n");
fprintf(txtFile, "// //\n");
fprintf(txtFile, "//////////////////////////////////////////////////////////////////////////////////\n\n");

#if !defined(RAUDIO_STANDALONE)

strcpy(varFileName, GetFileNameWithoutExt(fileName));
for (int i = 0; varFileName[i] != '\0'; i++) if (varFileName[i] >= 'a' && varFileName[i] <= 'z') { varFileName[i] = varFileName[i] - 32; }
#else
strcpy(varFileName, fileName);
#endif

fprintf(txtFile, "// Wave data information\n");
fprintf(txtFile, "#define %s_SAMPLE_COUNT %u\n", varFileName, wave.sampleCount);
fprintf(txtFile, "#define %s_SAMPLE_RATE %u\n", varFileName, wave.sampleRate);
fprintf(txtFile, "#define %s_SAMPLE_SIZE %u\n", varFileName, wave.sampleSize);
fprintf(txtFile, "#define %s_CHANNELS %u\n\n", varFileName, wave.channels);


fprintf(txtFile, "static unsigned char %s_DATA[%i] = { ", varFileName, dataSize);
for (int i = 0; i < dataSize - 1; i++) fprintf(txtFile, ((i%BYTES_TEXT_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), ((unsigned char *)wave.data)[i]);
fprintf(txtFile, "0x%x };\n", ((unsigned char *)wave.data)[dataSize - 1]);

fclose(txtFile);
}
}


void PlaySound(Sound sound)
{
PlayAudioBuffer(sound.stream.buffer);
}


void PlaySoundMulti(Sound sound)
{
int index = -1;
unsigned int oldAge = 0;
int oldIndex = -1;


for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
{
if (AUDIO.MultiChannel.channels[i] > oldAge)
{
oldAge = AUDIO.MultiChannel.channels[i];
oldIndex = i;
}

if (!IsAudioBufferPlaying(AUDIO.MultiChannel.pool[i]))
{
index = i;
break;
}
}


if (index == -1)
{
TRACELOG(LOG_WARNING, "SOUND: Buffer pool is already full, count: %i", AUDIO.MultiChannel.poolCounter);

if (oldIndex == -1)
{

TRACELOG(LOG_WARNING, "SOUND: Buffer pool could not determine oldest buffer not playing sound");
return;
}

index = oldIndex;


StopAudioBuffer(AUDIO.MultiChannel.pool[index]);
}





AUDIO.MultiChannel.channels[index] = AUDIO.MultiChannel.poolCounter;
AUDIO.MultiChannel.poolCounter++;

AUDIO.MultiChannel.pool[index]->volume = sound.stream.buffer->volume;
AUDIO.MultiChannel.pool[index]->pitch = sound.stream.buffer->pitch;
AUDIO.MultiChannel.pool[index]->looping = sound.stream.buffer->looping;
AUDIO.MultiChannel.pool[index]->usage = sound.stream.buffer->usage;
AUDIO.MultiChannel.pool[index]->isSubBufferProcessed[0] = false;
AUDIO.MultiChannel.pool[index]->isSubBufferProcessed[1] = false;
AUDIO.MultiChannel.pool[index]->sizeInFrames = sound.stream.buffer->sizeInFrames;
AUDIO.MultiChannel.pool[index]->data = sound.stream.buffer->data;

PlayAudioBuffer(AUDIO.MultiChannel.pool[index]);
}


void StopSoundMulti(void)
{
for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) StopAudioBuffer(AUDIO.MultiChannel.pool[i]);
}


int GetSoundsPlaying(void)
{
int counter = 0;

for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
{
if (IsAudioBufferPlaying(AUDIO.MultiChannel.pool[i])) counter++;
}

return counter;
}


void PauseSound(Sound sound)
{
PauseAudioBuffer(sound.stream.buffer);
}


void ResumeSound(Sound sound)
{
ResumeAudioBuffer(sound.stream.buffer);
}


void StopSound(Sound sound)
{
StopAudioBuffer(sound.stream.buffer);
}


bool IsSoundPlaying(Sound sound)
{
return IsAudioBufferPlaying(sound.stream.buffer);
}


void SetSoundVolume(Sound sound, float volume)
{
SetAudioBufferVolume(sound.stream.buffer, volume);
}


void SetSoundPitch(Sound sound, float pitch)
{
SetAudioBufferPitch(sound.stream.buffer, pitch);
}


void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
ma_format formatIn = ((wave->sampleSize == 8)? ma_format_u8 : ((wave->sampleSize == 16)? ma_format_s16 : ma_format_f32));
ma_format formatOut = (( sampleSize == 8)? ma_format_u8 : (( sampleSize == 16)? ma_format_s16 : ma_format_f32));

ma_uint32 frameCountIn = wave->sampleCount; 

ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, 0, formatOut, channels, sampleRate, NULL, frameCountIn, formatIn, wave->channels, wave->sampleRate);
if (frameCount == 0)
{
TRACELOG(LOG_WARNING, "WAVE: Failed to get frame count for format conversion");
return;
}

void *data = RL_MALLOC(frameCount*channels*(sampleSize/8));

frameCount = (ma_uint32)ma_convert_frames(data, frameCount, formatOut, channels, sampleRate, wave->data, frameCountIn, formatIn, wave->channels, wave->sampleRate);
if (frameCount == 0)
{
TRACELOG(LOG_WARNING, "WAVE: Failed format conversion");
return;
}

wave->sampleCount = frameCount;
wave->sampleSize = sampleSize;
wave->sampleRate = sampleRate;
wave->channels = channels;
RL_FREE(wave->data);
wave->data = data;
}


Wave WaveCopy(Wave wave)
{
Wave newWave = { 0 };

newWave.data = RL_MALLOC(wave.sampleCount*wave.sampleSize/8*wave.channels);

if (newWave.data != NULL)
{

memcpy(newWave.data, wave.data, wave.sampleCount*wave.channels*wave.sampleSize/8);

newWave.sampleCount = wave.sampleCount;
newWave.sampleRate = wave.sampleRate;
newWave.sampleSize = wave.sampleSize;
newWave.channels = wave.channels;
}

return newWave;
}



void WaveCrop(Wave *wave, int initSample, int finalSample)
{
if ((initSample >= 0) && (initSample < finalSample) &&
(finalSample > 0) && ((unsigned int)finalSample < wave->sampleCount))
{
int sampleCount = finalSample - initSample;

void *data = RL_MALLOC(sampleCount*wave->sampleSize/8*wave->channels);

memcpy(data, (unsigned char *)wave->data + (initSample*wave->channels*wave->sampleSize/8), sampleCount*wave->channels*wave->sampleSize/8);

RL_FREE(wave->data);
wave->data = data;
}
else TRACELOG(LOG_WARNING, "WAVE: Crop range out of bounds");
}



float *GetWaveData(Wave wave)
{
float *samples = (float *)RL_MALLOC(wave.sampleCount*wave.channels*sizeof(float));

for (unsigned int i = 0; i < wave.sampleCount; i++)
{
for (unsigned int j = 0; j < wave.channels; j++)
{
if (wave.sampleSize == 8) samples[wave.channels*i + j] = (float)(((unsigned char *)wave.data)[wave.channels*i + j] - 127)/256.0f;
else if (wave.sampleSize == 16) samples[wave.channels*i + j] = (float)((short *)wave.data)[wave.channels*i + j]/32767.0f;
else if (wave.sampleSize == 32) samples[wave.channels*i + j] = ((float *)wave.data)[wave.channels*i + j];
}
}

return samples;
}






Music LoadMusicStream(const char *fileName)
{
Music music = { 0 };
bool musicLoaded = false;

if (false) { }
#if defined(SUPPORT_FILEFORMAT_OGG)
else if (IsFileExtension(fileName, ".ogg"))
{

music.ctxData = stb_vorbis_open_filename(fileName, NULL, NULL);

if (music.ctxData != NULL)
{
music.ctxType = MUSIC_AUDIO_OGG;
stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData); 


music.stream = InitAudioStream(info.sample_rate, 16, info.channels);
music.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData)*info.channels;
music.loopCount = 0; 
musicLoaded = true;
}
}
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
else if (IsFileExtension(fileName, ".flac"))
{
music.ctxData = drflac_open_file(fileName);

if (music.ctxData != NULL)
{
music.ctxType = MUSIC_AUDIO_FLAC;
drflac *ctxFlac = (drflac *)music.ctxData;

music.stream = InitAudioStream(ctxFlac->sampleRate, ctxFlac->bitsPerSample, ctxFlac->channels);
music.sampleCount = (unsigned int)ctxFlac->totalSampleCount;
music.loopCount = 0; 
musicLoaded = true;
}
}
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
else if (IsFileExtension(fileName, ".mp3"))
{
drmp3 *ctxMp3 = RL_MALLOC(sizeof(drmp3));
music.ctxData = ctxMp3;

int result = drmp3_init_file(ctxMp3, fileName, NULL);

if (result > 0)
{
music.ctxType = MUSIC_AUDIO_MP3;

music.stream = InitAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
music.sampleCount = (unsigned int)drmp3_get_pcm_frame_count(ctxMp3)*ctxMp3->channels;
music.loopCount = 0; 
musicLoaded = true;
}
}
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
else if (IsFileExtension(fileName, ".xm"))
{
jar_xm_context_t *ctxXm = NULL;

int result = jar_xm_create_context_from_file(&ctxXm, 48000, fileName);

if (result == 0) 
{
music.ctxType = MUSIC_MODULE_XM;
jar_xm_set_max_loop_count(ctxXm, 0); 


music.stream = InitAudioStream(48000, 16, 2);
music.sampleCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm)*2;
music.loopCount = 0; 
jar_xm_reset(ctxXm); 
musicLoaded = true;

music.ctxData = ctxXm;
}
}
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
else if (IsFileExtension(fileName, ".mod"))
{
jar_mod_context_t *ctxMod = RL_MALLOC(sizeof(jar_mod_context_t));

jar_mod_init(ctxMod);
int result = jar_mod_load_file(ctxMod, fileName);

if (result > 0)
{
music.ctxType = MUSIC_MODULE_MOD;


music.stream = InitAudioStream(48000, 16, 2);
music.sampleCount = (unsigned int)jar_mod_max_samples(ctxMod)*2;
music.loopCount = 0; 
musicLoaded = true;

music.ctxData = ctxMod;
}
}
#endif

if (!musicLoaded)
{
if (false) { }
#if defined(SUPPORT_FILEFORMAT_OGG)
else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
#endif

TRACELOG(LOG_WARNING, "FILEIO: [%s] Music file could not be opened", fileName);
}
else
{

TRACELOG(LOG_INFO, "FILEIO: [%s] Music file successfully loaded:", fileName);
TRACELOG(LOG_INFO, " > Total samples: %i", music.sampleCount);
TRACELOG(LOG_INFO, " > Sample rate: %i Hz", music.stream.sampleRate);
TRACELOG(LOG_INFO, " > Sample size: %i bits", music.stream.sampleSize);
TRACELOG(LOG_INFO, " > Channels: %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
}

return music;
}


void UnloadMusicStream(Music music)
{
CloseAudioStream(music.stream);

if (false) { }
#if defined(SUPPORT_FILEFORMAT_OGG)
else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
#endif
}


void PlayMusicStream(Music music)
{
if (music.stream.buffer != NULL)
{




ma_uint32 frameCursorPos = music.stream.buffer->frameCursorPos;
PlayAudioStream(music.stream); 
music.stream.buffer->frameCursorPos = frameCursorPos;
}
}


void PauseMusicStream(Music music)
{
PauseAudioStream(music.stream);
}


void ResumeMusicStream(Music music)
{
ResumeAudioStream(music.stream);
}


void StopMusicStream(Music music)
{
StopAudioStream(music.stream);

switch (music.ctxType)
{
#if defined(SUPPORT_FILEFORMAT_OGG)
case MUSIC_AUDIO_OGG: stb_vorbis_seek_start((stb_vorbis *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
case MUSIC_AUDIO_FLAC: drflac_seek_to_pcm_frame((drflac *)music.ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
case MUSIC_AUDIO_MP3: drmp3_seek_to_pcm_frame((drmp3 *)music.ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
case MUSIC_MODULE_XM: jar_xm_reset((jar_xm_context_t *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
case MUSIC_MODULE_MOD: jar_mod_seek_start((jar_mod_context_t *)music.ctxData); break;
#endif
default: break;
}
}


void UpdateMusicStream(Music music)
{
if (music.stream.buffer == NULL) return;

bool streamEnding = false;

unsigned int subBufferSizeInFrames = music.stream.buffer->sizeInFrames/2;


void *pcm = RL_CALLOC(subBufferSizeInFrames*music.stream.channels*music.stream.sampleSize/8, 1);

int samplesCount = 0; 



int sampleLeft = music.sampleCount - (music.stream.buffer->totalFramesProcessed*music.stream.channels);

while (IsAudioStreamProcessed(music.stream))
{
if ((sampleLeft/music.stream.channels) >= subBufferSizeInFrames) samplesCount = subBufferSizeInFrames*music.stream.channels;
else samplesCount = sampleLeft;

switch (music.ctxType)
{
#if defined(SUPPORT_FILEFORMAT_OGG)
case MUSIC_AUDIO_OGG:
{

stb_vorbis_get_samples_short_interleaved((stb_vorbis *)music.ctxData, music.stream.channels, (short *)pcm, samplesCount);

} break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
case MUSIC_AUDIO_FLAC:
{

drflac_read_pcm_frames_s16((drflac *)music.ctxData, samplesCount, (short *)pcm);

} break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
case MUSIC_AUDIO_MP3:
{

drmp3_read_pcm_frames_f32((drmp3 *)music.ctxData, samplesCount/music.stream.channels, (float *)pcm);

} break;
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
case MUSIC_MODULE_XM:
{

jar_xm_generate_samples_16bit((jar_xm_context_t *)music.ctxData, (short *)pcm, samplesCount/2);
} break;
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
case MUSIC_MODULE_MOD:
{

jar_mod_fillbuffer((jar_mod_context_t *)music.ctxData, (short *)pcm, samplesCount/2, 0);
} break;
#endif
default: break;
}

UpdateAudioStream(music.stream, pcm, samplesCount);

if ((music.ctxType == MUSIC_MODULE_XM) || (music.ctxType == MUSIC_MODULE_MOD))
{
if (samplesCount > 1) sampleLeft -= samplesCount/2;
else sampleLeft -= samplesCount;
}
else sampleLeft -= samplesCount;

if (sampleLeft <= 0)
{
streamEnding = true;
break;
}
}


RL_FREE(pcm);


if (streamEnding)
{
StopMusicStream(music); 


if (music.loopCount > 1)
{
music.loopCount--; 
PlayMusicStream(music); 
}
else if (music.loopCount == 0) PlayMusicStream(music);
}
else
{


if (IsMusicPlaying(music)) PlayMusicStream(music);
}
}


bool IsMusicPlaying(Music music)
{
return IsAudioStreamPlaying(music.stream);
}


void SetMusicVolume(Music music, float volume)
{
SetAudioStreamVolume(music.stream, volume);
}


void SetMusicPitch(Music music, float pitch)
{
SetAudioStreamPitch(music.stream, pitch);
}



void SetMusicLoopCount(Music music, int count)
{
music.loopCount = count;
}


float GetMusicTimeLength(Music music)
{
float totalSeconds = 0.0f;

totalSeconds = (float)music.sampleCount/(music.stream.sampleRate*music.stream.channels);

return totalSeconds;
}


float GetMusicTimePlayed(Music music)
{
float secondsPlayed = 0.0f;

if (music.stream.buffer != NULL)
{

unsigned int samplesPlayed = music.stream.buffer->totalFramesProcessed*music.stream.channels;
secondsPlayed = (float)samplesPlayed / (music.stream.sampleRate*music.stream.channels);
}

return secondsPlayed;
}


AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
AudioStream stream = { 0 };

stream.sampleRate = sampleRate;
stream.sampleSize = sampleSize;
stream.channels = channels;

ma_format formatIn = ((stream.sampleSize == 8)? ma_format_u8 : ((stream.sampleSize == 16)? ma_format_s16 : ma_format_f32));


unsigned int periodSize = AUDIO.System.device.playback.internalPeriodSizeInFrames;
unsigned int subBufferSize = AUDIO.Buffer.defaultSize; 

if (subBufferSize < periodSize) subBufferSize = periodSize;


stream.buffer = LoadAudioBuffer(formatIn, stream.channels, stream.sampleRate, subBufferSize*2, AUDIO_BUFFER_USAGE_STREAM);

if (stream.buffer != NULL)
{
stream.buffer->looping = true; 
TRACELOG(LOG_INFO, "STREAM: Initialized successfully (%i Hz, %i bit, %s)", stream.sampleRate, stream.sampleSize, (stream.channels == 1)? "Mono" : "Stereo");
}
else TRACELOG(LOG_WARNING, "STREAM: Failed to load audio buffer, stream could not be created");

return stream;
}


void CloseAudioStream(AudioStream stream)
{
UnloadAudioBuffer(stream.buffer);

TRACELOG(LOG_INFO, "STREAM: Unloaded audio stream data from RAM");
}




void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount)
{
if (stream.buffer != NULL)
{
if (stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1])
{
ma_uint32 subBufferToUpdate = 0;

if (stream.buffer->isSubBufferProcessed[0] && stream.buffer->isSubBufferProcessed[1])
{


subBufferToUpdate = 0;
stream.buffer->frameCursorPos = 0;
}
else
{

subBufferToUpdate = (stream.buffer->isSubBufferProcessed[0])? 0 : 1;
}

ma_uint32 subBufferSizeInFrames = stream.buffer->sizeInFrames/2;
unsigned char *subBuffer = stream.buffer->data + ((subBufferSizeInFrames*stream.channels*(stream.sampleSize/8))*subBufferToUpdate);


stream.buffer->totalFramesProcessed += subBufferSizeInFrames;



if (subBufferSizeInFrames >= (ma_uint32)samplesCount/stream.channels)
{
ma_uint32 framesToWrite = subBufferSizeInFrames;

if (framesToWrite > ((ma_uint32)samplesCount/stream.channels)) framesToWrite = (ma_uint32)samplesCount/stream.channels;

ma_uint32 bytesToWrite = framesToWrite*stream.channels*(stream.sampleSize/8);
memcpy(subBuffer, data, bytesToWrite);


ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

if (leftoverFrameCount > 0) memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sampleSize/8));

stream.buffer->isSubBufferProcessed[subBufferToUpdate] = false;
}
else TRACELOG(LOG_WARNING, "STREAM: Attempting to write too many frames to buffer");
}
else TRACELOG(LOG_WARNING, "STREAM: Buffer not available for updating");
}
}


bool IsAudioStreamProcessed(AudioStream stream)
{
if (stream.buffer == NULL) return false;

return (stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1]);
}


void PlayAudioStream(AudioStream stream)
{
PlayAudioBuffer(stream.buffer);
}


void PauseAudioStream(AudioStream stream)
{
PauseAudioBuffer(stream.buffer);
}


void ResumeAudioStream(AudioStream stream)
{
ResumeAudioBuffer(stream.buffer);
}


bool IsAudioStreamPlaying(AudioStream stream)
{
return IsAudioBufferPlaying(stream.buffer);
}


void StopAudioStream(AudioStream stream)
{
StopAudioBuffer(stream.buffer);
}


void SetAudioStreamVolume(AudioStream stream, float volume)
{
SetAudioBufferVolume(stream.buffer, volume);
}


void SetAudioStreamPitch(AudioStream stream, float pitch)
{
SetAudioBufferPitch(stream.buffer, pitch);
}


void SetAudioStreamBufferSizeDefault(int size)
{
AUDIO.Buffer.defaultSize = size;
}






static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message)
{
(void)pContext;
(void)pDevice;

TRACELOG(LOG_ERROR, "miniaudio: %s", message); 
}


static ma_uint32 ReadAudioBufferFramesInInternalFormat(AudioBuffer *audioBuffer, void *framesOut, ma_uint32 frameCount)
{
ma_uint32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1)? audioBuffer->sizeInFrames/2 : audioBuffer->sizeInFrames;
ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos/subBufferSizeInFrames;

if (currentSubBufferIndex > 1) return 0;



bool isSubBufferProcessed[2];
isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn, audioBuffer->converter.config.channelsIn);


ma_uint32 framesRead = 0;
while (1)
{




if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC)
{
if (framesRead >= frameCount) break;
}
else
{
if (isSubBufferProcessed[currentSubBufferIndex]) break;
}

ma_uint32 totalFramesRemaining = (frameCount - framesRead);
if (totalFramesRemaining == 0) break;

ma_uint32 framesRemainingInOutputBuffer;
if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC)
{
framesRemainingInOutputBuffer = audioBuffer->sizeInFrames - audioBuffer->frameCursorPos;
}
else
{
ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames*currentSubBufferIndex;
framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrameIndexOfThisSubBuffer);
}

ma_uint32 framesToRead = totalFramesRemaining;
if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

memcpy((unsigned char *)framesOut + (framesRead*frameSizeInBytes), audioBuffer->data + (audioBuffer->frameCursorPos*frameSizeInBytes), framesToRead*frameSizeInBytes);
audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead)%audioBuffer->sizeInFrames;
framesRead += framesToRead;


if (framesToRead == framesRemainingInOutputBuffer)
{
audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
isSubBufferProcessed[currentSubBufferIndex] = true;

currentSubBufferIndex = (currentSubBufferIndex + 1)%2;


if (!audioBuffer->looping)
{
StopAudioBuffer(audioBuffer);
break;
}
}
}


ma_uint32 totalFramesRemaining = (frameCount - framesRead);
if (totalFramesRemaining > 0)
{
memset((unsigned char *)framesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);




if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
}

return framesRead;
}


static ma_uint32 ReadAudioBufferFramesInMixingFormat(AudioBuffer *audioBuffer, float *framesOut, ma_uint32 frameCount)
{




ma_uint8 inputBuffer[4096];
ma_uint32 inputBufferFrameCap = sizeof(inputBuffer) / ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn, audioBuffer->converter.config.channelsIn);

ma_uint32 totalOutputFramesProcessed = 0;
while (totalOutputFramesProcessed < frameCount)
{
ma_uint64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;

ma_uint64 inputFramesToProcessThisIteration = ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFramesToProcessThisIteration);
if (inputFramesToProcessThisIteration > inputBufferFrameCap)
{
inputFramesToProcessThisIteration = inputBufferFrameCap;
}

float *runningFramesOut = framesOut + (totalOutputFramesProcessed * audioBuffer->converter.config.channelsOut);


ma_uint64 inputFramesProcessedThisIteration = ReadAudioBufferFramesInInternalFormat(audioBuffer, inputBuffer, (ma_uint32)inputFramesToProcessThisIteration); 
ma_uint64 outputFramesProcessedThisIteration = outputFramesToProcessThisIteration;
ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessedThisIteration, runningFramesOut, &outputFramesProcessedThisIteration);

totalOutputFramesProcessed += (ma_uint32)outputFramesProcessedThisIteration; 

if (inputFramesProcessedThisIteration < inputFramesToProcessThisIteration)
{
break; 
}


if (inputFramesProcessedThisIteration == 0 && outputFramesProcessedThisIteration == 0)
{
break;
}
}

return totalOutputFramesProcessed;
}




static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount)
{
(void)pDevice;


memset(pFramesOut, 0, frameCount*pDevice->playback.channels*ma_get_bytes_per_sample(pDevice->playback.format));



ma_mutex_lock(&AUDIO.System.lock);
{
for (AudioBuffer *audioBuffer = AUDIO.Buffer.first; audioBuffer != NULL; audioBuffer = audioBuffer->next)
{

if (!audioBuffer->playing || audioBuffer->paused) continue;

ma_uint32 framesRead = 0;

while (1)
{
if (framesRead >= frameCount) break;


ma_uint32 framesToRead = (frameCount - framesRead);

while (framesToRead > 0)
{
float tempBuffer[1024]; 

ma_uint32 framesToReadRightNow = framesToRead;
if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS)
{
framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS;
}

ma_uint32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadRightNow);
if (framesJustRead > 0)
{
float *framesOut = (float *)pFramesOut + (framesRead*AUDIO.System.device.playback.channels);
float *framesIn = tempBuffer;

MixAudioFrames(framesOut, framesIn, framesJustRead, audioBuffer->volume);

framesToRead -= framesJustRead;
framesRead += framesJustRead;
}

if (!audioBuffer->playing)
{
framesRead = frameCount;
break;
}


if (framesJustRead < framesToReadRightNow)
{
if (!audioBuffer->looping)
{
StopAudioBuffer(audioBuffer);
break;
}
else
{


audioBuffer->frameCursorPos = 0;
continue;
}
}
}



if (framesToRead > 0) break;
}
}
}

ma_mutex_unlock(&AUDIO.System.lock);
}



static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume)
{
for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
{
for (ma_uint32 iChannel = 0; iChannel < AUDIO.System.device.playback.channels; ++iChannel)
{
float *frameOut = framesOut + (iFrame*AUDIO.System.device.playback.channels);
const float *frameIn = framesIn + (iFrame*AUDIO.System.device.playback.channels);

frameOut[iChannel] += (frameIn[iChannel]*localVolume);
}
}
}


static void InitAudioBufferPool(void)
{

for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
{
AUDIO.MultiChannel.pool[i] = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO_DEVICE_SAMPLE_RATE, 0, AUDIO_BUFFER_USAGE_STATIC);
}


TRACELOG(LOG_INFO, "AUDIO: Multichannel pool size: %i", MAX_AUDIO_BUFFER_POOL_CHANNELS);
}


static void CloseAudioBufferPool(void)
{
for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
{
RL_FREE(AUDIO.MultiChannel.pool[i]->data);
RL_FREE(AUDIO.MultiChannel.pool[i]);
}
}

#if defined(SUPPORT_FILEFORMAT_WAV)

static Wave LoadWAV(const char *fileName)
{

typedef struct {
char chunkID[4];
int chunkSize;
char format[4];
} WAVRiffHeader;

typedef struct {
char subChunkID[4];
int subChunkSize;
short audioFormat;
short numChannels;
int sampleRate;
int byteRate;
short blockAlign;
short bitsPerSample;
} WAVFormat;

typedef struct {
char subChunkID[4];
int subChunkSize;
} WAVData;

WAVRiffHeader wavRiffHeader = { 0 };
WAVFormat wavFormat = { 0 };
WAVData wavData = { 0 };

Wave wave = { 0 };
FILE *wavFile = NULL;

wavFile = fopen(fileName, "rb");

if (wavFile == NULL)
{
TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open WAV file", fileName);
wave.data = NULL;
}
else
{

fread(&wavRiffHeader, sizeof(WAVRiffHeader), 1, wavFile);


if ((wavRiffHeader.chunkID[0] != 'R') ||
(wavRiffHeader.chunkID[1] != 'I') ||
(wavRiffHeader.chunkID[2] != 'F') ||
(wavRiffHeader.chunkID[3] != 'F') ||
(wavRiffHeader.format[0] != 'W') ||
(wavRiffHeader.format[1] != 'A') ||
(wavRiffHeader.format[2] != 'V') ||
(wavRiffHeader.format[3] != 'E'))
{
TRACELOG(LOG_WARNING, "WAVE: [%s] RIFF or WAVE header are not valid", fileName);
}
else
{

fread(&wavFormat, sizeof(WAVFormat), 1, wavFile);


if ((wavFormat.subChunkID[0] != 'f') || (wavFormat.subChunkID[1] != 'm') ||
(wavFormat.subChunkID[2] != 't') || (wavFormat.subChunkID[3] != ' '))
{
TRACELOG(LOG_WARNING, "WAVE: [%s] Wave format header is not valid", fileName);
}
else
{

if (wavFormat.subChunkSize > 16) fseek(wavFile, sizeof(short), SEEK_CUR);


fread(&wavData, sizeof(WAVData), 1, wavFile);


if ((wavData.subChunkID[0] != 'd') || (wavData.subChunkID[1] != 'a') ||
(wavData.subChunkID[2] != 't') || (wavData.subChunkID[3] != 'a'))
{
TRACELOG(LOG_WARNING, "WAVE: [%s] Data header is not valid", fileName);
}
else
{

wave.data = RL_MALLOC(wavData.subChunkSize);


fread(wave.data, wavData.subChunkSize, 1, wavFile);


wave.sampleRate = wavFormat.sampleRate;
wave.sampleSize = wavFormat.bitsPerSample;
wave.channels = wavFormat.numChannels;


if ((wave.sampleSize != 8) && (wave.sampleSize != 16) && (wave.sampleSize != 32))
{
TRACELOG(LOG_WARNING, "WAVE: [%s] Sample size (%ibit) not supported, converted to 16bit", fileName, wave.sampleSize);
WaveFormat(&wave, wave.sampleRate, 16, wave.channels);
}


if (wave.channels > 2)
{
WaveFormat(&wave, wave.sampleRate, wave.sampleSize, 2);
TRACELOG(LOG_WARNING, "WAVE: [%s] Channels number (%i) not supported, converted to 2 channels", fileName, wave.channels);
}


wave.sampleCount = (wavData.subChunkSize/(wave.sampleSize/8))/wave.channels;

TRACELOG(LOG_INFO, "WAVE: [%s] File loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
}
}
}

fclose(wavFile);
}

return wave;
}


static int SaveWAV(Wave wave, const char *fileName)
{
int success = 0;
int dataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;


typedef struct {
char chunkID[4];
int chunkSize;
char format[4];
} RiffHeader;

typedef struct {
char subChunkID[4];
int subChunkSize;
short audioFormat;
short numChannels;
int sampleRate;
int byteRate;
short blockAlign;
short bitsPerSample;
} WaveFormat;

typedef struct {
char subChunkID[4];
int subChunkSize;
} WaveData;

FILE *wavFile = fopen(fileName, "wb");

if (wavFile == NULL) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open audio file", fileName);
else
{
RiffHeader riffHeader;
WaveFormat waveFormat;
WaveData waveData;


riffHeader.chunkID[0] = 'R';
riffHeader.chunkID[1] = 'I';
riffHeader.chunkID[2] = 'F';
riffHeader.chunkID[3] = 'F';
riffHeader.chunkSize = 44 - 4 + wave.sampleCount*wave.sampleSize/8;
riffHeader.format[0] = 'W';
riffHeader.format[1] = 'A';
riffHeader.format[2] = 'V';
riffHeader.format[3] = 'E';

waveFormat.subChunkID[0] = 'f';
waveFormat.subChunkID[1] = 'm';
waveFormat.subChunkID[2] = 't';
waveFormat.subChunkID[3] = ' ';
waveFormat.subChunkSize = 16;
waveFormat.audioFormat = 1;
waveFormat.numChannels = wave.channels;
waveFormat.sampleRate = wave.sampleRate;
waveFormat.byteRate = wave.sampleRate*wave.sampleSize/8;
waveFormat.blockAlign = wave.sampleSize/8;
waveFormat.bitsPerSample = wave.sampleSize;

waveData.subChunkID[0] = 'd';
waveData.subChunkID[1] = 'a';
waveData.subChunkID[2] = 't';
waveData.subChunkID[3] = 'a';
waveData.subChunkSize = dataSize;

fwrite(&riffHeader, sizeof(RiffHeader), 1, wavFile);
fwrite(&waveFormat, sizeof(WaveFormat), 1, wavFile);
fwrite(&waveData, sizeof(WaveData), 1, wavFile);

success = fwrite(wave.data, dataSize, 1, wavFile);

fclose(wavFile);
}


return success;
}
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)


static Wave LoadOGG(const char *fileName)
{
Wave wave = { 0 };

stb_vorbis *oggFile = stb_vorbis_open_filename(fileName, NULL, NULL);

if (oggFile == NULL) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open OGG file", fileName);
else
{
stb_vorbis_info info = stb_vorbis_get_info(oggFile);

wave.sampleRate = info.sample_rate;
wave.sampleSize = 16; 
wave.channels = info.channels;
wave.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples(oggFile)*info.channels; 

float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);
if (totalSeconds > 10) TRACELOG(LOG_WARNING, "WAVE: [%s] Ogg audio length larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

wave.data = (short *)RL_MALLOC(wave.sampleCount*wave.channels*sizeof(short));


stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *)wave.data, wave.sampleCount*wave.channels);
TRACELOG(LOG_INFO, "WAVE: [%s] OGG file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");

stb_vorbis_close(oggFile);
}

return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)


static Wave LoadFLAC(const char *fileName)
{
Wave wave = { 0 };


unsigned long long int totalSampleCount = 0;
wave.data = drflac_open_file_and_read_pcm_frames_s16(fileName, &wave.channels, &wave.sampleRate, &totalSampleCount);

if (wave.data == NULL) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load FLAC data", fileName);
else
{
wave.sampleCount = (unsigned int)totalSampleCount;
wave.sampleSize = 16;


if (wave.channels > 2) TRACELOG(LOG_WARNING, "WAVE: [%s] FLAC channels number (%i) not supported", fileName, wave.channels);

TRACELOG(LOG_INFO, "WAVE: [%s] FLAC file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
}

return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)


static Wave LoadMP3(const char *fileName)
{
Wave wave = { 0 };


unsigned long long int totalFrameCount = 0;
drmp3_config config = { 0 };
wave.data = drmp3_open_file_and_read_f32(fileName, &config, &totalFrameCount);

if (wave.data == NULL) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load MP3 data", fileName);
else
{
wave.channels = config.outputChannels;
wave.sampleRate = config.outputSampleRate;
wave.sampleCount = (int)totalFrameCount*wave.channels;
wave.sampleSize = 32;


if (wave.channels > 2) TRACELOG(LOG_WARNING, "WAVE: [%s] MP3 channels number (%i) not supported", fileName, wave.channels);

TRACELOG(LOG_INFO, "WAVE: [%s] MP3 file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
}

return wave;
}
#endif


#if defined(RAUDIO_STANDALONE)

bool IsFileExtension(const char *fileName, const char *ext)
{
bool result = false;
const char *fileExt;

if ((fileExt = strrchr(fileName, '.')) != NULL)
{
if (strcmp(fileExt, ext) == 0) result = true;
}

return result;
}
#endif

#undef AudioBuffer
