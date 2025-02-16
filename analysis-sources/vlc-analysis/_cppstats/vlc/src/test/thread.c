#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#undef NDEBUG
#include <assert.h>
#include <vlc_common.h>
static int thread_data_magic = 0x1234;
static int thread_return_magic = 0x4321;
#define TEST_THREAD_JOIN(th, eret) do { void *r; vlc_join(th, &r); assert(r == eret); } while (0)
#define TEST_THREAD_CANCEL_JOIN(th) do { vlc_cancel(th); TEST_THREAD_JOIN(th, VLC_THREAD_CANCELED); } while (0)
#define TEST_THREAD_JOIN_MAGIC(th) TEST_THREAD_JOIN(th, &thread_return_magic)
#define TEST_THREAD_CLONE(th, f, data) assert(vlc_clone(th, f, data, VLC_THREAD_PRIORITY_LOW) == 0)
#define TEST_THREAD_CLONE_MAGIC(th, f) TEST_THREAD_CLONE(th, f, &thread_data_magic)
struct cond_data
{
int state; 
vlc_mutex_t mutex;
vlc_cond_t cond_started; 
vlc_cond_t cond_end; 
};
static void cond_data_init(struct cond_data *data, int count)
{
assert(count > 0);
data->state = count;
vlc_mutex_init(&data->mutex);
vlc_cond_init(&data->cond_started);
vlc_cond_init(&data->cond_end);
}
static void *thread_func_noop(void *data)
{
assert(data == &thread_data_magic);
return &thread_return_magic;
}
static void test__thread_create()
{
vlc_thread_t th;
TEST_THREAD_CLONE_MAGIC(&th, thread_func_noop);
TEST_THREAD_JOIN_MAGIC(th);
}
static void *thread_func_loop(void *data)
{
assert(data == &thread_data_magic);
while(1) {
sleep(10);
}
return &thread_return_magic;
}
static void test__thread_cancelation()
{
vlc_thread_t th;
TEST_THREAD_CLONE_MAGIC(&th, thread_func_loop);
TEST_THREAD_CANCEL_JOIN(th);
}
static void thread_func_cleanup(void *data)
{
int *cleanup_state = data;
assert(*cleanup_state == 0);
*cleanup_state = 1;
}
static void *thread_func_loop_cleanup(void *data)
{
int *cleanup_state = data;
assert(*cleanup_state == 0);
vlc_cleanup_push(thread_func_cleanup, data);
while(1) {
vlc_testcancel();
}
vlc_cleanup_pop();
return &thread_return_magic;
}
static void test__thread_cancelation_with_cleanup()
{
vlc_thread_t th;
int th_cleanup_state = 0;
TEST_THREAD_CLONE(&th, thread_func_loop_cleanup, &th_cleanup_state);
TEST_THREAD_CANCEL_JOIN(th);
assert(th_cleanup_state == 1);
}
static void *thread_func_cond(void *ptr)
{
struct cond_data *data = ptr;
vlc_mutex_lock(&data->mutex);
data->state -= 1;
vlc_cond_signal(&data->cond_started);
vlc_mutex_unlock(&data->mutex);
vlc_mutex_lock(&data->mutex);
mutex_cleanup_push(&data->mutex);
while(data->state != -1) {
vlc_cond_wait(&data->cond_end, &data->mutex);
}
vlc_cleanup_pop();
vlc_mutex_unlock(&data->mutex);
return &thread_return_magic;
}
static void test__cond_broadcast()
{
struct cond_data data;
vlc_thread_t threads[20];
cond_data_init(&data, ARRAY_SIZE(threads));
for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
TEST_THREAD_CLONE(&threads[i], thread_func_cond, &data);
}
vlc_mutex_lock(&data.mutex);
while(data.state > 0) {
vlc_cond_wait(&data.cond_started, &data.mutex);
}
vlc_mutex_unlock(&data.mutex);
vlc_mutex_lock(&data.mutex);
data.state = -1;
vlc_cond_broadcast(&data.cond_end);
vlc_mutex_unlock(&data.mutex);
for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
TEST_THREAD_JOIN_MAGIC(threads[i]);
}
}
static void test__cond_wait()
{
struct cond_data data;
cond_data_init(&data, 1);
vlc_thread_t th;
TEST_THREAD_CLONE(&th, thread_func_cond, &data);
vlc_mutex_lock(&data.mutex);
while(data.state != 0) {
vlc_cond_wait(&data.cond_started, &data.mutex);
}
vlc_mutex_unlock(&data.mutex);
vlc_mutex_lock(&data.mutex);
data.state = -1;
vlc_cond_signal(&data.cond_end);
vlc_mutex_unlock(&data.mutex);
TEST_THREAD_JOIN_MAGIC(th);
}
static void test__cond_wait_cancelation_with_cleanup()
{
struct cond_data data;
cond_data_init(&data, 1);
vlc_thread_t th;
TEST_THREAD_CLONE(&th, thread_func_cond, &data);
vlc_mutex_lock(&data.mutex);
while(data.state != 0) {
vlc_cond_wait(&data.cond_started, &data.mutex);
}
vlc_mutex_unlock(&data.mutex);
TEST_THREAD_CANCEL_JOIN(th);
}
static void *thread_func_cond_timeout(void *ptr)
{
struct cond_data *data = ptr;
vlc_mutex_lock(&data->mutex);
data->state -= 1;
vlc_cond_signal(&data->cond_started);
vlc_mutex_unlock(&data->mutex);
vlc_mutex_lock(&data->mutex);
mutex_cleanup_push(&data->mutex);
vlc_tick_t now = vlc_tick_now();
vlc_tick_t deadline = now + VLC_TICK_FROM_MS(25);
int ret = vlc_cond_timedwait(&data->cond_end, &data->mutex, deadline);
assert(ret == ETIMEDOUT);
vlc_tick_t elapsed = vlc_tick_now() - now;
assert(elapsed >= VLC_TICK_FROM_MS(25));
vlc_cleanup_pop();
vlc_mutex_unlock(&data->mutex);
return &thread_return_magic;
}
static void test__cond_wait_timeout()
{
struct cond_data data;
cond_data_init(&data, 1);
vlc_thread_t th;
TEST_THREAD_CLONE(&th, thread_func_cond_timeout, &data);
vlc_mutex_lock(&data.mutex);
while(data.state != 0) {
vlc_cond_wait(&data.cond_started, &data.mutex);
}
vlc_mutex_unlock(&data.mutex);
TEST_THREAD_JOIN_MAGIC(th);
}
static void *thread_func_tick_sleep(void *data)
{
assert(data == &thread_data_magic);
vlc_tick_sleep(VLC_TICK_FROM_SEC(10));
return &thread_return_magic;
}
static void test__vlc_tick_sleep_cancelation()
{
vlc_thread_t th;
TEST_THREAD_CLONE_MAGIC(&th, thread_func_tick_sleep);
TEST_THREAD_CANCEL_JOIN(th);
}
static void test__vlc_tick_sleep()
{
vlc_tick_t now = vlc_tick_now();
vlc_tick_sleep(VLC_TICK_FROM_MS(25));
vlc_tick_t elapsed = vlc_tick_now() - now;
assert(elapsed >= VLC_TICK_FROM_MS(25));
}
int main(void)
{
alarm(4);
test__thread_create();
test__thread_cancelation();
test__thread_cancelation_with_cleanup();
test__cond_wait();
test__cond_wait_cancelation_with_cleanup();
test__cond_wait_timeout();
test__cond_broadcast();
test__vlc_tick_sleep_cancelation();
test__vlc_tick_sleep();
return 0;
}
