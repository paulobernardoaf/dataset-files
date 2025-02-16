#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "nvim/assert.h"
#include "nvim/profile.h"
#include "nvim/os/time.h"
#include "nvim/func_attr.h"
#include "nvim/os/os_defs.h"
#include "nvim/globals.h" 
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "profile.c.generated.h"
#endif
static proftime_T prof_wait_time;
proftime_T profile_start(void) FUNC_ATTR_WARN_UNUSED_RESULT
{
return os_hrtime();
}
proftime_T profile_end(proftime_T tm) FUNC_ATTR_WARN_UNUSED_RESULT
{
return profile_sub(os_hrtime(), tm);
}
const char *profile_msg(proftime_T tm) FUNC_ATTR_WARN_UNUSED_RESULT
{
static char buf[50];
snprintf(buf, sizeof(buf), "%10.6lf",
(double)profile_signed(tm) / 1000000000.0);
return buf;
}
proftime_T profile_setlimit(int64_t msec) FUNC_ATTR_WARN_UNUSED_RESULT
{
if (msec <= 0) {
return profile_zero();
}
assert(msec <= (INT64_MAX / 1000000LL) - 1);
proftime_T nsec = (proftime_T)msec * 1000000ULL;
return os_hrtime() + nsec;
}
bool profile_passed_limit(proftime_T tm) FUNC_ATTR_WARN_UNUSED_RESULT
{
if (tm == 0) {
return false;
}
return profile_cmp(os_hrtime(), tm) < 0;
}
proftime_T profile_zero(void) FUNC_ATTR_CONST
{
return 0;
}
proftime_T profile_divide(proftime_T tm, int count) FUNC_ATTR_CONST
{
if (count <= 0) {
return profile_zero();
}
return (proftime_T) round((double) tm / (double) count);
}
proftime_T profile_add(proftime_T tm1, proftime_T tm2) FUNC_ATTR_CONST
{
return tm1 + tm2;
}
proftime_T profile_sub(proftime_T tm1, proftime_T tm2) FUNC_ATTR_CONST
{
return tm1 - tm2;
}
proftime_T profile_self(proftime_T self, proftime_T total, proftime_T children)
FUNC_ATTR_CONST
{
if (total <= children) {
return self;
}
return profile_sub(profile_add(self, total), children);
}
proftime_T profile_get_wait(void) FUNC_ATTR_PURE
{
return prof_wait_time;
}
void profile_set_wait(proftime_T wait)
{
prof_wait_time = wait;
}
proftime_T profile_sub_wait(proftime_T tm, proftime_T tma) FUNC_ATTR_PURE
{
proftime_T tm3 = profile_sub(profile_get_wait(), tm);
return profile_sub(tma, tm3);
}
bool profile_equal(proftime_T tm1, proftime_T tm2) FUNC_ATTR_CONST
{
return tm1 == tm2;
}
int64_t profile_signed(proftime_T tm)
FUNC_ATTR_CONST
{
return (tm <= INT64_MAX) ? (int64_t)tm : -(int64_t)(UINT64_MAX - tm);
}
int profile_cmp(proftime_T tm1, proftime_T tm2) FUNC_ATTR_CONST
{
if (tm1 == tm2) {
return 0;
}
return profile_signed(tm2 - tm1) < 0 ? -1 : 1;
}
static proftime_T g_start_time;
static proftime_T g_prev_time;
void time_push(proftime_T *rel, proftime_T *start)
{
proftime_T now = profile_start();
*rel = profile_sub(now, g_prev_time);
*start = now;
g_prev_time = now;
}
void time_pop(proftime_T tp)
{
g_prev_time -= tp;
}
static void time_diff(proftime_T then, proftime_T now)
{
proftime_T diff = profile_sub(now, then);
fprintf(time_fd, "%07.3lf", (double)diff / 1.0E6);
}
void time_start(const char *message)
{
if (time_fd == NULL) {
return;
}
g_prev_time = g_start_time = profile_start();
fprintf(time_fd, "\n\ntimes in msec\n");
fprintf(time_fd, " clock self+sourced self: sourced script\n");
fprintf(time_fd, " clock elapsed: other lines\n\n");
time_msg(message, NULL);
}
void time_msg(const char *mesg, const proftime_T *start)
{
if (time_fd == NULL) {
return;
}
proftime_T now = profile_start();
time_diff(g_start_time, now);
if (start != NULL) {
fprintf(time_fd, " ");
time_diff(*start, now);
}
fprintf(time_fd, " ");
time_diff(g_prev_time, now);
g_prev_time = now;
fprintf(time_fd, ": %s\n", mesg);
}
