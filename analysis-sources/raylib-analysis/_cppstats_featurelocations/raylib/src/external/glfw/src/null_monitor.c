




























#include "internal.h"






void _glfwPlatformFreeMonitor(_GLFWmonitor* monitor)
{
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
}

void _glfwPlatformGetMonitorContentScale(_GLFWmonitor* monitor,
float* xscale, float* yscale)
{
if (xscale)
*xscale = 1.f;
if (yscale)
*yscale = 1.f;
}

void _glfwPlatformGetMonitorWorkarea(_GLFWmonitor* monitor,
int* xpos, int* ypos,
int* width, int* height)
{
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
return NULL;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
}

GLFWbool _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
return GLFW_FALSE;
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
}

