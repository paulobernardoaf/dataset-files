



























#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#if !defined(VC_EXTRALEAN)
#define VC_EXTRALEAN
#endif

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif




#undef APIENTRY


#if !defined(UNICODE)
#define UNICODE
#endif


#if WINVER < 0x0501
#undef WINVER
#define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif


#define DIRECTINPUT_VERSION 0x0800


#define OEMRESOURCE

#include <wctype.h>
#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include <dbt.h>


#if !defined(WM_MOUSEHWHEEL)
#define WM_MOUSEHWHEEL 0x020E
#endif
#if !defined(WM_DWMCOMPOSITIONCHANGED)
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#if !defined(WM_COPYGLOBALDATA)
#define WM_COPYGLOBALDATA 0x0049
#endif
#if !defined(WM_UNICHAR)
#define WM_UNICHAR 0x0109
#endif
#if !defined(UNICODE_NOCHAR)
#define UNICODE_NOCHAR 0xFFFF
#endif
#if !defined(WM_DPICHANGED)
#define WM_DPICHANGED 0x02E0
#endif
#if !defined(GET_XBUTTON_WPARAM)
#define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#if !defined(EDS_ROTATEDMODE)
#define EDS_ROTATEDMODE 0x00000004
#endif
#if !defined(DISPLAY_DEVICE_ACTIVE)
#define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#if !defined(_WIN32_WINNT_WINBLUE)
#define _WIN32_WINNT_WINBLUE 0x0602
#endif
#if !defined(_WIN32_WINNT_WIN8)
#define _WIN32_WINNT_WIN8 0x0602
#endif
#if !defined(WM_GETDPISCALEDSIZE)
#define WM_GETDPISCALEDSIZE 0x02e4
#endif
#if !defined(USER_DEFAULT_SCREEN_DPI)
#define USER_DEFAULT_SCREEN_DPI 96
#endif
#if !defined(OCR_HAND)
#define OCR_HAND 32649
#endif

#if WINVER < 0x0601
typedef struct
{
DWORD cbSize;
DWORD ExtStatus;
} CHANGEFILTERSTRUCT;
#if !defined(MSGFLT_ALLOW)
#define MSGFLT_ALLOW 1
#endif
#endif 

