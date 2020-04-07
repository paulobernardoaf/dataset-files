#include "mfxcommon.h"
#if !defined (__GNUC__)
#pragma warning(disable: 4201)
#endif
#if defined(__cplusplus)
extern "C"
{
#endif 
enum {
MFX_CODEC_AAC =MFX_MAKEFOURCC('A','A','C',' '),
MFX_CODEC_MP3 =MFX_MAKEFOURCC('M','P','3',' ')
};
enum {
MFX_PROFILE_AAC_LC =2,
MFX_PROFILE_AAC_LTP =4,
MFX_PROFILE_AAC_MAIN =1,
MFX_PROFILE_AAC_SSR =3,
MFX_PROFILE_AAC_HE =5,
MFX_PROFILE_AAC_ALS =0x20,
MFX_PROFILE_AAC_BSAC =22,
MFX_PROFILE_AAC_PS =29,
MFX_AUDIO_MPEG1_LAYER1 =0x00000110, 
MFX_AUDIO_MPEG1_LAYER2 =0x00000120,
MFX_AUDIO_MPEG1_LAYER3 =0x00000140,
MFX_AUDIO_MPEG2_LAYER1 =0x00000210,
MFX_AUDIO_MPEG2_LAYER2 =0x00000220,
MFX_AUDIO_MPEG2_LAYER3 =0x00000240
};
enum {
MFX_AUDIO_AAC_HE_DWNSMPL_OFF=0,
MFX_AUDIO_AAC_HE_DWNSMPL_ON= 1
};
enum {
MFX_AUDIO_AAC_PS_DISABLE= 0,
MFX_AUDIO_AAC_PS_PARSER= 1,
MFX_AUDIO_AAC_PS_ENABLE_BL= 111,
MFX_AUDIO_AAC_PS_ENABLE_UR= 411
};
enum {
MFX_AUDIO_AAC_SBR_DISABLE = 0,
MFX_AUDIO_AAC_SBR_ENABLE= 1,
MFX_AUDIO_AAC_SBR_UNDEF= 2
};
enum{
MFX_AUDIO_AAC_ADTS= 1,
MFX_AUDIO_AAC_ADIF= 2,
MFX_AUDIO_AAC_RAW= 3,
};
enum 
{
MFX_AUDIO_AAC_MONO= 0,
MFX_AUDIO_AAC_LR_STEREO= 1,
MFX_AUDIO_AAC_MS_STEREO= 2,
MFX_AUDIO_AAC_JOINT_STEREO= 3
};
typedef struct {
mfxU32 CodecId;
mfxU16 CodecProfile;
mfxU16 CodecLevel;
mfxU32 Bitrate;
mfxU32 SampleFrequency;
mfxU16 NumChannel;
mfxU16 BitPerSample;
mfxU16 reserved1[22]; 
union { 
struct { 
mfxU16 FlagPSSupportLev;
mfxU16 Layer;
mfxU16 AACHeaderDataSize;
mfxU8 AACHeaderData[64];
};
struct { 
mfxU16 OutputFormat;
mfxU16 StereoMode;
mfxU16 reserved2[61]; 
};
};
} mfxAudioInfoMFX;
typedef struct {
mfxU16 AsyncDepth;
mfxU16 Protected;
mfxU16 reserved[14]; 
mfxAudioInfoMFX mfx;
mfxExtBuffer** ExtParam;
mfxU16 NumExtParam;
} mfxAudioParam;
typedef struct {
mfxU32 SuggestedInputSize;
mfxU32 SuggestedOutputSize;
mfxU32 reserved[6];
} mfxAudioAllocRequest;
typedef struct {
mfxU64 TimeStamp; 
mfxU16 Locked;
mfxU16 NumChannels;
mfxU32 SampleFrequency;
mfxU16 BitPerSample;
mfxU16 reserved1[7]; 
mfxU8* Data;
mfxU32 reserved2;
mfxU32 DataLength;
mfxU32 MaxLength;
mfxU32 NumExtParam;
mfxExtBuffer **ExtParam;
} mfxAudioFrame;
#if defined(__cplusplus)
}
#endif 
