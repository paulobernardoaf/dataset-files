#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xrandr.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/XInput2.h>
typedef XRRCrtcGamma* (* PFN_XRRAllocGamma)(int);
typedef void (* PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);
typedef void (* PFN_XRRFreeGamma)(XRRCrtcGamma*);
typedef void (* PFN_XRRFreeOutputInfo)(XRROutputInfo*);
typedef void (* PFN_XRRFreeScreenResources)(XRRScreenResources*);
typedef XRRCrtcGamma* (* PFN_XRRGetCrtcGamma)(Display*,RRCrtc);
typedef int (* PFN_XRRGetCrtcGammaSize)(Display*,RRCrtc);
typedef XRRCrtcInfo* (* PFN_XRRGetCrtcInfo) (Display*,XRRScreenResources*,RRCrtc);
typedef XRROutputInfo* (* PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
typedef RROutput (* PFN_XRRGetOutputPrimary)(Display*,Window);
typedef XRRScreenResources* (* PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
typedef Bool (* PFN_XRRQueryExtension)(Display*,int*,int*);
typedef Status (* PFN_XRRQueryVersion)(Display*,int*,int*);
typedef void (* PFN_XRRSelectInput)(Display*,Window,int);
typedef Status (* PFN_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
typedef void (* PFN_XRRSetCrtcGamma)(Display*,RRCrtc,XRRCrtcGamma*);
typedef int (* PFN_XRRUpdateConfiguration)(XEvent*);
#define XRRAllocGamma _glfw.x11.randr.AllocGamma
#define XRRFreeCrtcInfo _glfw.x11.randr.FreeCrtcInfo
#define XRRFreeGamma _glfw.x11.randr.FreeGamma
#define XRRFreeOutputInfo _glfw.x11.randr.FreeOutputInfo
#define XRRFreeScreenResources _glfw.x11.randr.FreeScreenResources
#define XRRGetCrtcGamma _glfw.x11.randr.GetCrtcGamma
#define XRRGetCrtcGammaSize _glfw.x11.randr.GetCrtcGammaSize
#define XRRGetCrtcInfo _glfw.x11.randr.GetCrtcInfo
#define XRRGetOutputInfo _glfw.x11.randr.GetOutputInfo
#define XRRGetOutputPrimary _glfw.x11.randr.GetOutputPrimary
#define XRRGetScreenResourcesCurrent _glfw.x11.randr.GetScreenResourcesCurrent
#define XRRQueryExtension _glfw.x11.randr.QueryExtension
#define XRRQueryVersion _glfw.x11.randr.QueryVersion
#define XRRSelectInput _glfw.x11.randr.SelectInput
#define XRRSetCrtcConfig _glfw.x11.randr.SetCrtcConfig
#define XRRSetCrtcGamma _glfw.x11.randr.SetCrtcGamma
#define XRRUpdateConfiguration _glfw.x11.randr.UpdateConfiguration
typedef XcursorImage* (* PFN_XcursorImageCreate)(int,int);
typedef void (* PFN_XcursorImageDestroy)(XcursorImage*);
typedef Cursor (* PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
typedef char* (* PFN_XcursorGetTheme)(Display*);
typedef int (* PFN_XcursorGetDefaultSize)(Display*);
typedef XcursorImage* (* PFN_XcursorLibraryLoadImage)(const char*,const char*,int);
#define XcursorImageCreate _glfw.x11.xcursor.ImageCreate
#define XcursorImageDestroy _glfw.x11.xcursor.ImageDestroy
#define XcursorImageLoadCursor _glfw.x11.xcursor.ImageLoadCursor
#define XcursorGetTheme _glfw.x11.xcursor.GetTheme
#define XcursorGetDefaultSize _glfw.x11.xcursor.GetDefaultSize
#define XcursorLibraryLoadImage _glfw.x11.xcursor.LibraryLoadImage
typedef Bool (* PFN_XineramaIsActive)(Display*);
typedef Bool (* PFN_XineramaQueryExtension)(Display*,int*,int*);
typedef XineramaScreenInfo* (* PFN_XineramaQueryScreens)(Display*,int*);
#define XineramaIsActive _glfw.x11.xinerama.IsActive
#define XineramaQueryExtension _glfw.x11.xinerama.QueryExtension
#define XineramaQueryScreens _glfw.x11.xinerama.QueryScreens
typedef XID xcb_window_t;
typedef XID xcb_visualid_t;
typedef struct xcb_connection_t xcb_connection_t;
typedef xcb_connection_t* (* PFN_XGetXCBConnection)(Display*);
#define XGetXCBConnection _glfw.x11.x11xcb.GetXCBConnection
typedef Bool (* PFN_XF86VidModeQueryExtension)(Display*,int*,int*);
typedef Bool (* PFN_XF86VidModeGetGammaRamp)(Display*,int,int,unsigned short*,unsigned short*,unsigned short*);
typedef Bool (* PFN_XF86VidModeSetGammaRamp)(Display*,int,int,unsigned short*,unsigned short*,unsigned short*);
typedef Bool (* PFN_XF86VidModeGetGammaRampSize)(Display*,int,int*);
#define XF86VidModeQueryExtension _glfw.x11.vidmode.QueryExtension
#define XF86VidModeGetGammaRamp _glfw.x11.vidmode.GetGammaRamp
#define XF86VidModeSetGammaRamp _glfw.x11.vidmode.SetGammaRamp
#define XF86VidModeGetGammaRampSize _glfw.x11.vidmode.GetGammaRampSize
typedef Status (* PFN_XIQueryVersion)(Display*,int*,int*);
typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
#define XIQueryVersion _glfw.x11.xi.QueryVersion
#define XISelectEvents _glfw.x11.xi.SelectEvents
typedef Bool (* PFN_XRenderQueryExtension)(Display*,int*,int*);
typedef Status (* PFN_XRenderQueryVersion)(Display*dpy,int*,int*);
typedef XRenderPictFormat* (* PFN_XRenderFindVisualFormat)(Display*,Visual const*);
#define XRenderQueryExtension _glfw.x11.xrender.QueryExtension
#define XRenderQueryVersion _glfw.x11.xrender.QueryVersion
#define XRenderFindVisualFormat _glfw.x11.xrender.FindVisualFormat
typedef VkFlags VkXlibSurfaceCreateFlagsKHR;
typedef VkFlags VkXcbSurfaceCreateFlagsKHR;
typedef struct VkXlibSurfaceCreateInfoKHR
{
VkStructureType sType;
const void* pNext;
VkXlibSurfaceCreateFlagsKHR flags;
Display* dpy;
Window window;
} VkXlibSurfaceCreateInfoKHR;
typedef struct VkXcbSurfaceCreateInfoKHR
{
VkStructureType sType;
const void* pNext;
VkXcbSurfaceCreateFlagsKHR flags;
xcb_connection_t* connection;
xcb_window_t window;
} VkXcbSurfaceCreateInfoKHR;
typedef VkResult (APIENTRY *PFN_vkCreateXlibSurfaceKHR)(VkInstance,const VkXlibSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)(VkPhysicalDevice,uint32_t,Display*,VisualID);
typedef VkResult (APIENTRY *PFN_vkCreateXcbSurfaceKHR)(VkInstance,const VkXcbSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)(VkPhysicalDevice,uint32_t,xcb_connection_t*,xcb_visualid_t);
#include "posix_thread.h"
#include "posix_time.h"
#include "xkb_unicode.h"
#include "glx_context.h"
#include "egl_context.h"
#include "osmesa_context.h"
#if defined(__linux__)
#include "linux_joystick.h"
#else
#include "null_joystick.h"
#endif
#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)
#define _GLFW_EGL_NATIVE_WINDOW ((EGLNativeWindowType) window->x11.handle)
#define _GLFW_EGL_NATIVE_DISPLAY ((EGLNativeDisplayType) _glfw.x11.display)
#define _GLFW_PLATFORM_WINDOW_STATE _GLFWwindowX11 x11
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryX11 x11
#define _GLFW_PLATFORM_MONITOR_STATE _GLFWmonitorX11 x11
#define _GLFW_PLATFORM_CURSOR_STATE _GLFWcursorX11 x11
typedef struct _GLFWwindowX11
{
Colormap colormap;
Window handle;
Window parent;
XIC ic;
GLFWbool overrideRedirect;
GLFWbool iconified;
GLFWbool maximized;
GLFWbool transparent;
int width, height;
int xpos, ypos;
int lastCursorPosX, lastCursorPosY;
int warpCursorPosX, warpCursorPosY;
Time lastKeyTime;
} _GLFWwindowX11;
typedef struct _GLFWlibraryX11
{
Display* display;
int screen;
Window root;
float contentScaleX, contentScaleY;
Window helperWindowHandle;
Cursor hiddenCursorHandle;
XContext context;
XIM im;
int errorCode;
char* primarySelectionString;
char* clipboardString;
char keynames[GLFW_KEY_LAST + 1][5];
short int keycodes[256];
short int scancodes[GLFW_KEY_LAST + 1];
double restoreCursorPosX, restoreCursorPosY;
_GLFWwindow* disabledCursorWindow;
Atom NET_SUPPORTED;
Atom NET_SUPPORTING_WM_CHECK;
Atom WM_PROTOCOLS;
Atom WM_STATE;
Atom WM_DELETE_WINDOW;
Atom NET_WM_NAME;
Atom NET_WM_ICON_NAME;
Atom NET_WM_ICON;
Atom NET_WM_PID;
Atom NET_WM_PING;
Atom NET_WM_WINDOW_TYPE;
Atom NET_WM_WINDOW_TYPE_NORMAL;
Atom NET_WM_STATE;
Atom NET_WM_STATE_ABOVE;
Atom NET_WM_STATE_FULLSCREEN;
Atom NET_WM_STATE_MAXIMIZED_VERT;
Atom NET_WM_STATE_MAXIMIZED_HORZ;
Atom NET_WM_STATE_DEMANDS_ATTENTION;
Atom NET_WM_BYPASS_COMPOSITOR;
Atom NET_WM_FULLSCREEN_MONITORS;
Atom NET_WM_WINDOW_OPACITY;
Atom NET_WM_CM_Sx;
Atom NET_WORKAREA;
Atom NET_CURRENT_DESKTOP;
Atom NET_ACTIVE_WINDOW;
Atom NET_FRAME_EXTENTS;
Atom NET_REQUEST_FRAME_EXTENTS;
Atom MOTIF_WM_HINTS;
Atom XdndAware;
Atom XdndEnter;
Atom XdndPosition;
Atom XdndStatus;
Atom XdndActionCopy;
Atom XdndDrop;
Atom XdndFinished;
Atom XdndSelection;
Atom XdndTypeList;
Atom text_uri_list;
Atom TARGETS;
Atom MULTIPLE;
Atom INCR;
Atom CLIPBOARD;
Atom PRIMARY;
Atom CLIPBOARD_MANAGER;
Atom SAVE_TARGETS;
Atom NULL_;
Atom UTF8_STRING;
Atom COMPOUND_STRING;
Atom ATOM_PAIR;
Atom GLFW_SELECTION;
struct {
GLFWbool available;
void* handle;
int eventBase;
int errorBase;
int major;
int minor;
GLFWbool gammaBroken;
GLFWbool monitorBroken;
PFN_XRRAllocGamma AllocGamma;
PFN_XRRFreeCrtcInfo FreeCrtcInfo;
PFN_XRRFreeGamma FreeGamma;
PFN_XRRFreeOutputInfo FreeOutputInfo;
PFN_XRRFreeScreenResources FreeScreenResources;
PFN_XRRGetCrtcGamma GetCrtcGamma;
PFN_XRRGetCrtcGammaSize GetCrtcGammaSize;
PFN_XRRGetCrtcInfo GetCrtcInfo;
PFN_XRRGetOutputInfo GetOutputInfo;
PFN_XRRGetOutputPrimary GetOutputPrimary;
PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent;
PFN_XRRQueryExtension QueryExtension;
PFN_XRRQueryVersion QueryVersion;
PFN_XRRSelectInput SelectInput;
PFN_XRRSetCrtcConfig SetCrtcConfig;
PFN_XRRSetCrtcGamma SetCrtcGamma;
PFN_XRRUpdateConfiguration UpdateConfiguration;
} randr;
struct {
GLFWbool available;
GLFWbool detectable;
int majorOpcode;
int eventBase;
int errorBase;
int major;
int minor;
unsigned int group;
} xkb;
struct {
int count;
int timeout;
int interval;
int blanking;
int exposure;
} saver;
struct {
int version;
Window source;
Atom format;
} xdnd;
struct {
void* handle;
PFN_XcursorImageCreate ImageCreate;
PFN_XcursorImageDestroy ImageDestroy;
PFN_XcursorImageLoadCursor ImageLoadCursor;
PFN_XcursorGetTheme GetTheme;
PFN_XcursorGetDefaultSize GetDefaultSize;
PFN_XcursorLibraryLoadImage LibraryLoadImage;
} xcursor;
struct {
GLFWbool available;
void* handle;
int major;
int minor;
PFN_XineramaIsActive IsActive;
PFN_XineramaQueryExtension QueryExtension;
PFN_XineramaQueryScreens QueryScreens;
} xinerama;
struct {
void* handle;
PFN_XGetXCBConnection GetXCBConnection;
} x11xcb;
struct {
GLFWbool available;
void* handle;
int eventBase;
int errorBase;
PFN_XF86VidModeQueryExtension QueryExtension;
PFN_XF86VidModeGetGammaRamp GetGammaRamp;
PFN_XF86VidModeSetGammaRamp SetGammaRamp;
PFN_XF86VidModeGetGammaRampSize GetGammaRampSize;
} vidmode;
struct {
GLFWbool available;
void* handle;
int majorOpcode;
int eventBase;
int errorBase;
int major;
int minor;
PFN_XIQueryVersion QueryVersion;
PFN_XISelectEvents SelectEvents;
} xi;
struct {
GLFWbool available;
void* handle;
int major;
int minor;
int eventBase;
int errorBase;
PFN_XRenderQueryExtension QueryExtension;
PFN_XRenderQueryVersion QueryVersion;
PFN_XRenderFindVisualFormat FindVisualFormat;
} xrender;
} _GLFWlibraryX11;
typedef struct _GLFWmonitorX11
{
RROutput output;
RRCrtc crtc;
RRMode oldMode;
int index;
} _GLFWmonitorX11;
typedef struct _GLFWcursorX11
{
Cursor handle;
} _GLFWcursorX11;
void _glfwPollMonitorsX11(void);
void _glfwSetVideoModeX11(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoModeX11(_GLFWmonitor* monitor);
Cursor _glfwCreateCursorX11(const GLFWimage* image, int xhot, int yhot);
unsigned long _glfwGetWindowPropertyX11(Window window,
Atom property,
Atom type,
unsigned char** value);
GLFWbool _glfwIsVisualTransparentX11(Visual* visual);
void _glfwGrabErrorHandlerX11(void);
void _glfwReleaseErrorHandlerX11(void);
void _glfwInputErrorX11(int error, const char* message);
void _glfwPushSelectionToManagerX11(void);
