




























#if !defined(__MFXDEFS_H__)
#define __MFXDEFS_H__

#define MFX_VERSION_MAJOR 1
#define MFX_VERSION_MINOR 27

#define MFX_VERSION_NEXT 1028

#if !defined(MFX_VERSION)
#define MFX_VERSION (MFX_VERSION_MAJOR * 1000 + MFX_VERSION_MINOR)
#endif

#if defined(__cplusplus)
extern "C"
{
#endif 


#if (defined( _WIN32 ) || defined ( _WIN64 )) && !defined (__GNUC__)
#define __INT64 __int64
#define __UINT64 unsigned __int64
#else
#define __INT64 long long
#define __UINT64 unsigned long long
#endif

#if defined(_WIN32)
#define MFX_CDECL __cdecl
#define MFX_STDCALL __stdcall
#else
#define MFX_CDECL
#define MFX_STDCALL
#endif 

#define MFX_INFINITE 0xFFFFFFFF

typedef unsigned char mfxU8;
typedef char mfxI8;
typedef short mfxI16;
typedef unsigned short mfxU16;
typedef unsigned int mfxU32;
typedef int mfxI32;
#if defined( _WIN32 ) || defined ( _WIN64 )
typedef unsigned long mfxUL32;
typedef long mfxL32;
#else
typedef unsigned int mfxUL32;
typedef int mfxL32;
#endif
typedef float mfxF32;
typedef double mfxF64;
typedef __UINT64 mfxU64;
typedef __INT64 mfxI64;
typedef void* mfxHDL;
typedef mfxHDL mfxMemId;
typedef void* mfxThreadTask;
typedef char mfxChar;

typedef struct {
mfxI16 x;
mfxI16 y;
} mfxI16Pair;

typedef struct {
mfxHDL first;
mfxHDL second;
} mfxHDLPair;





typedef enum
{

MFX_ERR_NONE = 0, 


MFX_ERR_UNKNOWN = -1, 


MFX_ERR_NULL_PTR = -2, 
MFX_ERR_UNSUPPORTED = -3, 
MFX_ERR_MEMORY_ALLOC = -4, 
MFX_ERR_NOT_ENOUGH_BUFFER = -5, 
MFX_ERR_INVALID_HANDLE = -6, 
MFX_ERR_LOCK_MEMORY = -7, 
MFX_ERR_NOT_INITIALIZED = -8, 
MFX_ERR_NOT_FOUND = -9, 
MFX_ERR_MORE_DATA = -10, 
MFX_ERR_MORE_SURFACE = -11, 
MFX_ERR_ABORTED = -12, 
MFX_ERR_DEVICE_LOST = -13, 
MFX_ERR_INCOMPATIBLE_VIDEO_PARAM = -14, 
MFX_ERR_INVALID_VIDEO_PARAM = -15, 
MFX_ERR_UNDEFINED_BEHAVIOR = -16, 
MFX_ERR_DEVICE_FAILED = -17, 
MFX_ERR_MORE_BITSTREAM = -18, 
MFX_ERR_INCOMPATIBLE_AUDIO_PARAM = -19, 
MFX_ERR_INVALID_AUDIO_PARAM = -20, 
MFX_ERR_GPU_HANG = -21, 
MFX_ERR_REALLOC_SURFACE = -22, 


MFX_WRN_IN_EXECUTION = 1, 
MFX_WRN_DEVICE_BUSY = 2, 
MFX_WRN_VIDEO_PARAM_CHANGED = 3, 
MFX_WRN_PARTIAL_ACCELERATION = 4, 
MFX_WRN_INCOMPATIBLE_VIDEO_PARAM = 5, 
MFX_WRN_VALUE_NOT_CHANGED = 6, 
MFX_WRN_OUT_OF_RANGE = 7, 
MFX_WRN_FILTER_SKIPPED = 10, 
MFX_WRN_INCOMPATIBLE_AUDIO_PARAM = 11, 


MFX_TASK_DONE = MFX_ERR_NONE, 
MFX_TASK_WORKING = 8, 
MFX_TASK_BUSY = 9, 


MFX_ERR_MORE_DATA_SUBMIT_TASK = -10000, 

} mfxStatus;



#if defined(MFX_DISPATCHER_EXPOSED_PREFIX)

#include "mfxdispatcherprefixedfunctions.h"

#endif 


#if defined(__cplusplus)
}
#endif 

#endif 
