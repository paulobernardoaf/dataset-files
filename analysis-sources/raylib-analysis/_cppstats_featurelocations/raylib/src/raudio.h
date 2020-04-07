



















































#if !defined(RAUDIO_H)
#define RAUDIO_H





#if !defined(RL_MALLOC)
#define RL_MALLOC(sz) malloc(sz)
#endif
#if !defined(RL_CALLOC)
#define RL_CALLOC(n,sz) calloc(n,sz)
#endif
#if !defined(RL_FREE)
#define RL_FREE(p) free(p)
#endif




#if !defined(__cplusplus)

#if !defined(_STDBOOL_H)
typedef enum { false, true } bool;
#define _STDBOOL_H
#endif
#endif


typedef struct Wave {
unsigned int sampleCount; 
unsigned int sampleRate; 
unsigned int sampleSize; 
unsigned int channels; 
void *data; 
} Wave;

typedef struct rAudioBuffer rAudioBuffer;



typedef struct AudioStream {
unsigned int sampleRate; 
unsigned int sampleSize; 
unsigned int channels; 

rAudioBuffer *buffer; 
} AudioStream;


typedef struct Sound {
unsigned int sampleCount; 
AudioStream stream; 
} Sound;



typedef struct Music {
int ctxType; 
void *ctxData; 

unsigned int sampleCount; 
unsigned int loopCount; 

AudioStream stream; 
} Music;

#if defined(__cplusplus)
extern "C" { 
#endif











void InitAudioDevice(void); 
void CloseAudioDevice(void); 
bool IsAudioDeviceReady(void); 
void SetMasterVolume(float volume); 


Wave LoadWave(const char *fileName); 
Sound LoadSound(const char *fileName); 
Sound LoadSoundFromWave(Wave wave); 
void UpdateSound(Sound sound, const void *data, int samplesCount);
void UnloadWave(Wave wave); 
void UnloadSound(Sound sound); 
void ExportWave(Wave wave, const char *fileName); 
void ExportWaveAsCode(Wave wave, const char *fileName); 


void PlaySound(Sound sound); 
void StopSound(Sound sound); 
void PauseSound(Sound sound); 
void ResumeSound(Sound sound); 
void PlaySoundMulti(Sound sound); 
void StopSoundMulti(void); 
int GetSoundsPlaying(void); 
bool IsSoundPlaying(Sound sound); 
void SetSoundVolume(Sound sound, float volume); 
void SetSoundPitch(Sound sound, float pitch); 
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels); 
Wave WaveCopy(Wave wave); 
void WaveCrop(Wave *wave, int initSample, int finalSample); 
float *GetWaveData(Wave wave); 


Music LoadMusicStream(const char *fileName); 
void UnloadMusicStream(Music music); 
void PlayMusicStream(Music music); 
void UpdateMusicStream(Music music); 
void StopMusicStream(Music music); 
void PauseMusicStream(Music music); 
void ResumeMusicStream(Music music); 
bool IsMusicPlaying(Music music); 
void SetMusicVolume(Music music, float volume); 
void SetMusicPitch(Music music, float pitch); 
void SetMusicLoopCount(Music music, int count); 
float GetMusicTimeLength(Music music); 
float GetMusicTimePlayed(Music music); 


AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); 
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount); 
void CloseAudioStream(AudioStream stream); 
bool IsAudioStreamProcessed(AudioStream stream); 
void PlayAudioStream(AudioStream stream); 
void PauseAudioStream(AudioStream stream); 
void ResumeAudioStream(AudioStream stream); 
bool IsAudioStreamPlaying(AudioStream stream); 
void StopAudioStream(AudioStream stream); 
void SetAudioStreamVolume(AudioStream stream, float volume); 
void SetAudioStreamPitch(AudioStream stream, float pitch); 
void SetAudioStreamBufferSizeDefault(int size); 

#if defined(__cplusplus)
}
#endif

#endif 
