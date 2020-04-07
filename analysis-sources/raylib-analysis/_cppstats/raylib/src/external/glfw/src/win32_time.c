#include "internal.h"
void _glfwInitTimerWin32(void)
{
uint64_t frequency;
if (QueryPerformanceFrequency((LARGE_INTEGER*) &frequency))
{
_glfw.timer.win32.hasPC = GLFW_TRUE;
_glfw.timer.win32.frequency = frequency;
}
else
{
_glfw.timer.win32.hasPC = GLFW_FALSE;
_glfw.timer.win32.frequency = 1000;
}
}
uint64_t _glfwPlatformGetTimerValue(void)
{
if (_glfw.timer.win32.hasPC)
{
uint64_t value;
QueryPerformanceCounter((LARGE_INTEGER*) &value);
return value;
}
else
return (uint64_t) timeGetTime();
}
uint64_t _glfwPlatformGetTimerFrequency(void)
{
return _glfw.timer.win32.frequency;
}
