


























#if !defined(BMD_DECKLINKAPI_v9_2_H)
#define BMD_DECKLINKAPI_v9_2_H

#include "DeckLinkAPI.h"




#define IID_IDeckLinkInput_v9_2 (REFIID){0x6D,0x40,0xEF,0x78,0x28,0xB9,0x4E,0x21,0x99,0x0D,0x95,0xBB,0x77,0x50,0xA0,0x4F}


#if defined(__cplusplus)



class IDeckLinkInput_v9_2 : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoMode ( BMDDisplayMode displayMode, BMDPixelFormat pixelFormat, BMDVideoInputFlags flags, BMDDisplayModeSupport *result, IDeckLinkDisplayMode **resultDisplayMode) = 0;
virtual HRESULT GetDisplayModeIterator ( IDeckLinkDisplayModeIterator **iterator) = 0;

virtual HRESULT SetScreenPreviewCallback ( IDeckLinkScreenPreviewCallback *previewCallback) = 0;



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
virtual HRESULT SetCallback ( IDeckLinkInputCallback *theCallback) = 0;



virtual HRESULT GetHardwareReferenceClock ( BMDTimeScale desiredTimeScale, BMDTimeValue *hardwareTime, BMDTimeValue *timeInFrame, BMDTimeValue *ticksPerFrame) = 0;

protected:
virtual ~IDeckLinkInput_v9_2 () {}; 
};


#endif 
#endif 
