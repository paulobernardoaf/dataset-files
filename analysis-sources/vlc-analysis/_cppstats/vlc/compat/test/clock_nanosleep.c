#include "config.h"
#include <stdbool.h>
#include <unistd.h>
#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/errno.h>
#define VLC_NSEC_PER_SEC 1000000000ull
#define VLC_TEST(id, flags, tp, eret, eerr) do { assert( clock_nanosleep(id, flags, tp, NULL) == eret ); if (eret != 0) assert( eerr == errno ); } while (0)
static void *waiting_thread_entry(void *unused)
{
(void)unused;
struct timespec ts = { 5, 0 };
assert(clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL) == 0);
assert(0);
return NULL;
}
int main(void)
{
alarm(1);
{
struct timespec inv_ts = { -1, -1 };
VLC_TEST(CLOCK_MONOTONIC, 0, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_MONOTONIC, TIMER_ABSTIME, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_REALTIME, 0, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_REALTIME, TIMER_ABSTIME, &inv_ts, -1, EINVAL);
}
{
struct timespec inv_ts = { -1, VLC_NSEC_PER_SEC };
VLC_TEST(CLOCK_MONOTONIC, 0, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_MONOTONIC, TIMER_ABSTIME, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_REALTIME, 0, &inv_ts, -1, EINVAL);
VLC_TEST(CLOCK_REALTIME, TIMER_ABSTIME, &inv_ts, -1, EINVAL);
}
{
struct timespec ts = { 0, 0 };
VLC_TEST(CLOCK_MONOTONIC, 0, &ts, 0, 0);
VLC_TEST(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0, 0);
VLC_TEST(CLOCK_REALTIME, 0, &ts, 0, 0);
VLC_TEST(CLOCK_REALTIME, TIMER_ABSTIME, &ts, 0, 0);
}
{
alarm(3);
struct timespec ts = { 0, VLC_NSEC_PER_SEC - 1 };
VLC_TEST(CLOCK_MONOTONIC, 0, &ts, 0, 0);
VLC_TEST(CLOCK_REALTIME, 0, &ts, 0, 0);
}
{
alarm(3);
struct timespec ts, ts_now;
clock_gettime(CLOCK_MONOTONIC, &ts);
ts.tv_sec += 1;
VLC_TEST(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0, 0);
clock_gettime(CLOCK_MONOTONIC, &ts_now);
assert(ts.tv_sec <= ts_now.tv_sec);
}
{
alarm(1);
struct timespec ts = { 10, 10 };
VLC_TEST(CLOCK_REALTIME, TIMER_ABSTIME, &ts, 0, 0);
}
{
alarm(1);
pthread_t th;
pthread_create(&th, NULL, waiting_thread_entry, NULL);
pthread_cancel(th);
pthread_join(th, NULL);
}
}
