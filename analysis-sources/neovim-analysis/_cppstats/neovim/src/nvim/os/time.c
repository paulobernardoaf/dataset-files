#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <uv.h>
#include "nvim/assert.h"
#include "nvim/os/time.h"
#include "nvim/os/input.h"
#include "nvim/event/loop.h"
#include "nvim/vim.h"
#include "nvim/main.h"
static uv_mutex_t delay_mutex;
static uv_cond_t delay_cond;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "os/time.c.generated.h"
#endif
void time_init(void)
{
uv_mutex_init(&delay_mutex);
uv_cond_init(&delay_cond);
}
uint64_t os_hrtime(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
return uv_hrtime();
}
uint64_t os_now(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
return uv_now(&main_loop.uv);
}
void os_delay(uint64_t ms, bool ignoreinput)
{
if (ignoreinput) {
if (ms > INT_MAX) {
ms = INT_MAX;
}
LOOP_PROCESS_EVENTS_UNTIL(&main_loop, NULL, (int)ms, got_int);
} else {
os_microdelay(ms * 1000u, ignoreinput);
}
}
void os_microdelay(uint64_t us, bool ignoreinput)
{
uint64_t elapsed = 0u;
uint64_t base = uv_hrtime();
const uint64_t ns = (us < UINT64_MAX / 1000u) ? us * 1000u : UINT64_MAX;
uv_mutex_lock(&delay_mutex);
while (elapsed < ns) {
const uint64_t ns_delta = ignoreinput
? ns - elapsed
: MIN(ns - elapsed, 100000000u); 
const int rv = uv_cond_timedwait(&delay_cond, &delay_mutex, ns_delta);
if (0 != rv && UV_ETIMEDOUT != rv) {
assert(false);
break;
} 
if (!ignoreinput && os_char_avail()) {
break;
}
const uint64_t now = uv_hrtime();
elapsed += now - base;
base = now;
}
uv_mutex_unlock(&delay_mutex);
}
struct tm *os_localtime_r(const time_t *restrict clock,
struct tm *restrict result) FUNC_ATTR_NONNULL_ALL
{
#if defined(UNIX)
return localtime_r(clock, result); 
#else
struct tm *local_time = localtime(clock); 
if (!local_time) {
return NULL;
}
*result = *local_time;
return result;
#endif
}
struct tm *os_localtime(struct tm *result) FUNC_ATTR_NONNULL_ALL
{
time_t rawtime = time(NULL);
return os_localtime_r(&rawtime, result);
}
Timestamp os_time(void)
FUNC_ATTR_WARN_UNUSED_RESULT
{
return (Timestamp) time(NULL);
}
