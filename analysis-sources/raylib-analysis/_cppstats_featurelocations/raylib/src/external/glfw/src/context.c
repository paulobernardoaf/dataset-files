




























#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>












GLFWbool _glfwIsValidContextConfig(const _GLFWctxconfig* ctxconfig)
{
if (ctxconfig->share)
{
if (ctxconfig->client == GLFW_NO_API ||
ctxconfig->share->context.client == GLFW_NO_API)
{
_glfwInputError(GLFW_NO_WINDOW_CONTEXT, NULL);
return GLFW_FALSE;
}
}

if (ctxconfig->source != GLFW_NATIVE_CONTEXT_API &&
ctxconfig->source != GLFW_EGL_CONTEXT_API &&
ctxconfig->source != GLFW_OSMESA_CONTEXT_API)
{
_glfwInputError(GLFW_INVALID_ENUM,
"Invalid context creation API 0x%08X",
ctxconfig->source);
return GLFW_FALSE;
}

if (ctxconfig->client != GLFW_NO_API &&
ctxconfig->client != GLFW_OPENGL_API &&
ctxconfig->client != GLFW_OPENGL_ES_API)
{
_glfwInputError(GLFW_INVALID_ENUM,
"Invalid client API 0x%08X",
ctxconfig->client);
return GLFW_FALSE;
}

if (ctxconfig->client == GLFW_OPENGL_API)
{
if ((ctxconfig->major < 1 || ctxconfig->minor < 0) ||
(ctxconfig->major == 1 && ctxconfig->minor > 5) ||
(ctxconfig->major == 2 && ctxconfig->minor > 1) ||
(ctxconfig->major == 3 && ctxconfig->minor > 3))
{






_glfwInputError(GLFW_INVALID_VALUE,
"Invalid OpenGL version %i.%i",
ctxconfig->major, ctxconfig->minor);
return GLFW_FALSE;
}

if (ctxconfig->profile)
{
if (ctxconfig->profile != GLFW_OPENGL_CORE_PROFILE &&
ctxconfig->profile != GLFW_OPENGL_COMPAT_PROFILE)
{
_glfwInputError(GLFW_INVALID_ENUM,
"Invalid OpenGL profile 0x%08X",
ctxconfig->profile);
return GLFW_FALSE;
}

if (ctxconfig->major <= 2 ||
(ctxconfig->major == 3 && ctxconfig->minor < 2))
{



_glfwInputError(GLFW_INVALID_VALUE,
"Context profiles are only defined for OpenGL version 3.2 and above");
return GLFW_FALSE;
}
}

if (ctxconfig->forward && ctxconfig->major <= 2)
{

_glfwInputError(GLFW_INVALID_VALUE,
"Forward-compatibility is only defined for OpenGL version 3.0 and above");
return GLFW_FALSE;
}
}
else if (ctxconfig->client == GLFW_OPENGL_ES_API)
{
if (ctxconfig->major < 1 || ctxconfig->minor < 0 ||
(ctxconfig->major == 1 && ctxconfig->minor > 1) ||
(ctxconfig->major == 2 && ctxconfig->minor > 0))
{





_glfwInputError(GLFW_INVALID_VALUE,
"Invalid OpenGL ES version %i.%i",
ctxconfig->major, ctxconfig->minor);
return GLFW_FALSE;
}
}

if (ctxconfig->robustness)
{
if (ctxconfig->robustness != GLFW_NO_RESET_NOTIFICATION &&
ctxconfig->robustness != GLFW_LOSE_CONTEXT_ON_RESET)
{
_glfwInputError(GLFW_INVALID_ENUM,
"Invalid context robustness mode 0x%08X",
ctxconfig->robustness);
return GLFW_FALSE;
}
}

if (ctxconfig->release)
{
if (ctxconfig->release != GLFW_RELEASE_BEHAVIOR_NONE &&
ctxconfig->release != GLFW_RELEASE_BEHAVIOR_FLUSH)
{
_glfwInputError(GLFW_INVALID_ENUM,
"Invalid context release behavior 0x%08X",
ctxconfig->release);
return GLFW_FALSE;
}
}

return GLFW_TRUE;
}



