#include <KHR/khrplatform.h>
#if !defined(EGLAPI)
#define EGLAPI KHRONOS_APICALL
#endif
#if !defined(EGLAPIENTRY)
#define EGLAPIENTRY KHRONOS_APIENTRY
#endif
#define EGLAPIENTRYP EGLAPIENTRY*
#if defined(_WIN32) || defined(__VC32__) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__) 
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif
#if !defined(PLATFORM_UWP)
typedef void *PVOID; 
typedef PVOID HANDLE; 
typedef HANDLE HWND; 
typedef HANDLE HDC; 
typedef HANDLE HBITMAP; 
#else
#include "Windows.h"
#endif
typedef HDC EGLNativeDisplayType;
typedef HBITMAP EGLNativePixmapType;
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) 
typedef HWND EGLNativeWindowType;
#else 
#include <inspectable.h>
typedef IInspectable* EGLNativeWindowType;
#endif
#elif defined(__WINSCW__) || defined(__SYMBIAN32__) 
typedef int EGLNativeDisplayType;
typedef void *EGLNativeWindowType;
typedef void *EGLNativePixmapType;
#elif defined(__ANDROID__) || defined(ANDROID)
#include <android/native_window.h>
struct egl_native_pixmap_t;
typedef struct ANativeWindow* EGLNativeWindowType;
typedef struct egl_native_pixmap_t* EGLNativePixmapType;
typedef void* EGLNativeDisplayType;
#elif defined(__unix__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
typedef Display *EGLNativeDisplayType;
typedef Pixmap EGLNativePixmapType;
typedef Window EGLNativeWindowType;
#elif defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )
typedef void *EGLNativeDisplayType;
typedef void *EGLNativePixmapType;
typedef void *EGLNativeWindowType;
#else
#error "Platform not recognized"
#endif
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType NativePixmapType;
typedef EGLNativeWindowType NativeWindowType;
typedef khronos_int32_t EGLint;
