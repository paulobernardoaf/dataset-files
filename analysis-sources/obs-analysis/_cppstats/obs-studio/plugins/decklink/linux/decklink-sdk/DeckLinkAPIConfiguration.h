#if !defined(BMD_CONST)
#if defined(_MSC_VER)
#define BMD_CONST __declspec(selectany) static const
#else
#define BMD_CONST static const
#endif
#endif
#if !defined(BMD_PUBLIC)
#define BMD_PUBLIC
#endif
BMD_CONST REFIID IID_IDeckLinkConfiguration = {0xEF,0x90,0x38,0x0B,0x4A,0xE5,0x43,0x46,0x90,0x77,0xE2,0x88,0xE1,0x49,0xF1,0x29};
BMD_CONST REFIID IID_IDeckLinkEncoderConfiguration = {0x13,0x80,0x50,0xE5,0xC6,0x0A,0x45,0x52,0xBF,0x3F,0x0F,0x35,0x80,0x49,0x32,0x7E};
typedef uint32_t BMDDeckLinkConfigurationID;
enum _BMDDeckLinkConfigurationID {
bmdDeckLinkConfigSwapSerialRxTx = 0x73737274,
bmdDeckLinkConfigHDMI3DPackingFormat = 0x33647066,
bmdDeckLinkConfigBypass = 0x62797073,
bmdDeckLinkConfigClockTimingAdjustment = 0x63746164,
bmdDeckLinkConfigDuplexMode = 0x64757078,
bmdDeckLinkConfigAnalogAudioConsumerLevels = 0x6161636C,
bmdDeckLinkConfigFieldFlickerRemoval = 0x66646672,
bmdDeckLinkConfigHD1080p24ToHD1080i5994Conversion = 0x746F3539,
bmdDeckLinkConfig444SDIVideoOutput = 0x3434346F,
bmdDeckLinkConfigBlackVideoOutputDuringCapture = 0x62766F63,
bmdDeckLinkConfigLowLatencyVideoOutput = 0x6C6C766F,
bmdDeckLinkConfigDownConversionOnAllAnalogOutput = 0x6361616F,
bmdDeckLinkConfigSMPTELevelAOutput = 0x736D7461,
bmdDeckLinkConfigRec2020Output = 0x72656332, 
bmdDeckLinkConfigQuadLinkSDIVideoOutputSquareDivisionSplit = 0x53445153,
bmdDeckLinkConfigOutput1080pAsPsF = 0x70667072,
bmdDeckLinkConfigVideoOutputConnection = 0x766F636E,
bmdDeckLinkConfigVideoOutputConversionMode = 0x766F636D,
bmdDeckLinkConfigAnalogVideoOutputFlags = 0x61766F66,
bmdDeckLinkConfigReferenceInputTimingOffset = 0x676C6F74,
bmdDeckLinkConfigVideoOutputIdleOperation = 0x766F696F,
bmdDeckLinkConfigDefaultVideoOutputMode = 0x64766F6D,
bmdDeckLinkConfigDefaultVideoOutputModeFlags = 0x64766F66,
bmdDeckLinkConfigSDIOutputLinkConfiguration = 0x736F6C63,
bmdDeckLinkConfigVideoOutputComponentLumaGain = 0x6F636C67,
bmdDeckLinkConfigVideoOutputComponentChromaBlueGain = 0x6F636362,
bmdDeckLinkConfigVideoOutputComponentChromaRedGain = 0x6F636372,
bmdDeckLinkConfigVideoOutputCompositeLumaGain = 0x6F696C67,
bmdDeckLinkConfigVideoOutputCompositeChromaGain = 0x6F696367,
bmdDeckLinkConfigVideoOutputSVideoLumaGain = 0x6F736C67,
bmdDeckLinkConfigVideoOutputSVideoChromaGain = 0x6F736367,
bmdDeckLinkConfigVideoInputScanning = 0x76697363, 
bmdDeckLinkConfigUseDedicatedLTCInput = 0x646C7463, 
bmdDeckLinkConfigSDIInput3DPayloadOverride = 0x33646473,
bmdDeckLinkConfigCapture1080pAsPsF = 0x63667072,
bmdDeckLinkConfigVideoInputConnection = 0x7669636E,
bmdDeckLinkConfigAnalogVideoInputFlags = 0x61766966,
bmdDeckLinkConfigVideoInputConversionMode = 0x7669636D,
bmdDeckLinkConfig32PulldownSequenceInitialTimecodeFrame = 0x70646966,
bmdDeckLinkConfigVANCSourceLine1Mapping = 0x76736C31,
bmdDeckLinkConfigVANCSourceLine2Mapping = 0x76736C32,
bmdDeckLinkConfigVANCSourceLine3Mapping = 0x76736C33,
bmdDeckLinkConfigCapturePassThroughMode = 0x6370746D,
bmdDeckLinkConfigVideoInputComponentLumaGain = 0x69636C67,
bmdDeckLinkConfigVideoInputComponentChromaBlueGain = 0x69636362,
bmdDeckLinkConfigVideoInputComponentChromaRedGain = 0x69636372,
bmdDeckLinkConfigVideoInputCompositeLumaGain = 0x69696C67,
bmdDeckLinkConfigVideoInputCompositeChromaGain = 0x69696367,
bmdDeckLinkConfigVideoInputSVideoLumaGain = 0x69736C67,
bmdDeckLinkConfigVideoInputSVideoChromaGain = 0x69736367,
bmdDeckLinkConfigMicrophonePhantomPower = 0x6D706870,
bmdDeckLinkConfigAudioInputConnection = 0x6169636E,
bmdDeckLinkConfigAnalogAudioInputScaleChannel1 = 0x61697331,
bmdDeckLinkConfigAnalogAudioInputScaleChannel2 = 0x61697332,
bmdDeckLinkConfigAnalogAudioInputScaleChannel3 = 0x61697333,
bmdDeckLinkConfigAnalogAudioInputScaleChannel4 = 0x61697334,
bmdDeckLinkConfigDigitalAudioInputScale = 0x64616973,
bmdDeckLinkConfigMicrophoneInputGain = 0x6D696367,
bmdDeckLinkConfigAudioOutputAESAnalogSwitch = 0x616F6161,
bmdDeckLinkConfigAnalogAudioOutputScaleChannel1 = 0x616F7331,
bmdDeckLinkConfigAnalogAudioOutputScaleChannel2 = 0x616F7332,
bmdDeckLinkConfigAnalogAudioOutputScaleChannel3 = 0x616F7333,
bmdDeckLinkConfigAnalogAudioOutputScaleChannel4 = 0x616F7334,
bmdDeckLinkConfigDigitalAudioOutputScale = 0x64616F73,
bmdDeckLinkConfigHeadphoneVolume = 0x68766F6C,
bmdDeckLinkConfigDeviceInformationLabel = 0x64696C61,
bmdDeckLinkConfigDeviceInformationSerialNumber = 0x6469736E,
bmdDeckLinkConfigDeviceInformationCompany = 0x6469636F,
bmdDeckLinkConfigDeviceInformationPhone = 0x64697068,
bmdDeckLinkConfigDeviceInformationEmail = 0x6469656D,
bmdDeckLinkConfigDeviceInformationDate = 0x64696461,
bmdDeckLinkConfigDeckControlConnection = 0x6463636F
};
typedef uint32_t BMDDeckLinkEncoderConfigurationID;
enum _BMDDeckLinkEncoderConfigurationID {
bmdDeckLinkEncoderConfigPreferredBitDepth = 0x65706272,
bmdDeckLinkEncoderConfigFrameCodingMode = 0x6566636D,
bmdDeckLinkEncoderConfigH265TargetBitrate = 0x68746272,
bmdDeckLinkEncoderConfigDNxHRCompressionID = 0x64636964,
bmdDeckLinkEncoderConfigDNxHRLevel = 0x646C6576,
bmdDeckLinkEncoderConfigMPEG4SampleDescription = 0x73747345, 
bmdDeckLinkEncoderConfigMPEG4CodecSpecificDesc = 0x65736473 
};
class IDeckLinkConfiguration;
class IDeckLinkEncoderConfiguration;
class BMD_PUBLIC IDeckLinkConfiguration : public IUnknown
{
public:
virtual HRESULT SetFlag ( BMDDeckLinkConfigurationID cfgID, bool value) = 0;
virtual HRESULT GetFlag ( BMDDeckLinkConfigurationID cfgID, bool *value) = 0;
virtual HRESULT SetInt ( BMDDeckLinkConfigurationID cfgID, int64_t value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkConfigurationID cfgID, int64_t *value) = 0;
virtual HRESULT SetFloat ( BMDDeckLinkConfigurationID cfgID, double value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkConfigurationID cfgID, double *value) = 0;
virtual HRESULT SetString ( BMDDeckLinkConfigurationID cfgID, const char *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkConfigurationID cfgID, const char **value) = 0;
virtual HRESULT WriteConfigurationToPreferences (void) = 0;
protected:
virtual ~IDeckLinkConfiguration () {} 
};
class BMD_PUBLIC IDeckLinkEncoderConfiguration : public IUnknown
{
public:
virtual HRESULT SetFlag ( BMDDeckLinkEncoderConfigurationID cfgID, bool value) = 0;
virtual HRESULT GetFlag ( BMDDeckLinkEncoderConfigurationID cfgID, bool *value) = 0;
virtual HRESULT SetInt ( BMDDeckLinkEncoderConfigurationID cfgID, int64_t value) = 0;
virtual HRESULT GetInt ( BMDDeckLinkEncoderConfigurationID cfgID, int64_t *value) = 0;
virtual HRESULT SetFloat ( BMDDeckLinkEncoderConfigurationID cfgID, double value) = 0;
virtual HRESULT GetFloat ( BMDDeckLinkEncoderConfigurationID cfgID, double *value) = 0;
virtual HRESULT SetString ( BMDDeckLinkEncoderConfigurationID cfgID, const char *value) = 0;
virtual HRESULT GetString ( BMDDeckLinkEncoderConfigurationID cfgID, const char **value) = 0;
virtual HRESULT GetBytes ( BMDDeckLinkEncoderConfigurationID cfgID, void *buffer , uint32_t *bufferSize) = 0;
protected:
virtual ~IDeckLinkEncoderConfiguration () {} 
};
extern "C" {
}