const _GLFWfbconfig* _glfwChooseFBConfig(const _GLFWfbconfig* desired,
const _GLFWfbconfig* alternatives,
unsigned int count)
{
unsigned int i;
unsigned int missing, leastMissing = UINT_MAX;
unsigned int colorDiff, leastColorDiff = UINT_MAX;
unsigned int extraDiff, leastExtraDiff = UINT_MAX;
const _GLFWfbconfig* current;
const _GLFWfbconfig* closest = NULL;

for (i = 0; i < count; i++)
{
current = alternatives + i;

if (desired->stereo > 0 && current->stereo == 0)
{

continue;
}

if (desired->doublebuffer != current->doublebuffer)
{

continue;
}


{
missing = 0;

if (desired->alphaBits > 0 && current->alphaBits == 0)
missing++;

if (desired->depthBits > 0 && current->depthBits == 0)
missing++;

if (desired->stencilBits > 0 && current->stencilBits == 0)
missing++;

if (desired->auxBuffers > 0 &&
current->auxBuffers < desired->auxBuffers)
{
missing += desired->auxBuffers - current->auxBuffers;
}

if (desired->samples > 0 && current->samples == 0)
{



missing++;
}

if (desired->transparent != current->transparent)
missing++;
}





{
colorDiff = 0;

if (desired->redBits != GLFW_DONT_CARE)
{
colorDiff += (desired->redBits - current->redBits) *
(desired->redBits - current->redBits);
}

if (desired->greenBits != GLFW_DONT_CARE)
{
colorDiff += (desired->greenBits - current->greenBits) *
(desired->greenBits - current->greenBits);
}

if (desired->blueBits != GLFW_DONT_CARE)
{
colorDiff += (desired->blueBits - current->blueBits) *
(desired->blueBits - current->blueBits);
}
}


{
extraDiff = 0;

if (desired->alphaBits != GLFW_DONT_CARE)
{
extraDiff += (desired->alphaBits - current->alphaBits) *
(desired->alphaBits - current->alphaBits);
}

if (desired->depthBits != GLFW_DONT_CARE)
{
extraDiff += (desired->depthBits - current->depthBits) *
(desired->depthBits - current->depthBits);
}

if (desired->stencilBits != GLFW_DONT_CARE)
{
extraDiff += (desired->stencilBits - current->stencilBits) *
(desired->stencilBits - current->stencilBits);
}

if (desired->accumRedBits != GLFW_DONT_CARE)
{
extraDiff += (desired->accumRedBits - current->accumRedBits) *
(desired->accumRedBits - current->accumRedBits);
}

if (desired->accumGreenBits != GLFW_DONT_CARE)
{
extraDiff += (desired->accumGreenBits - current->accumGreenBits) *
(desired->accumGreenBits - current->accumGreenBits);
}

if (desired->accumBlueBits != GLFW_DONT_CARE)
{
extraDiff += (desired->accumBlueBits - current->accumBlueBits) *
(desired->accumBlueBits - current->accumBlueBits);
}

if (desired->accumAlphaBits != GLFW_DONT_CARE)
{
extraDiff += (desired->accumAlphaBits - current->accumAlphaBits) *
(desired->accumAlphaBits - current->accumAlphaBits);
}

if (desired->samples != GLFW_DONT_CARE)
{
extraDiff += (desired->samples - current->samples) *
(desired->samples - current->samples);
}

if (desired->sRGB && !current->sRGB)
extraDiff++;
}





if (missing < leastMissing)
closest = current;
else if (missing == leastMissing)
{
if ((colorDiff < leastColorDiff) ||
(colorDiff == leastColorDiff && extraDiff < leastExtraDiff))
{
closest = current;
}
}

if (current == closest)
{
leastMissing = missing;
leastColorDiff = colorDiff;
leastExtraDiff = extraDiff;
}
}

return closest;
}



GLFWbool _glfwRefreshContextAttribs(_GLFWwindow* window,
const _GLFWctxconfig* ctxconfig)
{
int i;
_GLFWwindow* previous;
const char* version;
const char* prefixes[] =
{
"OpenGL ES-CM ",
"OpenGL ES-CL ",
"OpenGL ES ",
NULL
};

window->context.source = ctxconfig->source;
window->context.client = GLFW_OPENGL_API;

previous = _glfwPlatformGetTls(&_glfw.contextSlot);
glfwMakeContextCurrent((GLFWwindow*) window);

window->context.GetIntegerv = (PFNGLGETINTEGERVPROC)
window->context.getProcAddress("glGetIntegerv");
window->context.GetString = (PFNGLGETSTRINGPROC)
window->context.getProcAddress("glGetString");
if (!window->context.GetIntegerv || !window->context.GetString)
{
_glfwInputError(GLFW_PLATFORM_ERROR, "Entry point retrieval is broken");
glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_FALSE;
}

version = (const char*) window->context.GetString(GL_VERSION);
if (!version)
{
if (ctxconfig->client == GLFW_OPENGL_API)
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"OpenGL version string retrieval is broken");
}
else
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"OpenGL ES version string retrieval is broken");
}

glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_FALSE;
}

for (i = 0; prefixes[i]; i++)
{
const size_t length = strlen(prefixes[i]);

if (strncmp(version, prefixes[i], length) == 0)
{
version += length;
window->context.client = GLFW_OPENGL_ES_API;
break;
}
}

if (!sscanf(version, "%d.%d.%d",
&window->context.major,
&window->context.minor,
&window->context.revision))
{
if (window->context.client == GLFW_OPENGL_API)
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"No version found in OpenGL version string");
}
else
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"No version found in OpenGL ES version string");
}

glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_FALSE;
}

if (window->context.major < ctxconfig->major ||
(window->context.major == ctxconfig->major &&
window->context.minor < ctxconfig->minor))
{







if (window->context.client == GLFW_OPENGL_API)
{
_glfwInputError(GLFW_VERSION_UNAVAILABLE,
"Requested OpenGL version %i.%i, got version %i.%i",
ctxconfig->major, ctxconfig->minor,
window->context.major, window->context.minor);
}
else
{
_glfwInputError(GLFW_VERSION_UNAVAILABLE,
"Requested OpenGL ES version %i.%i, got version %i.%i",
ctxconfig->major, ctxconfig->minor,
window->context.major, window->context.minor);
}

glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_FALSE;
}

if (window->context.major >= 3)
{




window->context.GetStringi = (PFNGLGETSTRINGIPROC)
window->context.getProcAddress("glGetStringi");
if (!window->context.GetStringi)
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"Entry point retrieval is broken");
glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_FALSE;
}
}

if (window->context.client == GLFW_OPENGL_API)
{

if (window->context.major >= 3)
{
GLint flags;
window->context.GetIntegerv(GL_CONTEXT_FLAGS, &flags);

if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
window->context.forward = GLFW_TRUE;

if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
window->context.debug = GLFW_TRUE;
else if (glfwExtensionSupported("GL_ARB_debug_output") &&
ctxconfig->debug)
{



window->context.debug = GLFW_TRUE;
}

if (flags & GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR)
window->context.noerror = GLFW_TRUE;
}


if (window->context.major >= 4 ||
(window->context.major == 3 && window->context.minor >= 2))
{
GLint mask;
window->context.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
window->context.profile = GLFW_OPENGL_CORE_PROFILE;
else if (glfwExtensionSupported("GL_ARB_compatibility"))
{




window->context.profile = GLFW_OPENGL_COMPAT_PROFILE;
}
}


if (glfwExtensionSupported("GL_ARB_robustness"))
{



GLint strategy;
window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB,
&strategy);

if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
window->context.robustness = GLFW_LOSE_CONTEXT_ON_RESET;
else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
window->context.robustness = GLFW_NO_RESET_NOTIFICATION;
}
}
else
{

if (glfwExtensionSupported("GL_EXT_robustness"))
{



GLint strategy;
window->context.GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB,
&strategy);

if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
window->context.robustness = GLFW_LOSE_CONTEXT_ON_RESET;
else if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
window->context.robustness = GLFW_NO_RESET_NOTIFICATION;
}
}

if (glfwExtensionSupported("GL_KHR_context_flush_control"))
{
GLint behavior;
window->context.GetIntegerv(GL_CONTEXT_RELEASE_BEHAVIOR, &behavior);

if (behavior == GL_NONE)
window->context.release = GLFW_RELEASE_BEHAVIOR_NONE;
else if (behavior == GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH)
window->context.release = GLFW_RELEASE_BEHAVIOR_FLUSH;
}



{
PFNGLCLEARPROC glClear = (PFNGLCLEARPROC)
window->context.getProcAddress("glClear");
glClear(GL_COLOR_BUFFER_BIT);
window->context.swapBuffers(window);
}

