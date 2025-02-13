


























#if !defined(BMD_DECKLINKAPIMODES_H)
#define BMD_DECKLINKAPIMODES_H


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



bmdModeNTSC = 'ntsc',
bmdModeNTSC2398 = 'nt23', 
bmdModePAL = 'pal ',
bmdModeNTSCp = 'ntsp',
bmdModePALp = 'palp',



bmdModeHD1080p2398 = '23ps',
bmdModeHD1080p24 = '24ps',
bmdModeHD1080p25 = 'Hp25',
bmdModeHD1080p2997 = 'Hp29',
bmdModeHD1080p30 = 'Hp30',
bmdModeHD1080p50 = 'Hp50',
bmdModeHD1080p5994 = 'Hp59',
bmdModeHD1080p6000 = 'Hp60', 
bmdModeHD1080i50 = 'Hi50',
bmdModeHD1080i5994 = 'Hi59',
bmdModeHD1080i6000 = 'Hi60', 



bmdModeHD720p50 = 'hp50',
bmdModeHD720p5994 = 'hp59',
bmdModeHD720p60 = 'hp60',



bmdMode2k2398 = '2k23',
bmdMode2k24 = '2k24',
bmdMode2k25 = '2k25',



bmdMode2kDCI2398 = '2d23',
bmdMode2kDCI24 = '2d24',
bmdMode2kDCI25 = '2d25',
bmdMode2kDCI2997 = '2d29',
bmdMode2kDCI30 = '2d30',
bmdMode2kDCI50 = '2d50',
bmdMode2kDCI5994 = '2d59',
bmdMode2kDCI60 = '2d60',



bmdMode4K2160p2398 = '4k23',
bmdMode4K2160p24 = '4k24',
bmdMode4K2160p25 = '4k25',
bmdMode4K2160p2997 = '4k29',
bmdMode4K2160p30 = '4k30',
bmdMode4K2160p50 = '4k50',
bmdMode4K2160p5994 = '4k59',
bmdMode4K2160p60 = '4k60',



bmdMode4kDCI2398 = '4d23',
bmdMode4kDCI24 = '4d24',
bmdMode4kDCI25 = '4d25',
bmdMode4kDCI2997 = '4d29',
bmdMode4kDCI30 = '4d30',
bmdMode4kDCI50 = '4d50',
bmdMode4kDCI5994 = '4d59',
bmdMode4kDCI60 = '4d60',



bmdMode8K4320p2398 = '8k23',
bmdMode8K4320p24 = '8k24',
bmdMode8K4320p25 = '8k25',
bmdMode8K4320p2997 = '8k29',
bmdMode8K4320p30 = '8k30',
bmdMode8K4320p50 = '8k50',
bmdMode8K4320p5994 = '8k59',
bmdMode8K4320p60 = '8k60',



bmdMode8kDCI2398 = '8d23',
bmdMode8kDCI24 = '8d24',
bmdMode8kDCI25 = '8d25',
bmdMode8kDCI2997 = '8d29',
bmdMode8kDCI30 = '8d30',
bmdMode8kDCI50 = '8d50',
bmdMode8kDCI5994 = '8d59',
bmdMode8kDCI60 = '8d60',



bmdModeCintelRAW = 'rwci', 
bmdModeCintelCompressedRAW = 'rwcc', 



bmdModeUnknown = 'iunk'
};



typedef uint32_t BMDFieldDominance;
enum _BMDFieldDominance {
bmdUnknownFieldDominance = 0,
bmdLowerFieldFirst = 'lowr',
bmdUpperFieldFirst = 'uppr',
bmdProgressiveFrame = 'prog',
bmdProgressiveSegmentedFrame = 'psf '
};



typedef uint32_t BMDPixelFormat;
enum _BMDPixelFormat {
bmdFormat8BitYUV = '2vuy',
bmdFormat10BitYUV = 'v210',
bmdFormat8BitARGB = 32,
bmdFormat8BitBGRA = 'BGRA',
bmdFormat10BitRGB = 'r210', 
bmdFormat12BitRGB = 'R12B', 
bmdFormat12BitRGBLE = 'R12L', 
bmdFormat10BitRGBXLE = 'R10l', 
bmdFormat10BitRGBX = 'R10b', 
bmdFormatH265 = 'hev1', 



bmdFormatDNxHR = 'AVdh',



bmdFormat12BitRAWGRBG = 'r12p', 
bmdFormat12BitRAWJPEG = 'r16p' 
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
virtual HRESULT GetName ( CFStringRef *name) = 0;
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


#endif 
