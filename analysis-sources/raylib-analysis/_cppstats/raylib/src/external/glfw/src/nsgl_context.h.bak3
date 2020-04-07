




























#if MAC_OS_X_VERSION_MAX_ALLOWED < 101400
#define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
#define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
#endif

#define _GLFW_PLATFORM_CONTEXT_STATE _GLFWcontextNSGL nsgl
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE _GLFWlibraryNSGL nsgl

#include <stdatomic.h>




typedef struct _GLFWcontextNSGL
{
id pixelFormat;
id object;

} _GLFWcontextNSGL;



typedef struct _GLFWlibraryNSGL
{

CFBundleRef framework;

} _GLFWlibraryNSGL;


GLFWbool _glfwInitNSGL(void);
void _glfwTerminateNSGL(void);
GLFWbool _glfwCreateContextNSGL(_GLFWwindow* window,
const _GLFWctxconfig* ctxconfig,
const _GLFWfbconfig* fbconfig);
void _glfwDestroyContextNSGL(_GLFWwindow* window);

