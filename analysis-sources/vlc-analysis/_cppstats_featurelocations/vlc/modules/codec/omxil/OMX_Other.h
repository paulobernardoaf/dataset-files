



























#if !defined(OMX_Other_h)
#define OMX_Other_h

#if defined(__cplusplus)
extern "C" {
#endif 







#include <OMX_Core.h>







typedef enum OMX_OTHER_FORMATTYPE {
OMX_OTHER_FormatTime = 0, 

OMX_OTHER_FormatPower, 

OMX_OTHER_FormatStats, 

OMX_OTHER_FormatBinary, 
OMX_OTHER_FormatVendorReserved = 1000, 


OMX_OTHER_FormatKhronosExtensions = 0x6F000000, 
OMX_OTHER_FormatVendorStartUnused = 0x7F000000, 
OMX_OTHER_FormatMax = 0x7FFFFFFF
} OMX_OTHER_FORMATTYPE;




typedef enum OMX_TIME_SEEKMODETYPE {
OMX_TIME_SeekModeFast = 0, 



OMX_TIME_SeekModeAccurate, 



OMX_TIME_SeekModeKhronosExtensions = 0x6F000000, 
OMX_TIME_SeekModeVendorStartUnused = 0x7F000000, 
OMX_TIME_SeekModeMax = 0x7FFFFFFF
} OMX_TIME_SEEKMODETYPE;


typedef struct OMX_TIME_CONFIG_SEEKMODETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_TIME_SEEKMODETYPE eType; 
} OMX_TIME_CONFIG_SEEKMODETYPE;























typedef struct OMX_TIME_CONFIG_TIMESTAMPTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 

OMX_U32 nPortIndex; 
OMX_TICKS nTimestamp; 
} OMX_TIME_CONFIG_TIMESTAMPTYPE; 


typedef enum OMX_TIME_UPDATETYPE {
OMX_TIME_UpdateRequestFulfillment, 
OMX_TIME_UpdateScaleChanged, 
OMX_TIME_UpdateClockStateChanged, 
OMX_TIME_UpdateKhronosExtensions = 0x6F000000, 
OMX_TIME_UpdateVendorStartUnused = 0x7F000000, 
OMX_TIME_UpdateMax = 0x7FFFFFFF
} OMX_TIME_UPDATETYPE;


typedef enum OMX_TIME_REFCLOCKTYPE {
OMX_TIME_RefClockNone, 
OMX_TIME_RefClockAudio, 
OMX_TIME_RefClockVideo, 
OMX_TIME_RefClockKhronosExtensions = 0x6F000000, 
OMX_TIME_RefClockVendorStartUnused = 0x7F000000, 
OMX_TIME_RefClockMax = 0x7FFFFFFF
} OMX_TIME_REFCLOCKTYPE;


typedef enum OMX_TIME_CLOCKSTATE {
OMX_TIME_ClockStateRunning, 
OMX_TIME_ClockStateWaitingForStartTime, 


OMX_TIME_ClockStateStopped, 
OMX_TIME_ClockStateKhronosExtensions = 0x6F000000, 
OMX_TIME_ClockStateVendorStartUnused = 0x7F000000, 
OMX_TIME_ClockStateMax = 0x7FFFFFFF
} OMX_TIME_CLOCKSTATE;
























typedef struct OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_PTR pClientPrivate; 




OMX_TICKS nMediaTimestamp; 
OMX_TICKS nOffset; 

} OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE;




























typedef struct OMX_TIME_MEDIATIMETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nClientPrivate; 


OMX_TIME_UPDATETYPE eUpdateType; 
OMX_TICKS nMediaTimestamp; 

OMX_TICKS nOffset; 


OMX_TICKS nWallTimeAtMediaTime; 




OMX_S32 xScale; 
OMX_TIME_CLOCKSTATE eState; 

} OMX_TIME_MEDIATIMETYPE; 






typedef struct OMX_TIME_CONFIG_SCALETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_S32 xScale; 

} OMX_TIME_CONFIG_SCALETYPE;


#define OMX_CLOCKPORT0 0x00000001
#define OMX_CLOCKPORT1 0x00000002
#define OMX_CLOCKPORT2 0x00000004
#define OMX_CLOCKPORT3 0x00000008
#define OMX_CLOCKPORT4 0x00000010
#define OMX_CLOCKPORT5 0x00000020
#define OMX_CLOCKPORT6 0x00000040
#define OMX_CLOCKPORT7 0x00000080













typedef struct OMX_TIME_CONFIG_CLOCKSTATETYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 

OMX_TIME_CLOCKSTATE eState; 
OMX_TICKS nStartTime; 
OMX_TICKS nOffset; 



OMX_U32 nWaitMask; 
} OMX_TIME_CONFIG_CLOCKSTATETYPE;




typedef struct OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_TIME_REFCLOCKTYPE eClock; 
} OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE;



typedef struct OMX_OTHER_CONFIG_POWERTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_BOOL bEnablePM; 
} OMX_OTHER_CONFIG_POWERTYPE;




typedef struct OMX_OTHER_CONFIG_STATSTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 

} OMX_OTHER_CONFIG_STATSTYPE;







typedef struct OMX_OTHER_PORTDEFINITIONTYPE {
OMX_OTHER_FORMATTYPE eFormat; 
} OMX_OTHER_PORTDEFINITIONTYPE;




typedef struct OMX_OTHER_PARAM_PORTFORMATTYPE {
OMX_U32 nSize; 
OMX_VERSIONTYPE nVersion; 
OMX_U32 nPortIndex; 
OMX_U32 nIndex; 
OMX_OTHER_FORMATTYPE eFormat; 
} OMX_OTHER_PARAM_PORTFORMATTYPE; 

#if defined(__cplusplus)
}
#endif 

#endif

