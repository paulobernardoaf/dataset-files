


























#if !defined(BMD_DECKLINKAPI_v9_9_H)
#define BMD_DECKLINKAPI_v9_9_H

#include "DeckLinkAPI.h"



BMD_CONST REFIID IID_IDeckLinkOutput_v9_9 = {0xA3,0xEF,0x09,0x63,0x08,0x62,0x44,0xED,0x92,0xA9,0xEE,0x89,0xAB,0xF4,0x31,0xC7};


#if defined(__cplusplus)


class IDeckLinkOutput_v9_9;



class IDeckLinkOutput_v9_9 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoOutputFlags flags, BMDDisplayModeSupport *result, IDeckLinkDisplayMode **resultDisplayMode) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



virtual HRESULT EnableVideoOutput ( BMDDisplayMode displayMode, BMDVideoOutputFlags flags) = 0;
virtual HRESULT DisableVideoOutput (void) = 0;

virtual HRESULT SetVideoOutputFrameMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;
virtual HRESULT CreateVideoFrame ( int32_t width, int32_t height, int32_t rowBytes, BMDPixelFormat pixelFormat, BMDFrameFlags flags, IDeckLinkMutableVideoFrame **outFrame) = 0;
virtual HRESULT CreateAncillaryData ( BMDPixelFormat pixelFormat, IDeckLinkVideoFrameAncillary **outBuffer) = 0;

virtual HRESULT DisplayVideoFrameSync ( IDeckLinkVideoFrame *theFrame) = 0;
virtual HRESULT ScheduleVideoFrame ( IDeckLinkVideoFrame *theFrame, BMDTimeValue displayTime, BMDTimeValue displayDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT SetScheduledFrameCompletionCallback ( IDeckLinkVideoOutputCallback *theCallback) = 0;
virtual HRESULT GetBufferedVideoFrameCount ( uint32_t *bufferedFrameCount) = 0;



virtual HRESULT EnableAudioOutput ( BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount, BMDAudioOutputStreamType streamType) = 0;
virtual HRESULT DisableAudioOutput (void) = 0;

virtual HRESULT WriteAudioSamplesSync ( void *buffer, uint32_t sampleFrameCount, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT BeginAudioPreroll (void) = 0;
virtual HRESULT EndAudioPreroll (void) = 0;
virtual HRESULT ScheduleAudioSamples ( void *buffer, uint32_t sampleFrameCount, BMDTimeValue streamTime, BMDTimeScale timeScale, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT GetBufferedAudioSampleFrameCount ( uint32_t *bufferedSampleFrameCount) = 0;
virtual HRESULT FlushBufferedAudioSamples (void) = 0;

virtual HRESULT SetAudioCallback ( IDeckLinkAudioOutputCallback *theCallback) = 0;



virtual HRESULT StartScheduledPlayback ( BMDTimeValue playbackStartTime, BMDTimeScale timeScale, double playbackSpeed) = 0;
virtual HRESULT StopScheduledPlayback ( BMDTimeValue stopPlaybackAtTime, BMDTimeValue *actualStopTime, BMDTimeScale timeScale) = 0;
virtual HRESULT IsScheduledPlaybackRunning ( bool *active) = 0;
virtual HRESULT GetScheduledStreamTime ( BMDTimeScale desiredTimeScale, BMDTimeValue *streamTime, double *playbackSpeed) = 0;
virtual HRESULT GetReferenceStatus ( BMDReferenceStatus *referenceStatus) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkOutput_v9_9 () {}; 
};

#endif 
#endif 

