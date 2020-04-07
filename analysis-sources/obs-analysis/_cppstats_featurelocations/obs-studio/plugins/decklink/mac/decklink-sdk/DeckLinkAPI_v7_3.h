




























#if !defined(__DeckLink_API_v7_3_h__)
#define __DeckLink_API_v7_3_h__

#include "DeckLinkAPI.h"
#include "DeckLinkAPI_v7_6.h"



#define IID_IDeckLinkInputCallback_v7_3 (REFIID){0xFD,0x6F,0x31,0x1D,0x4D,0x00,0x44,0x4B,0x9E,0xD4,0x1F,0x25,0xB5,0x73,0x0A,0xD0}
#define IID_IDeckLinkOutput_v7_3 (REFIID){0x27,0x1C,0x65,0xE3,0xC3,0x23,0x43,0x44,0xA3,0x0F,0xD9,0x08,0xBC,0xB2,0x0A,0xA3}
#define IID_IDeckLinkInput_v7_3 (REFIID){0x49,0x73,0xF0,0x12,0x99,0x25,0x45,0x8C,0x87,0x1C,0x18,0x77,0x4C,0xDB,0xBE,0xCB}
#define IID_IDeckLinkVideoInputFrame_v7_3 (REFIID){0xCF,0x31,0x77,0x90,0x28,0x94,0x11,0xDE,0x8C,0x30,0x08,0x00,0x20,0x0C,0x9A,0x66}



#if defined(__cplusplus)



class IDeckLinkVideoInputFrame_v7_3;






class IDeckLinkOutput_v7_3 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode (BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDDisplayModeSupport *result) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator_v7_6 **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



virtual HRESULT EnableVideoOutput (BMDDisplayMode displayMode, BMDVideoOutputFlags flags) = 0;
virtual HRESULT DisableVideoOutput (void) = 0;

virtual HRESULT SetVideoOutputFrameMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;
virtual HRESULT CreateVideoFrame (int32_t width, int32_t height, int32_t rowBytes, BMDPixelFormat pixelFormat, BMDFrameFlags flags, IDeckLinkMutableVideoFrame_v7_6 **outFrame) = 0;
virtual HRESULT CreateAncillaryData (BMDPixelFormat pixelFormat, IDeckLinkVideoFrameAncillary **outBuffer) = 0;

virtual HRESULT DisplayVideoFrameSync ( IDeckLinkVideoFrame_v7_6 *theFrame) = 0;
virtual HRESULT ScheduleVideoFrame ( IDeckLinkVideoFrame_v7_6 *theFrame, BMDTimeValue displayTime, BMDTimeValue displayDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT SetScheduledFrameCompletionCallback ( IDeckLinkVideoOutputCallback *theCallback) = 0;
virtual HRESULT GetBufferedVideoFrameCount ( uint32_t *bufferedFrameCount) = 0;



virtual HRESULT EnableAudioOutput (BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount, BMDAudioOutputStreamType streamType) = 0;
virtual HRESULT DisableAudioOutput (void) = 0;

virtual HRESULT WriteAudioSamplesSync ( void *buffer, uint32_t sampleFrameCount, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT BeginAudioPreroll (void) = 0;
virtual HRESULT EndAudioPreroll (void) = 0;
virtual HRESULT ScheduleAudioSamples ( void *buffer, uint32_t sampleFrameCount, BMDTimeValue streamTime, BMDTimeScale timeScale, uint32_t *sampleFramesWritten) = 0;

virtual HRESULT GetBufferedAudioSampleFrameCount ( uint32_t *bufferedSampleFrameCount) = 0;
virtual HRESULT FlushBufferedAudioSamples (void) = 0;

virtual HRESULT SetAudioCallback ( IDeckLinkAudioOutputCallback *theCallback) = 0;



virtual HRESULT StartScheduledPlayback (BMDTimeValue playbackStartTime, BMDTimeScale timeScale, double playbackSpeed) = 0;
virtual HRESULT StopScheduledPlayback (BMDTimeValue stopPlaybackAtTime, BMDTimeValue *actualStopTime, BMDTimeScale timeScale) = 0;
virtual HRESULT IsScheduledPlaybackRunning ( bool *active) = 0;
virtual HRESULT GetHardwareReferenceClock (BMDTimeScale desiredTimeScale, BMDTimeValue *elapsedTimeSinceSchedulerBegan) = 0;

protected:
virtual ~IDeckLinkOutput_v7_3 () {}; 
};






class IDeckLinkInputCallback_v7_3 : public IUnknown
{
public:
virtual HRESULT VideoInputFormatChanged ( BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode_v7_6 *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) = 0;
virtual HRESULT VideoInputFrameArrived ( IDeckLinkVideoInputFrame_v7_3 *videoFrame, IDeckLinkAudioInputPacket *audioPacket) = 0;

protected:
virtual ~IDeckLinkInputCallback_v7_3 () {}; 
};






class IDeckLinkInput_v7_3 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode (BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDDisplayModeSupport *result) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator_v7_6 **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



virtual HRESULT EnableVideoInput (BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags) = 0;
virtual HRESULT DisableVideoInput (void) = 0;
virtual HRESULT GetAvailableVideoFrameCount ( uint32_t *availableFrameCount) = 0;



virtual HRESULT EnableAudioInput (BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount) = 0;
virtual HRESULT DisableAudioInput (void) = 0;
virtual HRESULT GetAvailableAudioSampleFrameCount ( uint32_t *availableSampleFrameCount) = 0;



virtual HRESULT StartStreams (void) = 0;
virtual HRESULT StopStreams (void) = 0;
virtual HRESULT PauseStreams (void) = 0;
virtual HRESULT FlushStreams (void) = 0;
virtual HRESULT SetCallback ( IDeckLinkInputCallback_v7_3 *theCallback) = 0;

protected:
virtual ~IDeckLinkInput_v7_3 () {}; 
};





class IDeckLinkVideoInputFrame_v7_3 : public IDeckLinkVideoFrame_v7_6
{
public:
virtual HRESULT GetStreamTime ( BMDTimeValue *frameTime, BMDTimeValue *frameDuration, BMDTimeScale timeScale) = 0;

protected:
virtual ~IDeckLinkVideoInputFrame_v7_3 () {}; 
};



#endif 
#endif 
