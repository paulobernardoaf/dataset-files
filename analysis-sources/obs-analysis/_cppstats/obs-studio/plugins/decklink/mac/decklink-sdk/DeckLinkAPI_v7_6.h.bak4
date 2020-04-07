




























#if !defined(__DeckLink_API_v7_6_h__)
#define __DeckLink_API_v7_6_h__

#include "DeckLinkAPI.h"



#define IID_IDeckLinkVideoOutputCallback_v7_6 (REFIID){0xE7,0x63,0xA6,0x26,0x4A,0x3C,0x49,0xD1,0xBF,0x13,0xE7,0xAD,0x36,0x92,0xAE,0x52}
#define IID_IDeckLinkInputCallback_v7_6 (REFIID){0x31,0xD2,0x8E,0xE7,0x88,0xB6,0x4C,0xB1,0x89,0x7A,0xCD,0xBF,0x79,0xA2,0x64,0x14}
#define IID_IDeckLinkDisplayModeIterator_v7_6 (REFIID){0x45,0x5D,0x74,0x1F,0x17,0x79,0x48,0x00,0x86,0xF5,0x0B,0x5D,0x13,0xD7,0x97,0x51}
#define IID_IDeckLinkDisplayMode_v7_6 (REFIID){0x87,0x45,0x1E,0x84,0x2B,0x7E,0x43,0x9E,0xA6,0x29,0x43,0x93,0xEA,0x4A,0x85,0x50}
#define IID_IDeckLinkOutput_v7_6 (REFIID){0x29,0x22,0x81,0x42,0xEB,0x8C,0x41,0x41,0xA6,0x21,0xF7,0x40,0x26,0x45,0x09,0x55}
#define IID_IDeckLinkInput_v7_6 (REFIID){0x30,0x0C,0x13,0x5A,0x9F,0x43,0x48,0xE2,0x99,0x06,0x6D,0x79,0x11,0xD9,0x3C,0xF1}
#define IID_IDeckLinkTimecode_v7_6 (REFIID){0xEF,0xB9,0xBC,0xA6,0xA5,0x21,0x44,0xF7,0xBD,0x69,0x23,0x32,0xF2,0x4D,0x9E,0xE6}
#define IID_IDeckLinkVideoFrame_v7_6 (REFIID){0xA8,0xD8,0x23,0x8E,0x6B,0x18,0x41,0x96,0x99,0xE1,0x5A,0xF7,0x17,0xB8,0x3D,0x32}
#define IID_IDeckLinkMutableVideoFrame_v7_6 (REFIID){0x46,0xFC,0xEE,0x00,0xB4,0xE6,0x43,0xD0,0x91,0xC0,0x02,0x3A,0x7F,0xCE,0xB3,0x4F}
#define IID_IDeckLinkVideoInputFrame_v7_6 (REFIID){0x9A,0x74,0xFA,0x41,0xAE,0x9F,0x47,0xAC,0x8C,0xF4,0x01,0xF4,0x2D,0xD5,0x99,0x65}
#define IID_IDeckLinkScreenPreviewCallback_v7_6 (REFIID){0x37,0x3F,0x49,0x9D,0x4B,0x4D,0x45,0x18,0xAD,0x22,0x63,0x54,0xE5,0xA5,0x82,0x5E}
#define IID_IDeckLinkCocoaScreenPreviewCallback_v7_6 (REFIID){0xD1,0x74,0x15,0x2F,0x8F,0x96,0x4C,0x07,0x83,0xA5,0xDD,0x5F,0x5A,0xF0,0xA2,0xAA}
#define IID_IDeckLinkGLScreenPreviewHelper_v7_6 (REFIID){0xBA,0x57,0x5C,0xD9,0xA1,0x5E,0x49,0x7B,0xB2,0xC2,0xF9,0xAF,0xE7,0xBE,0x4E,0xBA}
#define IID_IDeckLinkVideoConversion_v7_6 (REFIID){0x3E,0xB5,0x04,0xC9,0xF9,0x7D,0x40,0xFE,0xA1,0x58,0xD4,0x07,0xD4,0x8C,0xB5,0x3B}
#define IID_IDeckLinkConfiguration_v7_6 (REFIID){0xB8,0xEA,0xD5,0x69,0xB7,0x64,0x47,0xF0,0xA7,0x3F,0xAE,0x40,0xDF,0x6C,0xBF,0x10}


#if defined(__cplusplus)



