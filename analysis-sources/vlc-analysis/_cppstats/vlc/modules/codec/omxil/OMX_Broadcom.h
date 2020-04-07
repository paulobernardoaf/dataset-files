#include "OMX_Component.h"
#define OMX_BUFFERFLAG_TIME_UNKNOWN 0x00000100
#define OMX_BUFFERFLAG_CAPTURE_PREVIEW 0x00000200
#define OMX_BUFFERFLAG_ENDOFNAL 0x00000400
#define OMX_BUFFERFLAG_FRAGMENTLIST 0x00000800
#define OMX_BUFFERFLAG_DISCONTINUITY 0x00001000
#define OMX_BUFFERFLAG_CODECSIDEINFO 0x00002000
#if !defined(OMX_SKIP64BIT)
#define omx_ticks_from_s64(s) (s)
#define omx_ticks_to_s64(t) (t)
#else
static inline OMX_TICKS omx_ticks_from_s64(signed long long s) { OMX_TICKS t; t.nLowPart = (OMX_U32)s; t.nHighPart = (OMX_U32)(s>>32); return t; }
#define omx_ticks_to_s64(t) ((t).nLowPart | ((uint64_t)((t).nHighPart) << 32))
#endif 
typedef struct OMX_BUFFERFRAGMENTTYPE {
OMX_PTR pBuffer; 
OMX_U32 nLen; 
} OMX_BUFFERFRAGMENTTYPE;
typedef struct OMX_PARAM_IJGSCALINGTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnabled;
} OMX_PARAM_IJGSCALINGTYPE;
typedef enum OMX_DISPLAYTRANSFORMTYPE{
OMX_DISPLAY_ROT0 = 0,
OMX_DISPLAY_MIRROR_ROT0 = 1,
OMX_DISPLAY_MIRROR_ROT180 = 2,
OMX_DISPLAY_ROT180 = 3,
OMX_DISPLAY_MIRROR_ROT90 = 4,
OMX_DISPLAY_ROT270 = 5,
OMX_DISPLAY_ROT90 = 6,
OMX_DISPLAY_MIRROR_ROT270 = 7,
OMX_DISPLAY_DUMMY = 0x7FFFFFFF
} OMX_DISPLAYTRANSFORMTYPE;
typedef struct OMX_DISPLAYRECTTYPE {
OMX_S16 x_offset;
OMX_S16 y_offset;
OMX_S16 width;
OMX_S16 height;
} OMX_DISPLAYRECTTYPE;
typedef enum OMX_DISPLAYMODETYPE {
OMX_DISPLAY_MODE_FILL = 0,
OMX_DISPLAY_MODE_LETTERBOX = 1,
OMX_DISPLAY_MODE_DUMMY = 0x7FFFFFFF
} OMX_DISPLAYMODETYPE;
typedef enum OMX_DISPLAYSETTYPE {
OMX_DISPLAY_SET_NONE = 0,
OMX_DISPLAY_SET_NUM = 1,
OMX_DISPLAY_SET_FULLSCREEN = 2,
OMX_DISPLAY_SET_TRANSFORM = 4,
OMX_DISPLAY_SET_DEST_RECT = 8,
OMX_DISPLAY_SET_SRC_RECT = 0x10,
OMX_DISPLAY_SET_MODE = 0x20,
OMX_DISPLAY_SET_PIXEL = 0x40,
OMX_DISPLAY_SET_NOASPECT = 0x80,
OMX_DISPLAY_SET_LAYER = 0x100,
OMX_DISPLAY_SET_COPYPROTECT = 0x200,
OMX_DISPLAY_SET_ALPHA = 0x400,
OMX_DISPLAY_SET_DUMMY = 0x7FFFFFFF
} OMX_DISPLAYSETTYPE;
typedef struct OMX_CONFIG_DISPLAYREGIONTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_DISPLAYSETTYPE set;
OMX_U32 num;
OMX_BOOL fullscreen;
OMX_DISPLAYTRANSFORMTYPE transform;
OMX_DISPLAYRECTTYPE dest_rect;
OMX_DISPLAYRECTTYPE src_rect;
OMX_BOOL noaspect;
OMX_DISPLAYMODETYPE mode;
OMX_U32 pixel_x;
OMX_U32 pixel_y;
OMX_S32 layer;
OMX_BOOL copyprotect_required;
OMX_U32 alpha;
OMX_U32 wfc_context_width;
OMX_U32 wfc_context_height;
} OMX_CONFIG_DISPLAYREGIONTYPE;
typedef enum OMX_SOURCETYPE {
OMX_SOURCE_WHITE = 0, 
OMX_SOURCE_BLACK = 1, 
OMX_SOURCE_DIAGONAL = 2, 
OMX_SOURCE_NOISE = 3, 
OMX_SOURCE_RANDOM = 4, 
OMX_SOURCE_COLOUR = 5, 
OMX_SOURCE_BLOCKS = 6, 
OMX_SOURCE_SWIRLY, 
OMX_SOURCE_DUMMY = 0x7FFFFFFF
} OMX_SOURCETYPE;
typedef struct OMX_PARAM_SOURCETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_SOURCETYPE eType;
OMX_U32 nParam;
OMX_U32 nFrameCount;
OMX_U32 xFrameRate;
} OMX_PARAM_SOURCETYPE;
typedef struct OMX_PARAM_SOURCESEEDTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U16 nData[16];
} OMX_PARAM_SOURCESEEDTYPE;
typedef enum OMX_RESIZEMODETYPE {
OMX_RESIZE_NONE,
OMX_RESIZE_CROP,
OMX_RESIZE_BOX,
OMX_RESIZE_BYTES,
OMX_RESIZE_DUMMY = 0x7FFFFFFF
} OMX_RESIZEMODETYPE;
typedef struct OMX_PARAM_RESIZETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_RESIZEMODETYPE eMode;
OMX_U32 nMaxWidth;
OMX_U32 nMaxHeight;
OMX_U32 nMaxBytes;
OMX_BOOL bPreserveAspectRatio;
OMX_BOOL bAllowUpscaling;
} OMX_PARAM_RESIZETYPE;
typedef struct OMX_PARAM_TESTINTERFACETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bTest;
OMX_BOOL bSetExtra;
OMX_U32 nExtra;
OMX_BOOL bSetError;
OMX_BOOL stateError[2];
} OMX_PARAM_TESTINTERFACETYPE;
typedef struct OMX_CONFIG_VISUALISATIONTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U8 name[16];
OMX_U8 property[64];
} OMX_CONFIG_VISUALISATIONTYPE;
typedef struct OMX_CONFIG_BRCMAUDIODESTINATIONTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U8 sName[16];
} OMX_CONFIG_BRCMAUDIODESTINATIONTYPE;
typedef struct OMX_CONFIG_BRCMAUDIOSOURCETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U8 sName[16];
} OMX_CONFIG_BRCMAUDIOSOURCETYPE;
typedef struct OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 coeff[16];
} OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS;
typedef enum OMX_PLAYMODETYPE {
OMX_PLAYMODE_NORMAL,
OMX_PLAYMODE_FF,
OMX_PLAYMODE_REW,
OMX_PLAYMODE_DUMMY = 0x7FFFFFFF
} OMX_PLAYMODETYPE;
typedef struct OMX_CONFIG_PLAYMODETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_PLAYMODETYPE eMode;
} OMX_CONFIG_PLAYMODETYPE;
typedef enum OMX_DELIVERYFORMATTYPE {
OMX_DELIVERYFORMAT_STREAM, 
OMX_DELIVERYFORMAT_SINGLE_PACKET, 
OMX_DELIVERYFORMAT_DUMMY = 0x7FFFFFFF
} OMX_DELIVERYFORMATTYPE;
typedef struct OMX_PARAM_DELIVERYFORMATTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_DELIVERYFORMATTYPE eFormat;
} OMX_PARAM_DELIVERYFORMATTYPE;
typedef struct OMX_PARAM_CODECCONFIGTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 bCodecConfigIsComplete;
OMX_U8 nData[1];
} OMX_PARAM_CODECCONFIGTYPE;
typedef struct OMX_PARAM_STILLSFUNCTIONTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bBuffer;
OMX_PTR (*pOpenFunc)(void);
OMX_PTR (*pCloseFunc)(void);
OMX_PTR (*pReadFunc)(void);
OMX_PTR (*pSeekFunc)(void);
OMX_PTR (*pWriteFunc)(void);
} OMX_PARAM_STILLSFUNCTIONTYPE;
typedef void* OMX_BUFFERADDRESSHANDLETYPE;
typedef struct OMX_PARAM_BUFFERADDRESSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nAllocLen;
OMX_BUFFERADDRESSHANDLETYPE handle;
} OMX_PARAM_BUFFERADDRESSTYPE;
typedef struct OMX_PARAM_TUNNELSETUPTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_TUNNELSETUPTYPE sSetup;
} OMX_PARAM_TUNNELSETUPTYPE;
typedef struct OMX_PARAM_BRCMPORTEGLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bPortIsEGL;
} OMX_PARAM_BRCMPORTEGLTYPE;
typedef struct OMX_CONFIG_IMAGEFILTERPARAMSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_IMAGEFILTERTYPE eImageFilter;
OMX_U32 nNumParams;
OMX_U32 nParams[5];
} OMX_CONFIG_IMAGEFILTERPARAMSTYPE;
typedef struct OMX_CONFIG_TRANSITIONCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nPosStart;
OMX_U32 nPosEnd;
OMX_S32 nPosIncrement;
OMX_TICKS nFrameIncrement;
OMX_BOOL bSwapInputs;
OMX_U8 name[16];
OMX_U8 property[64];
} OMX_CONFIG_TRANSITIONCONTROLTYPE;
typedef enum OMX_AUDIOMONOTRACKOPERATIONSTYPE {
OMX_AUDIOMONOTRACKOPERATIONS_NOP,
OMX_AUDIOMONOTRACKOPERATIONS_L_TO_R,
OMX_AUDIOMONOTRACKOPERATIONS_R_TO_L,
OMX_AUDIOMONOTRACKOPERATIONS_DUMMY = 0x7FFFFFFF
} OMX_AUDIOMONOTRACKOPERATIONSTYPE ;
typedef struct OMX_CONFIG_AUDIOMONOTRACKCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_AUDIOMONOTRACKOPERATIONSTYPE eMode;
} OMX_CONFIG_AUDIOMONOTRACKCONTROLTYPE;
typedef enum OMX_CAMERAIMAGEPOOLINPUTMODETYPE {
OMX_CAMERAIMAGEPOOLINPUTMODE_ONEPOOL, 
OMX_CAMERAIMAGEPOOLINPUTMODE_TWOPOOLS, 
} OMX_CAMERAIMAGEPOOLINPUTMODETYPE;
typedef struct OMX_PARAM_CAMERAIMAGEPOOLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nNumHiResVideoFrames;
OMX_U32 nHiResVideoWidth;
OMX_U32 nHiResVideoHeight;
OMX_COLOR_FORMATTYPE eHiResVideoType;
OMX_U32 nNumHiResStillsFrames;
OMX_U32 nHiResStillsWidth;
OMX_U32 nHiResStillsHeight;
OMX_COLOR_FORMATTYPE eHiResStillsType;
OMX_U32 nNumLoResFrames;
OMX_U32 nLoResWidth;
OMX_U32 nLoResHeight;
OMX_COLOR_FORMATTYPE eLoResType;
OMX_U32 nNumSnapshotFrames;
OMX_COLOR_FORMATTYPE eSnapshotType;
OMX_CAMERAIMAGEPOOLINPUTMODETYPE eInputPoolMode;
OMX_U32 nNumInputVideoFrames;
OMX_U32 nInputVideoWidth;
OMX_U32 nInputVideoHeight;
OMX_COLOR_FORMATTYPE eInputVideoType;
OMX_U32 nNumInputStillsFrames;
OMX_U32 nInputStillsWidth;
OMX_U32 nInputStillsHeight;
OMX_COLOR_FORMATTYPE eInputStillsType;
} OMX_PARAM_CAMERAIMAGEPOOLTYPE;
typedef struct OMX_PARAM_IMAGEPOOLSIZETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 width;
OMX_U32 height;
OMX_U32 num_pages;
} OMX_PARAM_IMAGEPOOLSIZETYPE;
struct opaque_vc_pool_s;
typedef struct opaque_vc_pool_s OMX_BRCM_POOL_T;
typedef struct OMX_PARAM_IMAGEPOOLEXTERNALTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BRCM_POOL_T *image_pool;
OMX_BRCM_POOL_T *image_pool2;
OMX_BRCM_POOL_T *image_pool3;
OMX_BRCM_POOL_T *image_pool4;
OMX_BRCM_POOL_T *image_pool5;
} OMX_PARAM_IMAGEPOOLEXTERNALTYPE;
struct _IL_FIFO_T;
typedef struct OMX_PARAM_RUTILFIFOINFOTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
struct _IL_FIFO_T *pILFifo;
} OMX_PARAM_RUTILFIFOINFOTYPE;
typedef struct OMX_PARAM_ILFIFOCONFIG {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nDataSize; 
OMX_U32 nHeaderCount; 
} OMX_PARAM_ILFIFOCONFIG;
typedef struct OMX_CONFIG_CAMERASENSORMODETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nModeIndex;
OMX_U32 nNumModes;
OMX_U32 nWidth;
OMX_U32 nHeight;
OMX_U32 nPaddingRight;
OMX_U32 nPaddingDown;
OMX_COLOR_FORMATTYPE eColorFormat;
OMX_U32 nFrameRateMax;
OMX_U32 nFrameRateMin;
} OMX_CONFIG_CAMERASENSORMODETYPE;
typedef struct OMX_BRCMBUFFERSTATSTYPE {
OMX_U32 nOrdinal;
OMX_TICKS nTimeStamp;
OMX_U32 nFilledLen;
OMX_U32 nFlags;
union
{
OMX_U32 nU32;
struct
{
OMX_U32 nYpart;
OMX_U32 nUVpart;
} image;
} crc;
} OMX_BRCMBUFFERSTATSTYPE;
typedef struct OMX_CONFIG_BRCMPORTBUFFERSTATSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nCount;
OMX_BRCMBUFFERSTATSTYPE sData[1];
} OMX_CONFIG_BRCMPORTBUFFERSTATSTYPE;
typedef struct OMX_CONFIG_BRCMPORTSTATSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nImageCount;
OMX_U32 nBufferCount;
OMX_U32 nFrameCount;
OMX_U32 nFrameSkips;
OMX_U32 nDiscards;
OMX_U32 nEOS;
OMX_U32 nMaxFrameSize;
OMX_TICKS nByteCount;
OMX_TICKS nMaxTimeDelta;
OMX_U32 nCorruptMBs; 
} OMX_CONFIG_BRCMPORTSTATSTYPE;
typedef struct OMX_CONFIG_BRCMCAMERASTATSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nOutFrameCount;
OMX_U32 nDroppedFrameCount;
} OMX_CONFIG_BRCMCAMERASTATSTYPE;
typedef struct OMX_CONFIG_BRCMCAMERASTATSTYPE OMX_CONFIG_BRCMCAMERASTATS;
#define OMX_BRCM_MAXIOPERFBANDS 10
typedef struct {
OMX_U32 count[OMX_BRCM_MAXIOPERFBANDS];
OMX_U32 num[OMX_BRCM_MAXIOPERFBANDS];
} OMX_BRCM_PERFSTATS;
typedef struct OMX_CONFIG_BRCMIOPERFSTATSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bEnabled; 
OMX_BRCM_PERFSTATS write; 
OMX_BRCM_PERFSTATS flush; 
OMX_BRCM_PERFSTATS wait; 
} OMX_CONFIG_BRCMIOPERFSTATSTYPE;
typedef struct OMX_CONFIG_SHARPNESSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_S32 nSharpness;
} OMX_CONFIG_SHARPNESSTYPE;
typedef enum OMX_COMMONFLICKERCANCELTYPE {
OMX_COMMONFLICKERCANCEL_OFF,
OMX_COMMONFLICKERCANCEL_AUTO,
OMX_COMMONFLICKERCANCEL_50,
OMX_COMMONFLICKERCANCEL_60,
OMX_COMMONFLICKERCANCEL_DUMMY = 0x7FFFFFFF
} OMX_COMMONFLICKERCANCELTYPE;
typedef struct OMX_CONFIG_FLICKERCANCELTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_COMMONFLICKERCANCELTYPE eFlickerCancel;
} OMX_CONFIG_FLICKERCANCELTYPE;
typedef enum OMX_REDEYEREMOVALTYPE {
OMX_RedEyeRemovalNone, 
OMX_RedEyeRemovalOn, 
OMX_RedEyeRemovalAuto, 
OMX_RedEyeRemovalKhronosExtensions = 0x6F000000, 
OMX_RedEyeRemovalVendorStartUnused = 0x7F000000, 
OMX_RedEyeRemovalSimple, 
OMX_RedEyeRemovalMax = 0x7FFFFFFF
} OMX_REDEYEREMOVALTYPE;
typedef struct OMX_CONFIG_REDEYEREMOVALTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_REDEYEREMOVALTYPE eMode;
} OMX_CONFIG_REDEYEREMOVALTYPE;
typedef enum OMX_FACEDETECTIONCONTROLTYPE {
OMX_FaceDetectionControlNone, 
OMX_FaceDetectionControlOn, 
OMX_FaceDetectionControlKhronosExtensions = 0x6F000000, 
OMX_FaceDetectionControlVendorStartUnused = 0x7F000000, 
OMX_FaceDetectionControlMax = 0x7FFFFFFF
} OMX_FACEDETECTIONCONTROLTYPE;
typedef struct OMX_CONFIG_FACEDETECTIONCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_FACEDETECTIONCONTROLTYPE eMode;
OMX_U32 nFrames; 
OMX_U32 nMaxRegions; 
OMX_U32 nQuality; 
} OMX_CONFIG_FACEDETECTIONCONTROLTYPE;
typedef enum OMX_FACEREGIONFLAGSTYPE {
OMX_FaceRegionFlagsNone = 0,
OMX_FaceRegionFlagsBlink = 1,
OMX_FaceRegionFlagsSmile = 2,
OMX_FaceRegionFlagsKhronosExtensions = 0x6F000000, 
OMX_FaceRegionFlagsVendorStartUnused = 0x7F000000, 
OMX_FaceRegionFlagsMax = 0x7FFFFFFF
} OMX_FACEREGIONFLAGSTYPE;
typedef struct OMX_FACEREGIONTYPE {
OMX_S16 nLeft; 
OMX_S16 nTop; 
OMX_U16 nWidth; 
OMX_U16 nHeight; 
OMX_FACEREGIONFLAGSTYPE nFlags; 
#if !defined(OMX_SKIP64BIT)
OMX_U64 nFaceRecognitionId; 
#else
struct
{
OMX_U32 nLowPart; 
OMX_U32 nHighPart; 
} nFaceRecognitionId;
#endif
} OMX_FACEREGIONTYPE;
typedef struct OMX_CONFIG_FACEDETECTIONREGIONTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex; 
OMX_U32 nIndex; 
OMX_U32 nDetectedRegions; 
OMX_S32 nValidRegions; 
OMX_U32 nImageWidth; 
OMX_U32 nImageHeight; 
OMX_FACEREGIONTYPE sRegion[1]; 
} OMX_CONFIG_FACEDETECTIONREGIONTYPE;
typedef enum OMX_INTERLACETYPE {
OMX_InterlaceProgressive, 
OMX_InterlaceFieldSingleUpperFirst, 
OMX_InterlaceFieldSingleLowerFirst, 
OMX_InterlaceFieldsInterleavedUpperFirst, 
OMX_InterlaceFieldsInterleavedLowerFirst, 
OMX_InterlaceMixed, 
OMX_InterlaceKhronosExtensions = 0x6F000000, 
OMX_InterlaceVendorStartUnused = 0x7F000000, 
OMX_InterlaceMax = 0x7FFFFFFF
} OMX_INTERLACETYPE;
typedef struct OMX_CONFIG_INTERLACETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex; 
OMX_INTERLACETYPE eMode; 
OMX_BOOL bRepeatFirstField; 
} OMX_CONFIG_INTERLACETYPE;
typedef struct OMX_PARAM_CAMERAISPTUNERTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U8 tuner_name[64];
} OMX_PARAM_CAMERAISPTUNERTYPE;
typedef struct OMX_CONFIG_IMAGEPTRTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_PTR pImage;
} OMX_CONFIG_IMAGEPTRTYPE;
typedef enum OMX_AFASSISTTYPE {
OMX_AFAssistAuto,
OMX_AFAssistOn,
OMX_AFAssistOff,
OMX_AFAssistTorch,
OMX_AFAssistKhronosExtensions = 0x6F000000,
OMX_AFAssistVendorStartUnused = 0x7F000000,
OMX_AFAssistMax = 0x7FFFFFFF
} OMX_AFASSISTTYPE;
typedef struct OMX_CONFIG_AFASSISTTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_AFASSISTTYPE eMode;
} OMX_CONFIG_AFASSISTTYPE;
typedef struct OMX_CONFIG_INPUTCROPTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 xLeft; 
OMX_U32 xTop; 
OMX_U32 xWidth; 
OMX_U32 xHeight; 
} OMX_CONFIG_INPUTCROPTYPE;
typedef struct OMX_PARAM_CODECREQUIREMENTSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nCallbackID;
OMX_BOOL bTryHWCodec;
} OMX_PARAM_CODECREQUIREMENTSTYPE;
typedef struct OMX_CONFIG_BRCMEGLIMAGEMEMHANDLETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_PTR eglImage;
OMX_PTR memHandle;
} OMX_CONFIG_BRCMEGLIMAGEMEMHANDLETYPE;
typedef enum OMX_PRIVACYINDICATORTYPE {
OMX_PrivacyIndicatorOff,
OMX_PrivacyIndicatorOn,
OMX_PrivacyIndicatorForceOn,
OMX_PrivacyIndicatorKhronosExtensions = 0x6F000000,
OMX_PrivacyIndicatorVendorStartUnused = 0x7F000000,
OMX_PrivacyIndicatorMax = 0x7FFFFFFF
} OMX_PRIVACYINDICATORTYPE;
typedef struct OMX_CONFIG_PRIVACYINDICATORTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_PRIVACYINDICATORTYPE ePrivacyIndicatorMode;
} OMX_CONFIG_PRIVACYINDICATORTYPE;
typedef enum OMX_CAMERAFLASHTYPE {
OMX_CameraFlashDefault,
OMX_CameraFlashXenon,
OMX_CameraFlashLED,
OMX_CameraFlashNone,
OMX_CameraFlashKhronosExtensions = 0x6F000000,
OMX_CameraFlashVendorStartUnused = 0x7F000000,
OMX_CameraFlashMax = 0x7FFFFFFF
} OMX_CAMERAFLASHTYPE;
typedef struct OMX_PARAM_CAMERAFLASHTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_CAMERAFLASHTYPE eFlashType;
OMX_BOOL bRedEyeUsesTorchMode;
} OMX_PARAM_CAMERAFLASHTYPE;
typedef enum OMX_CAMERAFLASHCONFIGSYNCTYPE {
OMX_CameraFlashConfigSyncFrontSlow,
OMX_CameraFlashConfigSyncRearSlow,
OMX_CameraFlashConfigSyncFrontFast,
OMX_CameraFlashConfigSyncKhronosExtensions = 0x6F000000,
OMX_CameraFlashConfigSyncVendorStartUnused = 0x7F000000,
OMX_CameraFlashConfigSyncMax = 0x7FFFFFFF
} OMX_CAMERAFLASHCONFIGSYNCTYPE;
typedef struct OMX_CONFIG_CAMERAFLASHCONFIGTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bUsePreFlash;
OMX_BOOL bUseFocusDistanceInfo;
OMX_CAMERAFLASHCONFIGSYNCTYPE eFlashSync;
OMX_BOOL bIgnoreChargeState;
} OMX_CONFIG_CAMERAFLASHCONFIGTYPE;
typedef struct OMX_CONFIG_BRCMAUDIOTRACKGAPLESSPLAYBACKTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nDelay; 
OMX_U32 nPadding; 
} OMX_CONFIG_BRCMAUDIOTRACKGAPLESSPLAYBACKTYPE;
typedef struct OMX_CONFIG_BRCMAUDIOTRACKCHANGECONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nSrcPortIndex;
OMX_U32 nDstPortIndex;
OMX_U32 nXFade;
} OMX_CONFIG_BRCMAUDIOTRACKCHANGECONTROLTYPE;
typedef enum OMX_BRCMPIXELVALUERANGETYPE
{
OMX_PixelValueRangeUnspecified = 0,
OMX_PixelValueRangeITU_R_BT601,
OMX_PixelValueRangeFull8Bit,
OMX_PixelValueRangeKhronosExtensions = 0x6F000000, 
OMX_PixelValueRangeVendorStartUnused = 0x7F000000, 
OMX_PixelValueRangeMax = 0x7FFFFFFF
} OMX_BRCMPIXELVALUERANGETYPE;
typedef struct OMX_PARAM_BRCMPIXELVALUERANGETYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BRCMPIXELVALUERANGETYPE ePixelValueRange;
} OMX_PARAM_BRCMPIXELVALUERANGETYPE;
typedef enum OMX_CAMERADISABLEALGORITHMTYPE {
OMX_CameraDisableAlgorithmFacetracking,
OMX_CameraDisableAlgorithmRedEyeReduction,
OMX_CameraDisableAlgorithmVideoStabilisation,
OMX_CameraDisableAlgorithmWriteRaw,
OMX_CameraDisableAlgorithmVideoDenoise,
OMX_CameraDisableAlgorithmStillsDenoise,
OMX_CameraDisableAlgorithmAntiShake,
OMX_CameraDisableAlgorithmImageEffects,
OMX_CameraDisableAlgorithmDarkSubtract,
OMX_CameraDisableAlgorithmDynamicRangeExpansion,
OMX_CameraDisableAlgorithmFaceRecognition,
OMX_CameraDisableAlgorithmFaceBeautification,
OMX_CameraDisableAlgorithmSceneDetection,
OMX_CameraDisableAlgorithmHighDynamicRange,
OMX_CameraDisableAlgorithmKhronosExtensions = 0x6F000000, 
OMX_CameraDisableAlgorithmVendorStartUnused = 0x7F000000, 
OMX_CameraDisableAlgorithmMax = 0x7FFFFFFF
} OMX_CAMERADISABLEALGORITHMTYPE;
typedef struct OMX_PARAM_CAMERADISABLEALGORITHMTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_CAMERADISABLEALGORITHMTYPE eAlgorithm;
OMX_BOOL bDisabled;
} OMX_PARAM_CAMERADISABLEALGORITHMTYPE;
typedef struct OMX_CONFIG_BRCMAUDIOEFFECTCONTROLTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnable;
OMX_U8 name[16];
OMX_U8 property[256];
} OMX_CONFIG_BRCMAUDIOEFFECTCONTROLTYPE;
typedef struct OMX_CONFIG_BRCMMINIMUMPROCESSINGLATENCY {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_TICKS nOffset;
} OMX_CONFIG_BRCMMINIMUMPROCESSINGLATENCY;
typedef struct OMX_PARAM_BRCMVIDEOAVCSEIENABLETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnable;
} OMX_PARAM_BRCMVIDEOAVCSEIENABLETYPE;
typedef struct OMX_PARAM_BRCMALLOWMEMCHANGETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bEnable;
} OMX_PARAM_BRCMALLOWMEMCHANGETYPE;
typedef enum OMX_CONFIG_CAMERAUSECASE {
OMX_CameraUseCaseAuto,
OMX_CameraUseCaseVideo,
OMX_CameraUseCaseStills,
OMX_CameraUseCaseKhronosExtensions = 0x6F000000, 
OMX_CameraUseCaseVendorStartUnused = 0x7F000000, 
OMX_CameraUseCaseMax = 0x7FFFFFFF
} OMX_CONFIG_CAMERAUSECASE;
typedef struct OMX_CONFIG_CAMERAUSECASETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_CONFIG_CAMERAUSECASE eUseCase;
} OMX_CONFIG_CAMERAUSECASETYPE;
typedef struct OMX_PARAM_BRCMDISABLEPROPRIETARYTUNNELSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bUseBuffers;
} OMX_PARAM_BRCMDISABLEPROPRIETARYTUNNELSTYPE;
typedef struct OMX_PARAM_BRCMRETAINMEMORYTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bEnable;
} OMX_PARAM_BRCMRETAINMEMORYTYPE;
typedef struct OMX_PARAM_BRCMOUTPUTBUFFERSIZETYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nBufferSize;
} OMX_PARAM_BRCMOUTPUTBUFFERSIZETYPE;
#define OMX_CONFIG_CAMERAINFOTYPE_NAME_LEN 16
typedef struct OMX_CONFIG_LENSCALIBRATIONVALUETYPE
{
OMX_U16 nShutterDelayTime;
OMX_U16 nNdTransparency;
OMX_U16 nPwmPulseNearEnd; 
OMX_U16 nPwmPulseFarEnd; 
OMX_U16 nVoltagePIOutNearEnd[3];
OMX_U16 nVoltagePIOut10cm[3];
OMX_U16 nVoltagePIOutInfinity[3];
OMX_U16 nVoltagePIOutFarEnd[3];
OMX_U32 nAdcConversionNearEnd;
OMX_U32 nAdcConversionFarEnd;
} OMX_CONFIG_LENSCALIBRATIONVALUETYPE;
#define OMX_CONFIG_CAMERAINFOTYPE_NAME_LEN 16
#define OMX_CONFIG_CAMERAINFOTYPE_SERIALNUM_LEN 20
#define OMX_CONFIG_CAMERAINFOTYPE_EPROMVER_LEN 8
typedef struct OMX_CONFIG_CAMERAINFOTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U8 cameraname[OMX_CONFIG_CAMERAINFOTYPE_NAME_LEN];
OMX_U8 lensname[OMX_CONFIG_CAMERAINFOTYPE_NAME_LEN];
OMX_U16 nModelId;
OMX_U8 nManufacturerId;
OMX_U8 nRevNum;
OMX_U8 sSerialNumber[OMX_CONFIG_CAMERAINFOTYPE_SERIALNUM_LEN];
OMX_U8 sEpromVersion[OMX_CONFIG_CAMERAINFOTYPE_EPROMVER_LEN];
OMX_CONFIG_LENSCALIBRATIONVALUETYPE sLensCalibration;
OMX_U32 xFNumber;
OMX_U32 xFocalLength;
} OMX_CONFIG_CAMERAINFOTYPE;
typedef enum OMX_CONFIG_CAMERAFEATURESSHUTTER {
OMX_CameraFeaturesShutterUnknown,
OMX_CameraFeaturesShutterNotPresent,
OMX_CameraFeaturesShutterPresent,
OMX_CameraFeaturesShutterKhronosExtensions = 0x6F000000, 
OMX_CameraFeaturesShutterVendorStartUnused = 0x7F000000, 
OMX_CameraFeaturesShutterMax = 0x7FFFFFFF
} OMX_CONFIG_CAMERAFEATURESSHUTTER;
typedef struct OMX_CONFIG_CAMERAFEATURESTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_CONFIG_CAMERAFEATURESSHUTTER eHasMechanicalShutter;
OMX_BOOL bHasLens;
} OMX_CONFIG_CAMERAFEATURESTYPE;
typedef struct OMX_CONFIG_REQUESTCALLBACKTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_INDEXTYPE nIndex;
OMX_BOOL bEnable;
} OMX_CONFIG_REQUESTCALLBACKTYPE;
typedef enum OMX_FOCUSREGIONTYPE {
OMX_FocusRegionNormal,
OMX_FocusRegionFace,
OMX_FocusRegionMax
} OMX_FOCUSREGIONTYPE;
typedef struct OMX_FOCUSREGIONXY {
OMX_U32 xLeft;
OMX_U32 xTop;
OMX_U32 xWidth;
OMX_U32 xHeight;
OMX_U32 nWeight;
OMX_U32 nMask;
OMX_FOCUSREGIONTYPE eType;
} OMX_FOCUSREGIONXY;
typedef struct OMX_CONFIG_FOCUSREGIONXYTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nIndex;
OMX_U32 nTotalRegions;
OMX_S32 nValidRegions;
OMX_BOOL bLockToFaces;
OMX_U32 xFaceTolerance;
OMX_FOCUSREGIONXY sRegion[1];
} OMX_CONFIG_FOCUSREGIONXYTYPE;
typedef struct OMX_CONFIG_U8TYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U8 nU8; 
} OMX_PARAM_U8TYPE;
typedef struct OMX_CONFIG_CAMERASETTINGSTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nExposure;
OMX_U32 nAnalogGain;
OMX_U32 nDigitalGain;
OMX_U32 nLux;
OMX_U32 nRedGain;
OMX_U32 nBlueGain;
OMX_U32 nFocusPosition;
} OMX_CONFIG_CAMERASETTINGSTYPE;
typedef struct OMX_YUVCOLOUR {
OMX_U8 nY;
OMX_U8 nU;
OMX_U8 nV;
} OMX_YUVCOLOUR;
typedef struct OMX_CONFIG_DRAWBOXLINEPARAMS {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U32 xCornerSize; 
OMX_U32 nPrimaryFaceLineWidth; 
OMX_U32 nOtherFaceLineWidth; 
OMX_U32 nFocusRegionLineWidth; 
OMX_YUVCOLOUR sPrimaryFaceColour; 
OMX_YUVCOLOUR sPrimaryFaceSmileColour; 
OMX_YUVCOLOUR sPrimaryFaceBlinkColour; 
OMX_YUVCOLOUR sOtherFaceColour; 
OMX_YUVCOLOUR sOtherFaceSmileColour; 
OMX_YUVCOLOUR sOtherFaceBlinkColour; 
OMX_BOOL bShowFocusRegionsWhenIdle; 
OMX_YUVCOLOUR sFocusRegionColour; 
OMX_BOOL bShowAfState; 
OMX_BOOL bShowOnlyPrimaryAfState; 
OMX_BOOL bCombineNonFaceRegions; 
OMX_YUVCOLOUR sAfLockPrimaryFaceColour; 
OMX_YUVCOLOUR sAfLockOtherFaceColour; 
OMX_YUVCOLOUR sAfLockFocusRegionColour; 
OMX_YUVCOLOUR sAfFailPrimaryFaceColour; 
OMX_YUVCOLOUR sAfFailOtherFaceColour; 
OMX_YUVCOLOUR sAfFailFocusRegionColour; 
} OMX_CONFIG_DRAWBOXLINEPARAMS;
#define OMX_PARAM_CAMERARMITYPE_RMINAME_LEN 16
typedef struct OMX_PARAM_CAMERARMITYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bEnabled;
OMX_U8 sRmiName[OMX_PARAM_CAMERARMITYPE_RMINAME_LEN];
OMX_U32 nInputBufferHeight;
OMX_U32 nRmiBufferSize;
OMX_BRCM_POOL_T *pImagePool;
} OMX_PARAM_CAMERARMITYPE;
typedef struct OMX_CONFIG_BRCMSYNCOUTPUTTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
} OMX_CONFIG_BRCMSYNCOUTPUTTYPE;
typedef struct OMX_CONFIG_DRMVIEWTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nCurrentView; 
OMX_U32 nMaxView; 
} OMX_CONFIG_DRMVIEWTYPE;
typedef struct OMX_PARAM_BRCMU64TYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U32 nLowPart; 
OMX_U32 nHighPart; 
} OMX_PARAM_BRCMU64TYPE;
typedef struct OMX_PARAM_BRCMTHUMBNAILTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_BOOL bEnable; 
OMX_BOOL bUsePreview; 
OMX_U32 nWidth; 
OMX_U32 nHeight; 
} OMX_PARAM_BRCMTHUMBNAILTYPE;
typedef struct OMX_PARAM_BRCMASPECTRATIOTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nWidth;
OMX_U32 nHeight;
} OMX_PARAM_BRCMASPECTRATIOTYPE;
typedef struct OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BOOL bStartWithValidFrame; 
} OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE;
#define OMX_CONFIG_FLASHINFOTYPE_NAME_LEN 16
typedef struct OMX_CONFIG_FLASHINFOTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U8 sFlashName[OMX_CONFIG_FLASHINFOTYPE_NAME_LEN];
OMX_CAMERAFLASHTYPE eFlashType;
OMX_U8 nDeviceId;
OMX_U8 nDeviceVersion;
} OMX_CONFIG_FLASHINFOTYPE;
typedef enum OMX_DYNAMICRANGEEXPANSIONMODETYPE {
OMX_DynRangeExpOff,
OMX_DynRangeExpLow,
OMX_DynRangeExpMedium,
OMX_DynRangeExpHigh,
OMX_DynRangeExpKhronosExtensions = 0x6F000000,
OMX_DynRangeExpVendorStartUnused = 0x7F000000,
OMX_DynRangeExpMax = 0x7FFFFFFF
} OMX_DYNAMICRANGEEXPANSIONMODETYPE;
typedef struct OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_DYNAMICRANGEEXPANSIONMODETYPE eMode;
} OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE;
typedef enum OMX_BRCMTHREADAFFINITYTYPE {
OMX_BrcmThreadAffinityCPU0,
OMX_BrcmThreadAffinityCPU1,
OMX_BrcmThreadAffinityMax = 0x7FFFFFFF
} OMX_BRCMTHREADAFFINITYTYPE;
typedef struct OMX_PARAM_BRCMTHREADAFFINITYTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_BRCMTHREADAFFINITYTYPE eAffinity; 
} OMX_PARAM_BRCMTHREADAFFINITYTYPE;
typedef enum OMX_SCENEDETECTTYPE {
OMX_SceneDetectUnknown,
OMX_SceneDetectLandscape,
OMX_SceneDetectPortrait,
OMX_SceneDetectMacro,
OMX_SceneDetectNight,
OMX_SceneDetectPortraitNight,
OMX_SceneDetectBacklit,
OMX_SceneDetectPortraitBacklit,
OMX_SceneDetectSunset,
OMX_SceneDetectBeach,
OMX_SceneDetectSnow,
OMX_SceneDetectFireworks,
OMX_SceneDetectMax = 0x7FFFFFFF
} OMX_SCENEDETECTTYPE;
typedef struct OMX_CONFIG_SCENEDETECTTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_SCENEDETECTTYPE eScene; 
} OMX_CONFIG_SCENEDETECTTYPE;
typedef enum OMX_INDEXEXTTYPE {
OMX_IndexExtVideoStartUnused = OMX_IndexKhronosExtensions + 0x00600000,
OMX_IndexParamNalStreamFormatSupported, 
OMX_IndexParamNalStreamFormat, 
OMX_IndexParamNalStreamFormatSelect, 
OMX_IndexExtMax = 0x7FFFFFFF
} OMX_INDEXEXTTYPE;
typedef enum OMX_NALUFORMATSTYPE {
OMX_NaluFormatStartCodes = 1,
OMX_NaluFormatOneNaluPerBuffer = 2,
OMX_NaluFormatOneByteInterleaveLength = 4,
OMX_NaluFormatTwoByteInterleaveLength = 8,
OMX_NaluFormatFourByteInterleaveLength = 16,
OMX_NaluFormatCodingMax = 0x7FFFFFFF
} OMX_NALUFORMATSTYPE;
typedef struct OMX_NALSTREAMFORMATTYPE{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_NALUFORMATSTYPE eNaluFormat;
} OMX_NALSTREAMFORMATTYPE;
typedef struct OMX_VIDEO_PARAM_AVCTYPE OMX_VIDEO_PARAM_MVCTYPE;
typedef enum OMX_STATICBOXTYPE {
OMX_StaticBoxNormal,
OMX_StaticBoxPrimaryFaceAfIdle,
OMX_StaticBoxNonPrimaryFaceAfIdle,
OMX_StaticBoxFocusRegionAfIdle,
OMX_StaticBoxPrimaryFaceAfSuccess,
OMX_StaticBoxNonPrimaryFaceAfSuccess,
OMX_StaticBoxFocusRegionAfSuccess,
OMX_StaticBoxPrimaryFaceAfFail,
OMX_StaticBoxNonPrimaryFaceAfFail,
OMX_StaticBoxFocusRegionAfFail,
OMX_StaticBoxMax
} OMX_STATICBOXTYPE;
typedef struct OMX_STATICBOX {
OMX_U32 xLeft;
OMX_U32 xTop;
OMX_U32 xWidth;
OMX_U32 xHeight;
OMX_STATICBOXTYPE eType;
} OMX_STATICBOX;
typedef struct OMX_CONFIG_STATICBOXTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nIndex;
OMX_U32 nTotalBoxes;
OMX_S32 nValidBoxes;
OMX_BOOL bDrawOtherBoxes;
OMX_STATICBOX sBoxes[1];
} OMX_CONFIG_STATICBOXTYPE;
typedef struct OMX_CONFIG_PORTBOOLEANTYPE{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnabled;
} OMX_CONFIG_PORTBOOLEANTYPE;
typedef enum OMX_CAMERACAPTUREMODETYPE {
OMX_CameraCaptureModeWaitForCaptureEnd,
OMX_CameraCaptureModeWaitForCaptureEndAndUsePreviousInputImage,
OMX_CameraCaptureModeResumeViewfinderImmediately,
OMX_CameraCaptureModeMax,
} OMX_CAMERACAPTUREMODETYPE;
typedef struct OMX_PARAM_CAMERACAPTUREMODETYPE{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_CAMERACAPTUREMODETYPE eMode;
} OMX_PARAM_CAMERACAPTUREMODETYPE;
typedef enum OMX_BRCMDRMENCRYPTIONTYPE
{
OMX_DrmEncryptionNone = 0,
OMX_DrmEncryptionHdcp2,
OMX_DrmEncryptionKhronosExtensions = 0x6F000000, 
OMX_DrmEncryptionVendorStartUnused = 0x7F000000, 
OMX_DrmEncryptionRangeMax = 0x7FFFFFFF
} OMX_BRCMDRMENCRYPTIONTYPE;
typedef struct OMX_PARAM_BRCMDRMENCRYPTIONTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BRCMDRMENCRYPTIONTYPE eEncryption;
OMX_U32 nConfigDataLen;
OMX_U8 configData[1];
} OMX_PARAM_BRCMDRMENCRYPTIONTYPE;
typedef struct OMX_CONFIG_BUFFERSTALLTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bStalled; 
OMX_U32 nDelay; 
} OMX_CONFIG_BUFFERSTALLTYPE;
typedef struct OMX_CONFIG_LATENCYTARGETTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnabled; 
OMX_U32 nFilter; 
OMX_U32 nTarget; 
OMX_U32 nShift; 
OMX_S32 nSpeedFactor; 
OMX_S32 nInterFactor; 
OMX_S32 nAdjCap; 
} OMX_CONFIG_LATENCYTARGETTYPE;
typedef struct OMX_CONFIG_BRCMUSEPROPRIETARYCALLBACKTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_BOOL bEnable;
} OMX_CONFIG_BRCMUSEPROPRIETARYCALLBACKTYPE;
typedef enum OMX_TIMESTAMPMODETYPE
{
OMX_TimestampModeZero = 0, 
OMX_TimestampModeRawStc, 
OMX_TimestampModeResetStc, 
OMX_TimestampModeKhronosExtensions = 0x6F000000, 
OMX_TimestampModeVendorStartUnused = 0x7F000000, 
OMX_TimestampModeMax = 0x7FFFFFFF
} OMX_TIMESTAMPMODETYPE;
typedef struct OMX_PARAM_TIMESTAMPMODETYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_TIMESTAMPMODETYPE eTimestampMode;
} OMX_PARAM_TIMESTAMPMODETYPE;
typedef struct OMX_BRCMVEGLIMAGETYPE
{
OMX_U32 nWidth;
OMX_U32 nHeight;
OMX_U32 nStride;
OMX_U32 nUmemHandle;
OMX_U32 nUmemOffset;
OMX_U32 nFlipped; 
} OMX_BRCMVEGLIMAGETYPE;
typedef struct OMX_CONFIG_BRCMFOVTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 xFieldOfViewHorizontal; 
OMX_U32 xFieldOfViewVertical; 
} OMX_CONFIG_BRCMFOVTYPE;
typedef struct OMX_VIDEO_CONFIG_LEVEL_EXTEND {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 nCustomMaxMBPS; 
OMX_U32 nCustomMaxFS; 
OMX_U32 nCustomMaxBRandCPB; 
} OMX_VIDEO_CONFIG_LEVEL_EXTEND;
typedef struct OMX_VIDEO_EEDE_ENABLE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 enable;
} OMX_VIDEO_EEDE_ENABLE;
typedef struct OMX_VIDEO_EEDE_LOSSRATE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_U32 loss_rate; 
} OMX_VIDEO_EEDE_LOSSRATE;
typedef enum OMX_COLORSPACETYPE
{
OMX_COLORSPACE_UNKNOWN,
OMX_COLORSPACE_JPEG_JFIF,
OMX_COLORSPACE_ITU_R_BT601,
OMX_COLORSPACE_ITU_R_BT709,
OMX_COLORSPACE_FCC,
OMX_COLORSPACE_SMPTE240M,
OMX_COLORSPACE_BT470_2_M,
OMX_COLORSPACE_BT470_2_BG,
OMX_COLORSPACE_JFIF_Y16_255,
OMX_COLORSPACE_MAX = 0x7FFFFFFF
} OMX_COLORSPACETYPE;
typedef struct OMX_PARAM_COLORSPACETYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_COLORSPACETYPE eColorSpace;
} OMX_PARAM_COLORSPACETYPE;
typedef enum OMX_CAPTURESTATETYPE
{
OMX_NotCapturing,
OMX_CaptureStarted,
OMX_CaptureComplete,
OMX_CaptureMax = 0x7FFFFFFF
} OMX_CAPTURESTATETYPE;
typedef struct OMX_PARAM_CAPTURESTATETYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_CAPTURESTATETYPE eCaptureState;
} OMX_PARAM_CAPTURESTATETYPE;
typedef struct OMX_PARAM_BRCMCONFIGFILETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 fileSize; 
} OMX_PARAM_BRCMCONFIGFILETYPE;
typedef struct OMX_PARAM_BRCMCONFIGFILECHUNKTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 size; 
OMX_U32 offset; 
OMX_U8 data[1]; 
} OMX_PARAM_BRCMCONFIGFILECHUNKTYPE;
typedef struct OMX_PARAM_BRCMFRAMERATERANGETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex;
OMX_U32 xFramerateLow; 
OMX_U32 xFramerateHigh; 
} OMX_PARAM_BRCMFRAMERATERANGETYPE;
typedef struct OMX_PARAM_S32TYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_S32 nS32; 
} OMX_PARAM_S32TYPE;
typedef struct OMX_PARAM_BRCMVIDEODRMPROTECTBUFFERTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 size_wanted; 
OMX_U32 protect; 
OMX_U32 mem_handle; 
OMX_PTR phys_addr; 
} OMX_PARAM_BRCMVIDEODRMPROTECTBUFFERTYPE;
typedef struct OMX_CONFIG_ZEROSHUTTERLAGTYPE
{
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 bZeroShutterMode; 
OMX_U32 bConcurrentCapture; 
} OMX_CONFIG_ZEROSHUTTERLAGTYPE;
typedef struct OMX_PARAM_BRCMVIDEODECODECONFIGVD3TYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U8 config[1]; 
} OMX_PARAM_BRCMVIDEODECODECONFIGVD3TYPE;
