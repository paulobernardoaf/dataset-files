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
BMD_CONST REFIID IID_IDeckLinkDeckControlStatusCallback = {0x53,0x43,0x6F,0xFB,0xB4,0x34,0x49,0x06,0xBA,0xDC,0xAE,0x30,0x60,0xFF,0xE8,0xEF};
BMD_CONST REFIID IID_IDeckLinkDeckControl = {0x8E,0x1C,0x3A,0xCE,0x19,0xC7,0x4E,0x00,0x8B,0x92,0xD8,0x04,0x31,0xD9,0x58,0xBE};
typedef uint32_t BMDDeckControlMode;
enum _BMDDeckControlMode {
bmdDeckControlNotOpened = 0x6E746F70,
bmdDeckControlVTRControlMode = 0x76747263,
bmdDeckControlExportMode = 0x6578706D,
bmdDeckControlCaptureMode = 0x6361706D
};
typedef uint32_t BMDDeckControlEvent;
enum _BMDDeckControlEvent {
bmdDeckControlAbortedEvent = 0x61627465, 
bmdDeckControlPrepareForExportEvent = 0x70666565, 
bmdDeckControlExportCompleteEvent = 0x65786365, 
bmdDeckControlPrepareForCaptureEvent = 0x70666365, 
bmdDeckControlCaptureCompleteEvent = 0x63636576 
};
typedef uint32_t BMDDeckControlVTRControlState;
enum _BMDDeckControlVTRControlState {
bmdDeckControlNotInVTRControlMode = 0x6E76636D,
bmdDeckControlVTRControlPlaying = 0x76747270,
bmdDeckControlVTRControlRecording = 0x76747272,
bmdDeckControlVTRControlStill = 0x76747261,
bmdDeckControlVTRControlShuttleForward = 0x76747366,
bmdDeckControlVTRControlShuttleReverse = 0x76747372,
bmdDeckControlVTRControlJogForward = 0x76746A66,
bmdDeckControlVTRControlJogReverse = 0x76746A72,
bmdDeckControlVTRControlStopped = 0x7674726F
};
typedef uint32_t BMDDeckControlStatusFlags;
enum _BMDDeckControlStatusFlags {
bmdDeckControlStatusDeckConnected = 1 << 0,
bmdDeckControlStatusRemoteMode = 1 << 1,
bmdDeckControlStatusRecordInhibited = 1 << 2,
bmdDeckControlStatusCassetteOut = 1 << 3
};
typedef uint32_t BMDDeckControlExportModeOpsFlags;
enum _BMDDeckControlExportModeOpsFlags {
bmdDeckControlExportModeInsertVideo = 1 << 0,
bmdDeckControlExportModeInsertAudio1 = 1 << 1,
bmdDeckControlExportModeInsertAudio2 = 1 << 2,
bmdDeckControlExportModeInsertAudio3 = 1 << 3,
bmdDeckControlExportModeInsertAudio4 = 1 << 4,
bmdDeckControlExportModeInsertAudio5 = 1 << 5,
bmdDeckControlExportModeInsertAudio6 = 1 << 6,
bmdDeckControlExportModeInsertAudio7 = 1 << 7,
bmdDeckControlExportModeInsertAudio8 = 1 << 8,
bmdDeckControlExportModeInsertAudio9 = 1 << 9,
bmdDeckControlExportModeInsertAudio10 = 1 << 10,
bmdDeckControlExportModeInsertAudio11 = 1 << 11,
bmdDeckControlExportModeInsertAudio12 = 1 << 12,
bmdDeckControlExportModeInsertTimeCode = 1 << 13,
bmdDeckControlExportModeInsertAssemble = 1 << 14,
bmdDeckControlExportModeInsertPreview = 1 << 15,
bmdDeckControlUseManualExport = 1 << 16
};
typedef uint32_t BMDDeckControlError;
enum _BMDDeckControlError {
bmdDeckControlNoError = 0x6E6F6572,
bmdDeckControlModeError = 0x6D6F6572,
bmdDeckControlMissedInPointError = 0x6D696572,
bmdDeckControlDeckTimeoutError = 0x64746572,
bmdDeckControlCommandFailedError = 0x63666572,
bmdDeckControlDeviceAlreadyOpenedError = 0x64616C6F,
bmdDeckControlFailedToOpenDeviceError = 0x66646572,
bmdDeckControlInLocalModeError = 0x6C6D6572,
bmdDeckControlEndOfTapeError = 0x65746572,
bmdDeckControlUserAbortError = 0x75616572,
bmdDeckControlNoTapeInDeckError = 0x6E746572,
bmdDeckControlNoVideoFromCardError = 0x6E766663,
bmdDeckControlNoCommunicationError = 0x6E636F6D,
bmdDeckControlBufferTooSmallError = 0x6274736D,
bmdDeckControlBadChecksumError = 0x63686B73,
bmdDeckControlUnknownError = 0x756E6572
};
class IDeckLinkDeckControlStatusCallback;
class IDeckLinkDeckControl;
class BMD_PUBLIC IDeckLinkDeckControlStatusCallback : public IUnknown
{
public:
virtual HRESULT TimecodeUpdate ( BMDTimecodeBCD currentTimecode) = 0;
virtual HRESULT VTRControlStateChanged ( BMDDeckControlVTRControlState newState, BMDDeckControlError error) = 0;
virtual HRESULT DeckControlEventReceived ( BMDDeckControlEvent event, BMDDeckControlError error) = 0;
virtual HRESULT DeckControlStatusChanged ( BMDDeckControlStatusFlags flags, uint32_t mask) = 0;
protected:
virtual ~IDeckLinkDeckControlStatusCallback () {} 
};
class BMD_PUBLIC IDeckLinkDeckControl : public IUnknown
{
public:
virtual HRESULT Open ( BMDTimeScale timeScale, BMDTimeValue timeValue, bool timecodeIsDropFrame, BMDDeckControlError *error) = 0;
virtual HRESULT Close ( bool standbyOn) = 0;
virtual HRESULT GetCurrentState ( BMDDeckControlMode *mode, BMDDeckControlVTRControlState *vtrControlState, BMDDeckControlStatusFlags *flags) = 0;
virtual HRESULT SetStandby ( bool standbyOn) = 0;
virtual HRESULT SendCommand ( uint8_t *inBuffer, uint32_t inBufferSize, uint8_t *outBuffer, uint32_t *outDataSize, uint32_t outBufferSize, BMDDeckControlError *error) = 0;
virtual HRESULT Play ( BMDDeckControlError *error) = 0;
virtual HRESULT Stop ( BMDDeckControlError *error) = 0;
virtual HRESULT TogglePlayStop ( BMDDeckControlError *error) = 0;
virtual HRESULT Eject ( BMDDeckControlError *error) = 0;
virtual HRESULT GoToTimecode ( BMDTimecodeBCD timecode, BMDDeckControlError *error) = 0;
virtual HRESULT FastForward ( bool viewTape, BMDDeckControlError *error) = 0;
virtual HRESULT Rewind ( bool viewTape, BMDDeckControlError *error) = 0;
virtual HRESULT StepForward ( BMDDeckControlError *error) = 0;
virtual HRESULT StepBack ( BMDDeckControlError *error) = 0;
virtual HRESULT Jog ( double rate, BMDDeckControlError *error) = 0;
virtual HRESULT Shuttle ( double rate, BMDDeckControlError *error) = 0;
virtual HRESULT GetTimecodeString ( const char **currentTimeCode, BMDDeckControlError *error) = 0;
virtual HRESULT GetTimecode ( IDeckLinkTimecode **currentTimecode, BMDDeckControlError *error) = 0;
virtual HRESULT GetTimecodeBCD ( BMDTimecodeBCD *currentTimecode, BMDDeckControlError *error) = 0;
virtual HRESULT SetPreroll ( uint32_t prerollSeconds) = 0;
virtual HRESULT GetPreroll ( uint32_t *prerollSeconds) = 0;
virtual HRESULT SetExportOffset ( int32_t exportOffsetFields) = 0;
virtual HRESULT GetExportOffset ( int32_t *exportOffsetFields) = 0;
virtual HRESULT GetManualExportOffset ( int32_t *deckManualExportOffsetFields) = 0;
virtual HRESULT SetCaptureOffset ( int32_t captureOffsetFields) = 0;
virtual HRESULT GetCaptureOffset ( int32_t *captureOffsetFields) = 0;
virtual HRESULT StartExport ( BMDTimecodeBCD inTimecode, BMDTimecodeBCD outTimecode, BMDDeckControlExportModeOpsFlags exportModeOps, BMDDeckControlError *error) = 0;
virtual HRESULT StartCapture ( bool useVITC, BMDTimecodeBCD inTimecode, BMDTimecodeBCD outTimecode, BMDDeckControlError *error) = 0;
virtual HRESULT GetDeviceID ( uint16_t *deviceId, BMDDeckControlError *error) = 0;
virtual HRESULT Abort (void) = 0;
virtual HRESULT CrashRecordStart ( BMDDeckControlError *error) = 0;
virtual HRESULT CrashRecordStop ( BMDDeckControlError *error) = 0;
virtual HRESULT SetCallback ( IDeckLinkDeckControlStatusCallback *callback) = 0;
protected:
virtual ~IDeckLinkDeckControl () {} 
};
extern "C" {
}
