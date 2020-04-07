































#if !defined(AVS_CAPI_H)
#define AVS_CAPI_H

#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#if defined(BUILDING_AVSCORE)
#if defined(GCC) && defined(X86_32)
#define AVSC_CC
#else 
#if !defined(AVSC_USE_STDCALL)
#define AVSC_CC __cdecl
#else
#define AVSC_CC __stdcall
#endif
#endif
#else 
#if !defined(AVSC_WIN32_GCC32)
#if !defined(AVSC_USE_STDCALL)
#define AVSC_CC __cdecl
#else
#define AVSC_CC __stdcall
#endif
#else
#define AVSC_CC
#endif
#endif

















#define AVSC_INLINE static __inline

#if defined(BUILDING_AVSCORE)
#define AVSC_EXPORT __declspec(dllexport)
#define AVSC_API(ret, name) EXTERN_C AVSC_EXPORT ret AVSC_CC name
#else
#define AVSC_EXPORT EXTERN_C __declspec(dllexport)
#if !defined(AVSC_NO_DECLSPEC)
#define AVSC_API(ret, name) EXTERN_C __declspec(dllimport) ret AVSC_CC name
#else
#define AVSC_API(ret, name) typedef ret (AVSC_CC *name##_func)
#endif
#endif

#endif 
