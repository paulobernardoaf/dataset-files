#include "internal.h"
int _glfwPlatformInit(void)
{
_glfwInitTimerPOSIX();
return GLFW_TRUE;
}
void _glfwPlatformTerminate(void)
{
_glfwTerminateOSMesa();
}
const char* _glfwPlatformGetVersionString(void)
{
return _GLFW_VERSION_NUMBER " null OSMesa";
}