typedef uint32_t BMDVideoConnection_v7_6;
enum _BMDVideoConnection_v7_6 {
bmdVideoConnectionSDI_v7_6 = 'sdi ',
bmdVideoConnectionHDMI_v7_6 = 'hdmi',
bmdVideoConnectionOpticalSDI_v7_6 = 'opti',
bmdVideoConnectionComponent_v7_6 = 'cpnt',
bmdVideoConnectionComposite_v7_6 = 'cmst',
bmdVideoConnectionSVideo_v7_6 = 'svid'
};




class IDeckLinkVideoOutputCallback_v7_6;
class IDeckLinkInputCallback_v7_6;
class IDeckLinkDisplayModeIterator_v7_6;
class IDeckLinkDisplayMode_v7_6;
class IDeckLinkOutput_v7_6;
class IDeckLinkInput_v7_6;
class IDeckLinkTimecode_v7_6;
class IDeckLinkVideoFrame_v7_6;
class IDeckLinkMutableVideoFrame_v7_6;
class IDeckLinkVideoInputFrame_v7_6;
class IDeckLinkScreenPreviewCallback_v7_6;
class IDeckLinkCocoaScreenPreviewCallback_v7_6;
class IDeckLinkGLScreenPreviewHelper_v7_6;
class IDeckLinkVideoConversion_v7_6;




class IDeckLinkVideoOutputCallback_v7_6 : public IUnknown
{
public:
virtual HRESULT ScheduledFrameCompleted ( IDeckLinkVideoFrame_v7_6 *completedFrame, BMDOutputFrameCompletionResult result) = 0;
virtual HRESULT ScheduledPlaybackHasStopped (void) = 0;

protected:
virtual ~IDeckLinkVideoOutputCallback_v7_6 () {}; 
};




class IDeckLinkInputCallback_v7_6 : public IUnknown
{
public:
virtual HRESULT VideoInputFormatChanged ( BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode_v7_6 *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) = 0;
virtual HRESULT VideoInputFrameArrived ( IDeckLinkVideoInputFrame_v7_6* videoFrame, IDeckLinkAudioInputPacket* audioPacket) = 0;

protected:
virtual ~IDeckLinkInputCallback_v7_6 () {}; 
};




class IDeckLinkDisplayModeIterator_v7_6 : public IUnknown
{
public:
virtual HRESULT Next ( IDeckLinkDisplayMode_v7_6 **deckLinkDisplayMode) = 0;

protected:
virtual ~IDeckLinkDisplayModeIterator_v7_6 () {}; 
};




class IDeckLinkDisplayMode_v7_6 : public IUnknown
{
public:
virtual HRESULT GetName ( CFStringRef *name) = 0;
virtual BMDDisplayMode GetDisplayMode (void) = 0;
virtual long GetWidth (void) = 0;
virtual long GetHeight (void) = 0;
virtual HRESULT GetFrameRate ( BMDTimeValue *frameDuration, BMDTimeScale *timeScale) = 0;
virtual BMDFieldDominance GetFieldDominance (void) = 0;

protected:
virtual ~IDeckLinkDisplayMode_v7_6 () {}; 
};




class IDeckLinkOutput_v7_6 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDDisplayModeSupport *result) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator_v7_6 **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback_v7_6 *previewCallback) = 0;



virtual HRESULT EnableVideoOutput ( BMDDisplayMode displayMode, BMDVideoOutputFlags flags) = 0;
virtual HRESULT DisableVideoOutput (void) = 0;

virtual HRESULT SetVideoOutputFrameMemoryAllocator ( IDeckLinkMemoryAllocator *theAllocator) = 0;
virtual HRESULT CreateVideoFrame ( int32_t width, int32_t height, int32_t rowBytes, BMDPixelFormat pixelFormat, BMDFrameFlags flags, IDeckLinkMutableVideoFrame_v7_6 **outFrame) = 0;
virtual HRESULT CreateAncillaryData ( BMDPixelFormat pixelFormat, IDeckLinkVideoFrameAncillary **outBuffer) = 0;

virtual HRESULT DisplayVideoFrameSync ( IDeckLinkVideoFrame_v7_6 *theFrame) = 0;
virtual HRESULT ScheduleVideoFrame ( IDeckLinkVideoFrame_v7_6 *theFrame, BMDTimeValue displayTime, BMDTimeValue displayDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT SetScheduledFrameCompletionCallback ( IDeckLinkVideoOutputCallback_v7_6 *theCallback) = 0;
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



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkOutput_v7_6 () {}; 
};




