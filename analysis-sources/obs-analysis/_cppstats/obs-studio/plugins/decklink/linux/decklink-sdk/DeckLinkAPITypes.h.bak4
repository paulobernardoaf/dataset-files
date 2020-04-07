


























#if !defined(BMD_DECKLINKAPITYPES_H)
#define BMD_DECKLINKAPITYPES_H


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



typedef int64_t BMDTimeValue;
typedef int64_t BMDTimeScale;
typedef uint32_t BMDTimecodeBCD;
typedef uint32_t BMDTimecodeUserBits;



BMD_CONST REFIID IID_IDeckLinkTimecode = {0xBC,0x6C,0xFB,0xD3,0x83,0x17,0x43,0x25,0xAC,0x1C,0x12,0x16,0x39,0x1E,0x93,0x40};



typedef uint32_t BMDTimecodeFlags;
enum _BMDTimecodeFlags {
bmdTimecodeFlagDefault = 0,
bmdTimecodeIsDropFrame = 1 << 0,
bmdTimecodeFieldMark = 1 << 1,
bmdTimecodeColorFrame = 1 << 2
};



typedef uint32_t BMDVideoConnection;
enum _BMDVideoConnection {
bmdVideoConnectionSDI = 1 << 0,
bmdVideoConnectionHDMI = 1 << 1,
bmdVideoConnectionOpticalSDI = 1 << 2,
bmdVideoConnectionComponent = 1 << 3,
bmdVideoConnectionComposite = 1 << 4,
bmdVideoConnectionSVideo = 1 << 5
};



typedef uint32_t BMDAudioConnection;
enum _BMDAudioConnection {
bmdAudioConnectionEmbedded = 1 << 0,
bmdAudioConnectionAESEBU = 1 << 1,
bmdAudioConnectionAnalog = 1 << 2,
bmdAudioConnectionAnalogXLR = 1 << 3,
bmdAudioConnectionAnalogRCA = 1 << 4,
bmdAudioConnectionMicrophone = 1 << 5,
bmdAudioConnectionHeadphones = 1 << 6
};



typedef uint32_t BMDDeckControlConnection;
enum _BMDDeckControlConnection {
bmdDeckControlConnectionRS422Remote1 = 1 << 0,
bmdDeckControlConnectionRS422Remote2 = 1 << 1
};



class IDeckLinkTimecode;



class BMD_PUBLIC IDeckLinkTimecode : public IUnknown
{
public:
virtual BMDTimecodeBCD GetBCD (void) = 0;
virtual HRESULT GetComponents ( uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *frames) = 0;
virtual HRESULT GetString ( const char **timecode) = 0;
virtual BMDTimecodeFlags GetFlags (void) = 0;
virtual HRESULT GetTimecodeUserBits ( BMDTimecodeUserBits *userBits) = 0;

protected:
virtual ~IDeckLinkTimecode () {} 
};



extern "C" {


}


#endif 
