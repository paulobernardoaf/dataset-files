#define _POSIX_C_SOURCE 199309L
#include "internal.h"
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
void _glfwInitTimerPOSIX(void)
{
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
struct timespec ts;
if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
{
_glfw.timer.posix.monotonic = GLFW_TRUE;
_glfw.timer.posix.frequency = 1000000000;
}
else
#endif
{
_glfw.timer.posix.monotonic = GLFW_FALSE;
_glfw.timer.posix.frequency = 1000000;
}
}
uint64_t _glfwPlatformGetTimerValue(void)
{
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
if (_glfw.timer.posix.monotonic)
{
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
}
else
#endif
{
struct timeval tv;
gettimeofday(&tv, NULL);
return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
}
}
uint64_t _glfwPlatformGetTimerFrequency(void)
{
return _glfw.timer.posix.frequency;
}