class IDeckLinkInput_v7_6 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDDisplayModeSupport *result) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator_v7_6 **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback_v7_6 *previewCallback) = 0;



virtual HRESULT EnableVideoInput ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags) = 0;
virtual HRESULT DisableVideoInput (void) = 0;
virtual HRESULT GetAvailableVideoFrameCount ( uint32_t *availableFrameCount) = 0;



virtual HRESULT EnableAudioInput ( BMDAudioSampleRate sampleRate, BMDAudioSampleType sampleType, uint32_t channelCount) = 0;
virtual HRESULT DisableAudioInput (void) = 0;
virtual HRESULT GetAvailableAudioSampleFrameCount ( uint32_t *availableSampleFrameCount) = 0;



virtual HRESULT StartStreams (void) = 0;
virtual HRESULT StopStreams (void) = 0;
virtual HRESULT PauseStreams (void) = 0;
virtual HRESULT FlushStreams (void) = 0;
virtual HRESULT SetCallback ( IDeckLinkInputCallback_v7_6 *theCallback) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkInput_v7_6 () {}; 
};




class IDeckLinkTimecode_v7_6 : public IUnknown
{
public:
virtual BMDTimecodeBCD GetBCD (void) = 0;
virtual HRESULT GetComponents ( uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *frames) = 0;
virtual HRESULT GetString ( CFStringRef *timecode) = 0;
virtual BMDTimecodeFlags GetFlags (void) = 0;

protected:
virtual ~IDeckLinkTimecode_v7_6 () {}; 
};




class IDeckLinkVideoFrame_v7_6 : public IUnknown
{
public:
virtual long GetWidth (void) = 0;
virtual long GetHeight (void) = 0;
virtual long GetRowBytes (void) = 0;
virtual BMDPixelFormat GetPixelFormat (void) = 0;
virtual BMDFrameFlags GetFlags (void) = 0;
virtual HRESULT GetBytes ( void **buffer) = 0;

virtual HRESULT GetTimecode (BMDTimecodeFormat format, IDeckLinkTimecode_v7_6 **timecode) = 0;
virtual HRESULT GetAncillaryData ( IDeckLinkVideoFrameAncillary **ancillary) = 0;

protected:
virtual ~IDeckLinkVideoFrame_v7_6 () {}; 
};




class IDeckLinkMutableVideoFrame_v7_6 : public IDeckLinkVideoFrame_v7_6
{
public:
virtual HRESULT SetFlags (BMDFrameFlags newFlags) = 0;

virtual HRESULT SetTimecode (BMDTimecodeFormat format, IDeckLinkTimecode_v7_6 *timecode) = 0;
virtual HRESULT SetTimecodeFromComponents (BMDTimecodeFormat format, uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t frames, BMDTimecodeFlags flags) = 0;
virtual HRESULT SetAncillaryData ( IDeckLinkVideoFrameAncillary *ancillary) = 0;

protected:
virtual ~IDeckLinkMutableVideoFrame_v7_6 () {}; 
};




class IDeckLinkVideoInputFrame_v7_6 : public IDeckLinkVideoFrame_v7_6
{
public:
virtual HRESULT GetStreamTime ( BMDTimeValue *frameTime, BMDTimeValue *frameDuration, BMDTimeScale timeScale) = 0;
virtual HRESULT GetHardwareReferenceTimestamp (BMDTimeScale timeScale, BMDTimeValue *frameTime, BMDTimeValue *frameDuration) = 0;

protected:
virtual ~IDeckLinkVideoInputFrame_v7_6 () {}; 
};




class IDeckLinkScreenPreviewCallback_v7_6 : public IUnknown
{
public:
virtual HRESULT DrawFrame ( IDeckLinkVideoFrame_v7_6 *theFrame) = 0;

protected:
virtual ~IDeckLinkScreenPreviewCallback_v7_6 () {}; 
};




class IDeckLinkCocoaScreenPreviewCallback_v7_6 : public IDeckLinkScreenPreviewCallback_v7_6
{
public:

protected:
virtual ~IDeckLinkCocoaScreenPreviewCallback_v7_6 () {}; 
};




class IDeckLinkGLScreenPreviewHelper_v7_6 : public IUnknown
{
public:



virtual HRESULT InitializeGL (void) = 0;
virtual HRESULT PaintGL (void) = 0;
virtual HRESULT SetFrame ( IDeckLinkVideoFrame_v7_6 *theFrame) = 0;

protected:
virtual ~IDeckLinkGLScreenPreviewHelper_v7_6 () {}; 
};




