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
BMD_CONST REFIID IID_IBMDStreamingDeviceNotificationCallback = {0xF9,0x53,0x1D,0x64,0x33,0x05,0x4B,0x29,0xA3,0x87,0x7F,0x74,0xBB,0x0D,0x0E,0x84};
BMD_CONST REFIID IID_IBMDStreamingH264InputCallback = {0x82,0x3C,0x47,0x5F,0x55,0xAE,0x46,0xF9,0x89,0x0C,0x53,0x7C,0xC5,0xCE,0xDC,0xCA};
BMD_CONST REFIID IID_IBMDStreamingDiscovery = {0x2C,0x83,0x74,0x44,0xF9,0x89,0x4D,0x87,0x90,0x1A,0x47,0xC8,0xA3,0x6D,0x09,0x6D};
BMD_CONST REFIID IID_IBMDStreamingVideoEncodingMode = {0x1A,0xB8,0x03,0x5B,0xCD,0x13,0x45,0x8D,0xB6,0xDF,0x5E,0x8F,0x7C,0x21,0x41,0xD9};
BMD_CONST REFIID IID_IBMDStreamingMutableVideoEncodingMode = {0x19,0xBF,0x7D,0x90,0x1E,0x0A,0x40,0x0D,0xB2,0xC6,0xFF,0xC4,0xE7,0x8A,0xD4,0x9D};
BMD_CONST REFIID IID_IBMDStreamingVideoEncodingModePresetIterator = {0x7A,0xC7,0x31,0xA3,0xC9,0x50,0x4A,0xD0,0x80,0x4A,0x83,0x77,0xAA,0x51,0xC6,0xC4};
BMD_CONST REFIID IID_IBMDStreamingDeviceInput = {0x24,0xB6,0xB6,0xEC,0x17,0x27,0x44,0xBB,0x98,0x18,0x34,0xFF,0x08,0x6A,0xCF,0x98};
BMD_CONST REFIID IID_IBMDStreamingH264NALPacket = {0xE2,0x60,0xE9,0x55,0x14,0xBE,0x43,0x95,0x97,0x75,0x9F,0x02,0xCC,0x0A,0x9D,0x89};
BMD_CONST REFIID IID_IBMDStreamingAudioPacket = {0xD9,0xEB,0x59,0x02,0x1A,0xD2,0x43,0xF4,0x9E,0x2C,0x3C,0xFA,0x50,0xB5,0xEE,0x19};
BMD_CONST REFIID IID_IBMDStreamingMPEG2TSPacket = {0x91,0x81,0x0D,0x1C,0x4F,0xB3,0x4A,0xAA,0xAE,0x56,0xFA,0x30,0x1D,0x3D,0xFA,0x4C};
BMD_CONST REFIID IID_IBMDStreamingH264NALParser = {0x58,0x67,0xF1,0x8C,0x5B,0xFA,0x4C,0xCC,0xB2,0xA7,0x9D,0xFD,0x14,0x04,0x17,0xD2};
typedef uint32_t BMDStreamingDeviceMode;
enum _BMDStreamingDeviceMode {
bmdStreamingDeviceIdle = 'idle',
bmdStreamingDeviceEncoding = 'enco',
bmdStreamingDeviceStopping = 'stop',
bmdStreamingDeviceUnknown = 'munk'
};
typedef uint32_t BMDStreamingEncodingFrameRate;
enum _BMDStreamingEncodingFrameRate {
bmdStreamingEncodedFrameRate50i = 'e50i',
bmdStreamingEncodedFrameRate5994i = 'e59i',
bmdStreamingEncodedFrameRate60i = 'e60i',
bmdStreamingEncodedFrameRate2398p = 'e23p',
bmdStreamingEncodedFrameRate24p = 'e24p',
bmdStreamingEncodedFrameRate25p = 'e25p',
bmdStreamingEncodedFrameRate2997p = 'e29p',
bmdStreamingEncodedFrameRate30p = 'e30p',
bmdStreamingEncodedFrameRate50p = 'e50p',
bmdStreamingEncodedFrameRate5994p = 'e59p',
bmdStreamingEncodedFrameRate60p = 'e60p'
};
typedef uint32_t BMDStreamingEncodingSupport;
enum _BMDStreamingEncodingSupport {
bmdStreamingEncodingModeNotSupported = 0,
bmdStreamingEncodingModeSupported, 
bmdStreamingEncodingModeSupportedWithChanges 
};
typedef uint32_t BMDStreamingVideoCodec;
enum _BMDStreamingVideoCodec {
bmdStreamingVideoCodecH264 = 'H264'
};
typedef uint32_t BMDStreamingH264Profile;
enum _BMDStreamingH264Profile {
bmdStreamingH264ProfileHigh = 'high',
bmdStreamingH264ProfileMain = 'main',
bmdStreamingH264ProfileBaseline = 'base'
};
typedef uint32_t BMDStreamingH264Level;
enum _BMDStreamingH264Level {
bmdStreamingH264Level12 = 'lv12',
bmdStreamingH264Level13 = 'lv13',
bmdStreamingH264Level2 = 'lv2 ',
bmdStreamingH264Level21 = 'lv21',
bmdStreamingH264Level22 = 'lv22',
bmdStreamingH264Level3 = 'lv3 ',
bmdStreamingH264Level31 = 'lv31',
bmdStreamingH264Level32 = 'lv32',
bmdStreamingH264Level4 = 'lv4 ',
bmdStreamingH264Level41 = 'lv41',
bmdStreamingH264Level42 = 'lv42'
};
typedef uint32_t BMDStreamingH264EntropyCoding;
enum _BMDStreamingH264EntropyCoding {
bmdStreamingH264EntropyCodingCAVLC = 'EVLC',
bmdStreamingH264EntropyCodingCABAC = 'EBAC'
};
typedef uint32_t BMDStreamingAudioCodec;
enum _BMDStreamingAudioCodec {
bmdStreamingAudioCodecAAC = 'AAC '
};
typedef uint32_t BMDStreamingEncodingModePropertyID;
enum _BMDStreamingEncodingModePropertyID {
bmdStreamingEncodingPropertyVideoFrameRate = 'vfrt', 
bmdStreamingEncodingPropertyVideoBitRateKbps = 'vbrt',
bmdStreamingEncodingPropertyH264Profile = 'hprf',
bmdStreamingEncodingPropertyH264Level = 'hlvl',
bmdStreamingEncodingPropertyH264EntropyCoding = 'hent',
bmdStreamingEncodingPropertyH264HasBFrames = 'hBfr',
bmdStreamingEncodingPropertyAudioCodec = 'acdc',
bmdStreamingEncodingPropertyAudioSampleRate = 'asrt',
bmdStreamingEncodingPropertyAudioChannelCount = 'achc',
bmdStreamingEncodingPropertyAudioBitRateKbps = 'abrt'
};
class IBMDStreamingDeviceNotificationCallback;
class IBMDStreamingH264InputCallback;
class IBMDStreamingDiscovery;
class IBMDStreamingVideoEncodingMode;
class IBMDStreamingMutableVideoEncodingMode;
class IBMDStreamingVideoEncodingModePresetIterator;
class IBMDStreamingDeviceInput;
class IBMDStreamingH264NALPacket;
class IBMDStreamingAudioPacket;
class IBMDStreamingMPEG2TSPacket;
class IBMDStreamingH264NALParser;
class BMD_PUBLIC IBMDStreamingDeviceNotificationCallback : public IUnknown
{
public:
virtual HRESULT StreamingDeviceArrived ( IDeckLink* device) = 0;
virtual HRESULT StreamingDeviceRemoved ( IDeckLink* device) = 0;
virtual HRESULT StreamingDeviceModeChanged ( IDeckLink* device, BMDStreamingDeviceMode mode) = 0;
protected:
virtual ~IBMDStreamingDeviceNotificationCallback () {} 
};
class BMD_PUBLIC IBMDStreamingH264InputCallback : public IUnknown
{
public:
virtual HRESULT H264NALPacketArrived ( IBMDStreamingH264NALPacket* nalPacket) = 0;
virtual HRESULT H264AudioPacketArrived ( IBMDStreamingAudioPacket* audioPacket) = 0;
virtual HRESULT MPEG2TSPacketArrived ( IBMDStreamingMPEG2TSPacket* tsPacket) = 0;
virtual HRESULT H264VideoInputConnectorScanningChanged (void) = 0;
virtual HRESULT H264VideoInputConnectorChanged (void) = 0;
virtual HRESULT H264VideoInputModeChanged (void) = 0;
protected:
virtual ~IBMDStreamingH264InputCallback () {} 
};
class BMD_PUBLIC IBMDStreamingDiscovery : public IUnknown
{
public:
virtual HRESULT InstallDeviceNotifications ( IBMDStreamingDeviceNotificationCallback* theCallback) = 0;
virtual HRESULT UninstallDeviceNotifications (void) = 0;
protected:
virtual ~IBMDStreamingDiscovery () {} 
};
class BMD_PUBLIC IBMDStreamingVideoEncodingMode : public IUnknown
{
public:
virtual HRESULT GetName ( CFStringRef *name) = 0;
virtual unsigned int GetPresetID (void) = 0;
virtual unsigned int GetSourcePositionX (void) = 0;
virtual unsigned int GetSourcePositionY (void) = 0;
virtual unsigned int GetSourceWidth (void) = 0;
virtual unsigned int GetSourceHeight (void) = 0;
virtual unsigned int GetDestWidth (void) = 0;
virtual unsigned int GetDestHeight (void) = 0;
virtual HRESULT GetFlag ( BMDStreamingEncodingModePropertyID cfgID, bool* value) = 0;
virtual HRESULT GetInt ( BMDStreamingEncodingModePropertyID cfgID, int64_t* value) = 0;
virtual HRESULT GetFloat ( BMDStreamingEncodingModePropertyID cfgID, double* value) = 0;
virtual HRESULT GetString ( BMDStreamingEncodingModePropertyID cfgID, CFStringRef *value) = 0;
virtual HRESULT CreateMutableVideoEncodingMode ( IBMDStreamingMutableVideoEncodingMode** newEncodingMode) = 0; 
protected:
virtual ~IBMDStreamingVideoEncodingMode () {} 
};
class BMD_PUBLIC IBMDStreamingMutableVideoEncodingMode : public IBMDStreamingVideoEncodingMode
{
public:
virtual HRESULT SetSourceRect ( uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) = 0;
virtual HRESULT SetDestSize ( uint32_t width, uint32_t height) = 0;
virtual HRESULT SetFlag ( BMDStreamingEncodingModePropertyID cfgID, bool value) = 0;
virtual HRESULT SetInt ( BMDStreamingEncodingModePropertyID cfgID, int64_t value) = 0;
virtual HRESULT SetFloat ( BMDStreamingEncodingModePropertyID cfgID, double value) = 0;
virtual HRESULT SetString ( BMDStreamingEncodingModePropertyID cfgID, CFStringRef value) = 0;
protected:
virtual ~IBMDStreamingMutableVideoEncodingMode () {} 
};
class BMD_PUBLIC IBMDStreamingVideoEncodingModePresetIterator : public IUnknown
{
public:
virtual HRESULT Next ( IBMDStreamingVideoEncodingMode** videoEncodingMode) = 0;
protected:
virtual ~IBMDStreamingVideoEncodingModePresetIterator () {} 
};
class BMD_PUBLIC IBMDStreamingDeviceInput : public IUnknown
{
public:
virtual HRESULT DoesSupportVideoInputMode ( BMDDisplayMode inputMode, bool* result) = 0;
virtual HRESULT GetVideoInputModeIterator ( IDeckLinkDisplayModeIterator** iterator) = 0;
virtual HRESULT SetVideoInputMode ( BMDDisplayMode inputMode) = 0;
virtual HRESULT GetCurrentDetectedVideoInputMode ( BMDDisplayMode* detectedMode) = 0;
virtual HRESULT GetVideoEncodingMode ( IBMDStreamingVideoEncodingMode** encodingMode) = 0;
virtual HRESULT GetVideoEncodingModePresetIterator ( BMDDisplayMode inputMode, IBMDStreamingVideoEncodingModePresetIterator** iterator) = 0;
virtual HRESULT DoesSupportVideoEncodingMode ( BMDDisplayMode inputMode, IBMDStreamingVideoEncodingMode* encodingMode, BMDStreamingEncodingSupport* result, IBMDStreamingVideoEncodingMode** changedEncodingMode) = 0;
virtual HRESULT SetVideoEncodingMode ( IBMDStreamingVideoEncodingMode* encodingMode) = 0;
virtual HRESULT StartCapture (void) = 0;
virtual HRESULT StopCapture (void) = 0;
virtual HRESULT SetCallback ( IUnknown* theCallback) = 0;
protected:
virtual ~IBMDStreamingDeviceInput () {} 
};
class BMD_PUBLIC IBMDStreamingH264NALPacket : public IUnknown
{
public:
virtual long GetPayloadSize (void) = 0;
virtual HRESULT GetBytes ( void** buffer) = 0;
virtual HRESULT GetBytesWithSizePrefix ( void** buffer) = 0; 
virtual HRESULT GetDisplayTime ( uint64_t requestedTimeScale, uint64_t* displayTime) = 0;
virtual HRESULT GetPacketIndex ( uint32_t* packetIndex) = 0; 
protected:
virtual ~IBMDStreamingH264NALPacket () {} 
};
class BMD_PUBLIC IBMDStreamingAudioPacket : public IUnknown
{
public:
virtual BMDStreamingAudioCodec GetCodec (void) = 0;
virtual long GetPayloadSize (void) = 0;
virtual HRESULT GetBytes ( void** buffer) = 0;
virtual HRESULT GetPlayTime ( uint64_t requestedTimeScale, uint64_t* playTime) = 0;
virtual HRESULT GetPacketIndex ( uint32_t* packetIndex) = 0; 
protected:
virtual ~IBMDStreamingAudioPacket () {} 
};
class BMD_PUBLIC IBMDStreamingMPEG2TSPacket : public IUnknown
{
public:
virtual long GetPayloadSize (void) = 0;
virtual HRESULT GetBytes ( void** buffer) = 0;
protected:
virtual ~IBMDStreamingMPEG2TSPacket () {} 
};
class BMD_PUBLIC IBMDStreamingH264NALParser : public IUnknown
{
public:
virtual HRESULT IsNALSequenceParameterSet ( IBMDStreamingH264NALPacket* nal) = 0;
virtual HRESULT IsNALPictureParameterSet ( IBMDStreamingH264NALPacket* nal) = 0;
virtual HRESULT GetProfileAndLevelFromSPS ( IBMDStreamingH264NALPacket* nal, uint32_t* profileIdc, uint32_t* profileCompatability, uint32_t* levelIdc) = 0;
protected:
virtual ~IBMDStreamingH264NALParser () {} 
};
extern "C" {
IBMDStreamingDiscovery* BMD_PUBLIC CreateBMDStreamingDiscoveryInstance (void);
IBMDStreamingH264NALParser* BMD_PUBLIC CreateBMDStreamingH264NALParser (void);
}
