


























#include <dlfcn.h>

#define _GLFW_PLATFORM_WINDOW_STATE _GLFWwindowNull null

#define _GLFW_PLATFORM_CONTEXT_STATE struct { int dummyContext; }
#define _GLFW_PLATFORM_MONITOR_STATE struct { int dummyMonitor; }
#define _GLFW_PLATFORM_CURSOR_STATE struct { int dummyCursor; }
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE struct { int dummyLibraryWindow; }
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE struct { int dummyLibraryContext; }
#define _GLFW_EGL_CONTEXT_STATE struct { int dummyEGLContext; }
#define _GLFW_EGL_LIBRARY_CONTEXT_STATE struct { int dummyEGLLibraryContext; }

#include "osmesa_context.h"
#include "posix_time.h"
#include "posix_thread.h"
#include "null_joystick.h"

#if defined(_GLFW_WIN32)
#define _glfw_dlopen(name) LoadLibraryA(name)
#define _glfw_dlclose(handle) FreeLibrary((HMODULE) handle)
#define _glfw_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)
#else
#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)
#endif



typedef struct _GLFWwindowNull
{
int width;
int height;
} _GLFWwindowNull;