class IDeckLinkVideoConversion_v7_6 : public IUnknown
{
public:
virtual HRESULT ConvertFrame ( IDeckLinkVideoFrame_v7_6* srcFrame, IDeckLinkVideoFrame_v7_6* dstFrame) = 0;

protected:
virtual ~IDeckLinkVideoConversion_v7_6 () {}; 
};



class IDeckLinkConfiguration_v7_6 : public IUnknown
{
public:
virtual HRESULT GetConfigurationValidator ( IDeckLinkConfiguration_v7_6 **configObject) = 0;
virtual HRESULT WriteConfigurationToPreferences (void) = 0;



virtual HRESULT SetVideoOutputFormat ( BMDVideoConnection_v7_6 videoOutputConnection) = 0;
virtual HRESULT IsVideoOutputActive ( BMDVideoConnection_v7_6 videoOutputConnection, bool *active) = 0;

virtual HRESULT SetAnalogVideoOutputFlags ( BMDAnalogVideoFlags analogVideoFlags) = 0;
virtual HRESULT GetAnalogVideoOutputFlags ( BMDAnalogVideoFlags *analogVideoFlags) = 0;

virtual HRESULT EnableFieldFlickerRemovalWhenPaused ( bool enable) = 0;
virtual HRESULT IsEnabledFieldFlickerRemovalWhenPaused ( bool *enabled) = 0;

virtual HRESULT Set444And3GBpsVideoOutput ( bool enable444VideoOutput, bool enable3GbsOutput) = 0;
virtual HRESULT Get444And3GBpsVideoOutput ( bool *is444VideoOutputEnabled, bool *threeGbsOutputEnabled) = 0;

virtual HRESULT SetVideoOutputConversionMode ( BMDVideoOutputConversionMode conversionMode) = 0;
virtual HRESULT GetVideoOutputConversionMode ( BMDVideoOutputConversionMode *conversionMode) = 0;

virtual HRESULT Set_HD1080p24_to_HD1080i5994_Conversion ( bool enable) = 0;
virtual HRESULT Get_HD1080p24_to_HD1080i5994_Conversion ( bool *enabled) = 0;



virtual HRESULT SetVideoInputFormat ( BMDVideoConnection_v7_6 videoInputFormat) = 0;
virtual HRESULT GetVideoInputFormat ( BMDVideoConnection_v7_6 *videoInputFormat) = 0;

virtual HRESULT SetAnalogVideoInputFlags ( BMDAnalogVideoFlags analogVideoFlags) = 0;
virtual HRESULT GetAnalogVideoInputFlags ( BMDAnalogVideoFlags *analogVideoFlags) = 0;

virtual HRESULT SetVideoInputConversionMode ( BMDVideoInputConversionMode conversionMode) = 0;
virtual HRESULT GetVideoInputConversionMode ( BMDVideoInputConversionMode *conversionMode) = 0;

virtual HRESULT SetBlackVideoOutputDuringCapture ( bool blackOutInCapture) = 0;
virtual HRESULT GetBlackVideoOutputDuringCapture ( bool *blackOutInCapture) = 0;

virtual HRESULT Set32PulldownSequenceInitialTimecodeFrame ( uint32_t aFrameTimecode) = 0;
virtual HRESULT Get32PulldownSequenceInitialTimecodeFrame ( uint32_t *aFrameTimecode) = 0;

virtual HRESULT SetVancSourceLineMapping ( uint32_t activeLine1VANCsource, uint32_t activeLine2VANCsource, uint32_t activeLine3VANCsource) = 0;
virtual HRESULT GetVancSourceLineMapping ( uint32_t *activeLine1VANCsource, uint32_t *activeLine2VANCsource, uint32_t *activeLine3VANCsource) = 0;



virtual HRESULT SetAudioInputFormat ( BMDAudioConnection audioInputFormat) = 0;
virtual HRESULT GetAudioInputFormat ( BMDAudioConnection *audioInputFormat) = 0;
};





extern "C" {

IDeckLinkIterator* CreateDeckLinkIteratorInstance_v7_6 (void);
IDeckLinkGLScreenPreviewHelper_v7_6* CreateOpenGLScreenPreviewHelper_v7_6 (void);
IDeckLinkCocoaScreenPreviewCallback_v7_6* CreateCocoaScreenPreview_v7_6 (void* parentView);
IDeckLinkVideoConversion_v7_6* CreateVideoConversionInstance_v7_6 (void);

};


#endif 
#endif 