glfwMakeContextCurrent((GLFWwindow*) previous);
return GLFW_TRUE;
}



GLFWbool _glfwStringInExtensionString(const char* string, const char* extensions)
{
const char* start = extensions;

for (;;)
{
const char* where;
const char* terminator;

where = strstr(start, string);
if (!where)
return GLFW_FALSE;

terminator = where + strlen(string);
if (where == start || *(where - 1) == ' ')
{
if (*terminator == ' ' || *terminator == '\0')
break;
}

start = terminator;
}

return GLFW_TRUE;
}






GLFWAPI void glfwMakeContextCurrent(GLFWwindow* handle)
{
_GLFWwindow* window = (_GLFWwindow*) handle;
_GLFWwindow* previous = _glfwPlatformGetTls(&_glfw.contextSlot);

_GLFW_REQUIRE_INIT();

if (window && window->context.client == GLFW_NO_API)
{
_glfwInputError(GLFW_NO_WINDOW_CONTEXT,
"Cannot make current with a window that has no OpenGL or OpenGL ES context");
return;
}

if (previous)
{
if (!window || window->context.source != previous->context.source)
previous->context.makeCurrent(NULL);
}

if (window)
window->context.makeCurrent(window);
}

GLFWAPI GLFWwindow* glfwGetCurrentContext(void)
{
_GLFW_REQUIRE_INIT_OR_RETURN(NULL);
return _glfwPlatformGetTls(&_glfw.contextSlot);
}

GLFWAPI void glfwSwapBuffers(GLFWwindow* handle)
{
_GLFWwindow* window = (_GLFWwindow*) handle;
assert(window != NULL);

_GLFW_REQUIRE_INIT();

if (window->context.client == GLFW_NO_API)
{
_glfwInputError(GLFW_NO_WINDOW_CONTEXT,
"Cannot swap buffers of a window that has no OpenGL or OpenGL ES context");
return;
}

window->context.swapBuffers(window);
}

GLFWAPI void glfwSwapInterval(int interval)
{
_GLFWwindow* window;

_GLFW_REQUIRE_INIT();

window = _glfwPlatformGetTls(&_glfw.contextSlot);
if (!window)
{
_glfwInputError(GLFW_NO_CURRENT_CONTEXT,
"Cannot set swap interval without a current OpenGL or OpenGL ES context");
return;
}

window->context.swapInterval(interval);
}

GLFWAPI int glfwExtensionSupported(const char* extension)
{
_GLFWwindow* window;
assert(extension != NULL);

_GLFW_REQUIRE_INIT_OR_RETURN(GLFW_FALSE);

window = _glfwPlatformGetTls(&_glfw.contextSlot);
if (!window)
{
_glfwInputError(GLFW_NO_CURRENT_CONTEXT,
"Cannot query extension without a current OpenGL or OpenGL ES context");
return GLFW_FALSE;
}

if (*extension == '\0')
{
_glfwInputError(GLFW_INVALID_VALUE, "Extension name cannot be an empty string");
return GLFW_FALSE;
}

if (window->context.major >= 3)
{
int i;
GLint count;



window->context.GetIntegerv(GL_NUM_EXTENSIONS, &count);

for (i = 0; i < count; i++)
{
const char* en = (const char*)
window->context.GetStringi(GL_EXTENSIONS, i);
if (!en)
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"Extension string retrieval is broken");
return GLFW_FALSE;
}

if (strcmp(en, extension) == 0)
return GLFW_TRUE;
}
}
else
{


const char* extensions = (const char*)
window->context.GetString(GL_EXTENSIONS);
if (!extensions)
{
_glfwInputError(GLFW_PLATFORM_ERROR,
"Extension string retrieval is broken");
return GLFW_FALSE;
}

if (_glfwStringInExtensionString(extension, extensions))
return GLFW_TRUE;
}


return window->context.extensionSupported(extension);
}

GLFWAPI GLFWglproc glfwGetProcAddress(const char* procname)
{
_GLFWwindow* window;
assert(procname != NULL);

_GLFW_REQUIRE_INIT_OR_RETURN(NULL);

window = _glfwPlatformGetTls(&_glfw.contextSlot);
if (!window)
{
_glfwInputError(GLFW_NO_CURRENT_CONTEXT,
"Cannot query entry point without a current OpenGL or OpenGL ES context");
return NULL;
}

return window->context.getProcAddress(procname);
}

