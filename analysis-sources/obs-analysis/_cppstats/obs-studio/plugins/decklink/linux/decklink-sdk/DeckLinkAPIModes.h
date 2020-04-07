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
BMD_CONST REFIID IID_IDeckLinkDisplayModeIterator = {0x9C,0x88,0x49,0x9F,0xF6,0x01,0x40,0x21,0xB8,0x0B,0x03,0x2E,0x4E,0xB4,0x1C,0x35};
BMD_CONST REFIID IID_IDeckLinkDisplayMode = {0x3E,0xB2,0xC1,0xAB,0x0A,0x3D,0x45,0x23,0xA3,0xAD,0xF4,0x0D,0x7F,0xB1,0x4E,0x78};
typedef uint32_t BMDDisplayMode;
enum _BMDDisplayMode {
bmdModeNTSC = 0x6E747363,
bmdModeNTSC2398 = 0x6E743233, 
bmdModePAL = 0x70616C20,
bmdModeNTSCp = 0x6E747370,
bmdModePALp = 0x70616C70,
bmdModeHD1080p2398 = 0x32337073,
bmdModeHD1080p24 = 0x32347073,
bmdModeHD1080p25 = 0x48703235,
bmdModeHD1080p2997 = 0x48703239,
bmdModeHD1080p30 = 0x48703330,
bmdModeHD1080p50 = 0x48703530,
bmdModeHD1080p5994 = 0x48703539,
bmdModeHD1080p6000 = 0x48703630, 
bmdModeHD1080i50 = 0x48693530,
bmdModeHD1080i5994 = 0x48693539,
bmdModeHD1080i6000 = 0x48693630, 
bmdModeHD720p50 = 0x68703530,
bmdModeHD720p5994 = 0x68703539,
bmdModeHD720p60 = 0x68703630,
bmdMode2k2398 = 0x326B3233,
bmdMode2k24 = 0x326B3234,
bmdMode2k25 = 0x326B3235,
bmdMode2kDCI2398 = 0x32643233,
bmdMode2kDCI24 = 0x32643234,
bmdMode2kDCI25 = 0x32643235,
bmdMode2kDCI2997 = 0x32643239,
bmdMode2kDCI30 = 0x32643330,
bmdMode2kDCI50 = 0x32643530,
bmdMode2kDCI5994 = 0x32643539,
bmdMode2kDCI60 = 0x32643630,
bmdMode4K2160p2398 = 0x346B3233,
bmdMode4K2160p24 = 0x346B3234,
bmdMode4K2160p25 = 0x346B3235,
bmdMode4K2160p2997 = 0x346B3239,
bmdMode4K2160p30 = 0x346B3330,
bmdMode4K2160p50 = 0x346B3530,
bmdMode4K2160p5994 = 0x346B3539,
bmdMode4K2160p60 = 0x346B3630,
bmdMode4kDCI2398 = 0x34643233,
bmdMode4kDCI24 = 0x34643234,
bmdMode4kDCI25 = 0x34643235,
bmdMode4kDCI2997 = 0x34643239,
bmdMode4kDCI30 = 0x34643330,
bmdMode4kDCI50 = 0x34643530,
bmdMode4kDCI5994 = 0x34643539,
bmdMode4kDCI60 = 0x34643630,
bmdMode8K4320p2398 = 0x386B3233,
bmdMode8K4320p24 = 0x386B3234,
bmdMode8K4320p25 = 0x386B3235,
bmdMode8K4320p2997 = 0x386B3239,
bmdMode8K4320p30 = 0x386B3330,
bmdMode8K4320p50 = 0x386B3530,
bmdMode8K4320p5994 = 0x386B3539,
bmdMode8K4320p60 = 0x386B3630,
bmdMode8kDCI2398 = 0x38643233,
bmdMode8kDCI24 = 0x38643234,
bmdMode8kDCI25 = 0x38643235,
bmdMode8kDCI2997 = 0x38643239,
bmdMode8kDCI30 = 0x38643330,
bmdMode8kDCI50 = 0x38643530,
bmdMode8kDCI5994 = 0x38643539,
bmdMode8kDCI60 = 0x38643630,
bmdModeCintelRAW = 0x72776369, 
bmdModeCintelCompressedRAW = 0x72776363, 
bmdModeUnknown = 0x69756E6B
};
typedef uint32_t BMDFieldDominance;
enum _BMDFieldDominance {
bmdUnknownFieldDominance = 0,
bmdLowerFieldFirst = 0x6C6F7772,
bmdUpperFieldFirst = 0x75707072,
bmdProgressiveFrame = 0x70726F67,
bmdProgressiveSegmentedFrame = 0x70736620
};
typedef uint32_t BMDPixelFormat;
enum _BMDPixelFormat {
bmdFormat8BitYUV = 0x32767579,
bmdFormat10BitYUV = 0x76323130,
bmdFormat8BitARGB = 32,
bmdFormat8BitBGRA = 0x42475241,
bmdFormat10BitRGB = 0x72323130, 
bmdFormat12BitRGB = 0x52313242, 
bmdFormat12BitRGBLE = 0x5231324C, 
bmdFormat10BitRGBXLE = 0x5231306C, 
bmdFormat10BitRGBX = 0x52313062, 
bmdFormatH265 = 0x68657631, 
bmdFormatDNxHR = 0x41566468,
bmdFormat12BitRAWGRBG = 0x72313270, 
bmdFormat12BitRAWJPEG = 0x72313670 
};
typedef uint32_t BMDDisplayModeFlags;
enum _BMDDisplayModeFlags {
bmdDisplayModeSupports3D = 1 << 0,
bmdDisplayModeColorspaceRec601 = 1 << 1,
bmdDisplayModeColorspaceRec709 = 1 << 2,
bmdDisplayModeColorspaceRec2020 = 1 << 3
};
class IDeckLinkDisplayModeIterator;
class IDeckLinkDisplayMode;
class BMD_PUBLIC IDeckLinkDisplayModeIterator : public IUnknown
{
public:
virtual HRESULT Next ( IDeckLinkDisplayMode **deckLinkDisplayMode) = 0;
protected:
virtual ~IDeckLinkDisplayModeIterator () {} 
};
class BMD_PUBLIC IDeckLinkDisplayMode : public IUnknown
{
public:
virtual HRESULT GetName ( const char **name) = 0;
virtual BMDDisplayMode GetDisplayMode (void) = 0;
virtual long GetWidth (void) = 0;
virtual long GetHeight (void) = 0;
virtual HRESULT GetFrameRate ( BMDTimeValue *frameDuration, BMDTimeScale *timeScale) = 0;
virtual BMDFieldDominance GetFieldDominance (void) = 0;
virtual BMDDisplayModeFlags GetFlags (void) = 0;
protected:
virtual ~IDeckLinkDisplayMode () {} 
};
extern "C" {
}