#if WINVER < 0x0600
#define DWM_BB_ENABLE 0x00000001
#define DWM_BB_BLURREGION 0x00000002
typedef struct
{
DWORD dwFlags;
BOOL fEnable;
HRGN hRgnBlur;
BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;
#else
#include <dwmapi.h>
#endif 

#if !defined(DPI_ENUMS_DECLARED)
typedef enum
{
PROCESS_DPI_UNAWARE = 0,
PROCESS_SYSTEM_DPI_AWARE = 1,
PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
MDT_EFFECTIVE_DPI = 0,
MDT_ANGULAR_DPI = 1,
MDT_RAW_DPI = 2,
MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif 

#if !defined(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
#endif 


#define IsWindowsXPOrGreater() _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0)


#define IsWindowsVistaOrGreater() _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0)


#define IsWindows7OrGreater() _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0)


#define IsWindows8OrGreater() _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0)


#define IsWindows8Point1OrGreater() _glfwIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0)



#define _glfwIsWindows10AnniversaryUpdateOrGreaterWin32() _glfwIsWindows10BuildOrGreaterWin32(14393)

#define _glfwIsWindows10CreatorsUpdateOrGreaterWin32() _glfwIsWindows10BuildOrGreaterWin32(15063)



#if !defined(XINPUT_CAPS_WIRELESS)
#define XINPUT_CAPS_WIRELESS 0x0002
#endif
#if !defined(XINPUT_DEVSUBTYPE_WHEEL)
#define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#if !defined(XINPUT_DEVSUBTYPE_ARCADE_STICK)
#define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#if !defined(XINPUT_DEVSUBTYPE_FLIGHT_STICK)
#define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#if !defined(XINPUT_DEVSUBTYPE_DANCE_PAD)
#define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#if !defined(XINPUT_DEVSUBTYPE_GUITAR)
#define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#if !defined(XINPUT_DEVSUBTYPE_DRUM_KIT)
#define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#if !defined(XINPUT_DEVSUBTYPE_ARCADE_PAD)
#define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#if !defined(XUSER_MAX_COUNT)
#define XUSER_MAX_COUNT 4
#endif


#if !defined(DIDFT_OPTIONAL)
#define DIDFT_OPTIONAL 0x80000000
#endif


typedef DWORD (WINAPI * PFN_timeGetTime)(void);
#define timeGetTime _glfw.win32.winmm.GetTime


typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
#define XInputGetCapabilities _glfw.win32.xinput.GetCapabilities
#define XInputGetState _glfw.win32.xinput.GetState


typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
#define DirectInput8Create _glfw.win32.dinput8.Create


typedef BOOL (WINAPI * PFN_SetProcessDPIAware)(void);
typedef BOOL (WINAPI * PFN_ChangeWindowMessageFilterEx)(HWND,UINT,DWORD,CHANGEFILTERSTRUCT*);
typedef BOOL (WINAPI * PFN_EnableNonClientDpiScaling)(HWND);
typedef BOOL (WINAPI * PFN_SetProcessDpiAwarenessContext)(HANDLE);
typedef UINT (WINAPI * PFN_GetDpiForWindow)(HWND);
typedef BOOL (WINAPI * PFN_AdjustWindowRectExForDpi)(LPRECT,DWORD,BOOL,DWORD,UINT);
#define SetProcessDPIAware _glfw.win32.user32.SetProcessDPIAware_
#define ChangeWindowMessageFilterEx _glfw.win32.user32.ChangeWindowMessageFilterEx_
#define EnableNonClientDpiScaling _glfw.win32.user32.EnableNonClientDpiScaling_
#define SetProcessDpiAwarenessContext _glfw.win32.user32.SetProcessDpiAwarenessContext_
#define GetDpiForWindow _glfw.win32.user32.GetDpiForWindow_
#define AdjustWindowRectExForDpi _glfw.win32.user32.AdjustWindowRectExForDpi_


typedef HRESULT (WINAPI * PFN_DwmIsCompositionEnabled)(BOOL*);
typedef HRESULT (WINAPI * PFN_DwmFlush)(VOID);
typedef HRESULT(WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND,const DWM_BLURBEHIND*);
#define DwmIsCompositionEnabled _glfw.win32.dwmapi.IsCompositionEnabled
#define DwmFlush _glfw.win32.dwmapi.Flush
#define DwmEnableBlurBehindWindow _glfw.win32.dwmapi.EnableBlurBehindWindow


typedef HRESULT (WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT (WINAPI * PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
#define SetProcessDpiAwareness _glfw.win32.shcore.SetProcessDpiAwareness_
#define GetDpiForMonitor _glfw.win32.shcore.GetDpiForMonitor_


typedef LONG (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);
#define RtlVerifyVersionInfo _glfw.win32.ntdll.RtlVerifyVersionInfo_

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
VkStructureType sType;
const void* pNext;
VkWin32SurfaceCreateFlagsKHR flags;
HINSTANCE hinstance;
HWND hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(VkInstance,const VkWin32SurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice,uint32_t);

#include "win32_joystick.h"
#include "wgl_context.h"
#include "egl_context.h"
#include "osmesa_context.h"

#if !defined(_GLFW_WNDCLASSNAME)
#define _GLFW_WNDCLASSNAME L"GLFW30"
#endif

#define _glfw_dlopen(name) LoadLibraryA(name)
#define _glfw_dlclose(handle) FreeLibrary((HMODULE) handle)
#define _glfw_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)

#define _GLFW_EGL_NATIVE_WINDOW ((EGLNativeWindowType) window->win32.handle)
#define _GLFW_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY

#define _GLFW_PLATFORM_WINDOW_STATE _GLFWwindowWin32 win32
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryWin32 win32
#define _GLFW_PLATFORM_LIBRARY_TIMER_STATE _GLFWtimerWin32 win32
#define _GLFW_PLATFORM_MONITOR_STATE _GLFWmonitorWin32 win32
#define _GLFW_PLATFORM_CURSOR_STATE _GLFWcursorWin32 win32
#define _GLFW_PLATFORM_TLS_STATE _GLFWtlsWin32 win32
#define _GLFW_PLATFORM_MUTEX_STATE _GLFWmutexWin32 win32




typedef struct _GLFWwindowWin32
{
HWND handle;
HICON bigIcon;
HICON smallIcon;

GLFWbool cursorTracked;
GLFWbool frameAction;
GLFWbool iconified;
GLFWbool maximized;

GLFWbool transparent;
GLFWbool scaleToMonitor;
GLFWbool keymenu;


int lastCursorPosX, lastCursorPosY;

} _GLFWwindowWin32;



typedef struct _GLFWlibraryWin32
{
HWND helperWindowHandle;
HDEVNOTIFY deviceNotificationHandle;
DWORD foregroundLockTimeout;
int acquiredMonitorCount;
char* clipboardString;
short int keycodes[512];
short int scancodes[GLFW_KEY_LAST + 1];
char keynames[GLFW_KEY_LAST + 1][5];

double restoreCursorPosX, restoreCursorPosY;

_GLFWwindow* disabledCursorWindow;
RAWINPUT* rawInput;
int rawInputSize;
UINT mouseTrailSize;

struct {
HINSTANCE instance;
PFN_timeGetTime GetTime;
} winmm;

struct {
HINSTANCE instance;
PFN_DirectInput8Create Create;
IDirectInput8W* api;
} dinput8;

struct {
HINSTANCE instance;
PFN_XInputGetCapabilities GetCapabilities;
PFN_XInputGetState GetState;
} xinput;

struct {
HINSTANCE instance;
PFN_SetProcessDPIAware SetProcessDPIAware_;
PFN_ChangeWindowMessageFilterEx ChangeWindowMessageFilterEx_;
PFN_EnableNonClientDpiScaling EnableNonClientDpiScaling_;
PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_;
PFN_GetDpiForWindow GetDpiForWindow_;
PFN_AdjustWindowRectExForDpi AdjustWindowRectExForDpi_;
} user32;

struct {
HINSTANCE instance;
PFN_DwmIsCompositionEnabled IsCompositionEnabled;
PFN_DwmFlush Flush;
PFN_DwmEnableBlurBehindWindow EnableBlurBehindWindow;
} dwmapi;

struct {
HINSTANCE instance;
PFN_SetProcessDpiAwareness SetProcessDpiAwareness_;
PFN_GetDpiForMonitor GetDpiForMonitor_;
} shcore;

struct {
HINSTANCE instance;
PFN_RtlVerifyVersionInfo RtlVerifyVersionInfo_;
} ntdll;

} _GLFWlibraryWin32;



typedef struct _GLFWmonitorWin32
{
HMONITOR handle;

WCHAR adapterName[32];
WCHAR displayName[32];
char publicAdapterName[32];
char publicDisplayName[32];
GLFWbool modesPruned;
GLFWbool modeChanged;

} _GLFWmonitorWin32;



typedef struct _GLFWcursorWin32
{
HCURSOR handle;

} _GLFWcursorWin32;



typedef struct _GLFWtimerWin32
{
GLFWbool hasPC;
uint64_t frequency;

} _GLFWtimerWin32;



typedef struct _GLFWtlsWin32
{
GLFWbool allocated;
DWORD index;

} _GLFWtlsWin32;



typedef struct _GLFWmutexWin32
{
GLFWbool allocated;
CRITICAL_SECTION section;

} _GLFWmutexWin32;


GLFWbool _glfwRegisterWindowClassWin32(void);
void _glfwUnregisterWindowClassWin32(void);

WCHAR* _glfwCreateWideStringFromUTF8Win32(const char* source);
char* _glfwCreateUTF8FromWideStringWin32(const WCHAR* source);
BOOL _glfwIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
BOOL _glfwIsWindows10BuildOrGreaterWin32(WORD build);
void _glfwInputErrorWin32(int error, const char* description);
void _glfwUpdateKeyNamesWin32(void);

void _glfwInitTimerWin32(void);

void _glfwPollMonitorsWin32(void);
void _glfwSetVideoModeWin32(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoModeWin32(_GLFWmonitor* monitor);
void _glfwGetMonitorContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);

