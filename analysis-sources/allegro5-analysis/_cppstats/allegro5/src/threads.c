#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_thread.h"
#include "allegro5/internal/aintern_system.h"
typedef enum THREAD_STATE {
THREAD_STATE_CREATED, 
THREAD_STATE_STARTING, 
THREAD_STATE_STARTED, 
THREAD_STATE_JOINING, 
THREAD_STATE_JOINED, 
THREAD_STATE_DESTROYED,
THREAD_STATE_DETACHED
} THREAD_STATE;
struct ALLEGRO_THREAD {
_AL_THREAD thread;
_AL_MUTEX mutex;
_AL_COND cond;
THREAD_STATE thread_state;
void *proc;
void *arg;
void *retval;
};
struct ALLEGRO_MUTEX {
_AL_MUTEX mutex;
};
struct ALLEGRO_COND {
_AL_COND cond;
};
static void thread_func_trampoline(_AL_THREAD *inner, void *_outer)
{
ALLEGRO_THREAD *outer = (ALLEGRO_THREAD *) _outer;
ALLEGRO_SYSTEM *system = al_get_system_driver();
(void)inner;
if (system && system->vt && system->vt->thread_init) {
system->vt->thread_init(outer);
}
_al_mutex_lock(&outer->mutex);
while (outer->thread_state == THREAD_STATE_CREATED) {
_al_cond_wait(&outer->cond, &outer->mutex);
}
_al_mutex_unlock(&outer->mutex);
if (outer->thread_state == THREAD_STATE_STARTING) {
outer->thread_state = THREAD_STATE_STARTED;
outer->retval =
((void *(*)(ALLEGRO_THREAD *, void *))outer->proc)(outer, outer->arg);
}
if (system && system->vt && system->vt->thread_exit) {
system->vt->thread_exit(outer);
}
}
static void detached_thread_func_trampoline(_AL_THREAD *inner, void *_outer)
{
ALLEGRO_THREAD *outer = (ALLEGRO_THREAD *) _outer;
(void)inner;
((void *(*)(void *))outer->proc)(outer->arg);
al_free(outer);
}
static ALLEGRO_THREAD *create_thread(void)
{
ALLEGRO_THREAD *outer;
outer = al_malloc(sizeof(*outer));
if (!outer) {
return NULL;
}
_AL_MARK_MUTEX_UNINITED(outer->mutex); 
outer->retval = NULL;
return outer;
}
ALLEGRO_THREAD *al_create_thread(
void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg)
{
ALLEGRO_THREAD *outer = create_thread();
outer->thread_state = THREAD_STATE_CREATED;
_al_mutex_init(&outer->mutex);
_al_cond_init(&outer->cond);
outer->arg = arg;
outer->proc = proc;
_al_thread_create(&outer->thread, thread_func_trampoline, outer);
return outer;
}
ALLEGRO_THREAD *al_create_thread_with_stacksize(
void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg, size_t stacksize)
{
ALLEGRO_THREAD *outer = create_thread();
outer->thread_state = THREAD_STATE_CREATED;
_al_mutex_init(&outer->mutex);
_al_cond_init(&outer->cond);
outer->arg = arg;
outer->proc = proc;
_al_thread_create_with_stacksize(&outer->thread, thread_func_trampoline, outer, stacksize);
return outer;
}
void al_run_detached_thread(void *(*proc)(void *arg), void *arg)
{
ALLEGRO_THREAD *outer = create_thread();
outer->thread_state = THREAD_STATE_DETACHED;
outer->arg = arg;
outer->proc = proc;
_al_thread_create(&outer->thread, detached_thread_func_trampoline, outer);
_al_thread_detach(&outer->thread);
}
void al_start_thread(ALLEGRO_THREAD *thread)
{
ASSERT(thread);
switch (thread->thread_state) {
case THREAD_STATE_CREATED:
_al_mutex_lock(&thread->mutex);
thread->thread_state = THREAD_STATE_STARTING;
_al_cond_broadcast(&thread->cond);
_al_mutex_unlock(&thread->mutex);
break;
case THREAD_STATE_STARTING:
break;
case THREAD_STATE_STARTED:
break;
case THREAD_STATE_JOINING:
ASSERT(thread->thread_state != THREAD_STATE_JOINING);
break;
case THREAD_STATE_JOINED:
ASSERT(thread->thread_state != THREAD_STATE_JOINED);
break;
case THREAD_STATE_DESTROYED:
ASSERT(thread->thread_state != THREAD_STATE_DESTROYED);
break;
case THREAD_STATE_DETACHED:
ASSERT(thread->thread_state != THREAD_STATE_DETACHED);
break;
}
}
void al_join_thread(ALLEGRO_THREAD *thread, void **ret_value)
{
ASSERT(thread);
while (thread->thread_state == THREAD_STATE_STARTING) {
al_rest(0.001);
}
switch (thread->thread_state) {
case THREAD_STATE_CREATED: 
case THREAD_STATE_STARTED:
_al_mutex_lock(&thread->mutex);
thread->thread_state = THREAD_STATE_JOINING;
_al_cond_broadcast(&thread->cond);
_al_mutex_unlock(&thread->mutex);
_al_cond_destroy(&thread->cond);
_al_mutex_destroy(&thread->mutex);
_al_thread_join(&thread->thread);
thread->thread_state = THREAD_STATE_JOINED;
break;
case THREAD_STATE_STARTING:
ASSERT(thread->thread_state != THREAD_STATE_STARTING);
break;
case THREAD_STATE_JOINING:
ASSERT(thread->thread_state != THREAD_STATE_JOINING);
break;
case THREAD_STATE_JOINED:
ASSERT(thread->thread_state != THREAD_STATE_JOINED);
break;
case THREAD_STATE_DESTROYED:
ASSERT(thread->thread_state != THREAD_STATE_DESTROYED);
break;
case THREAD_STATE_DETACHED:
ASSERT(thread->thread_state != THREAD_STATE_DETACHED);
break;
}
if (ret_value) {
*ret_value = thread->retval;
}
}
void al_set_thread_should_stop(ALLEGRO_THREAD *thread)
{
ASSERT(thread);
_al_thread_set_should_stop(&thread->thread);
}
bool al_get_thread_should_stop(ALLEGRO_THREAD *thread)
{
ASSERT(thread);
return _al_get_thread_should_stop(&thread->thread);
}
void al_destroy_thread(ALLEGRO_THREAD *thread)
{
if (!thread) {
return;
}
switch (thread->thread_state) {
case THREAD_STATE_CREATED: 
case THREAD_STATE_STARTING: 
case THREAD_STATE_STARTED:
al_join_thread(thread, NULL);
break;
case THREAD_STATE_JOINING:
ASSERT(thread->thread_state != THREAD_STATE_JOINING);
break;
case THREAD_STATE_JOINED:
break;
case THREAD_STATE_DESTROYED:
ASSERT(thread->thread_state != THREAD_STATE_DESTROYED);
break;
case THREAD_STATE_DETACHED:
ASSERT(thread->thread_state != THREAD_STATE_DETACHED);
break;
}
thread->thread_state = THREAD_STATE_DESTROYED;
al_free(thread);
}
ALLEGRO_MUTEX *al_create_mutex(void)
{
ALLEGRO_MUTEX *mutex = al_malloc(sizeof(*mutex));
if (mutex) {
_AL_MARK_MUTEX_UNINITED(mutex->mutex);
_al_mutex_init(&mutex->mutex);
}
return mutex;
}
ALLEGRO_MUTEX *al_create_mutex_recursive(void)
{
ALLEGRO_MUTEX *mutex = al_malloc(sizeof(*mutex));
if (mutex) {
_AL_MARK_MUTEX_UNINITED(mutex->mutex);
_al_mutex_init_recursive(&mutex->mutex);
}
return mutex;
}
void al_lock_mutex(ALLEGRO_MUTEX *mutex)
{
ASSERT(mutex);
_al_mutex_lock(&mutex->mutex);
}
void al_unlock_mutex(ALLEGRO_MUTEX *mutex)
{
ASSERT(mutex);
_al_mutex_unlock(&mutex->mutex);
}
void al_destroy_mutex(ALLEGRO_MUTEX *mutex)
{
if (mutex) {
_al_mutex_destroy(&mutex->mutex);
al_free(mutex);
}
}
ALLEGRO_COND *al_create_cond(void)
{
ALLEGRO_COND *cond = al_malloc(sizeof(*cond));
if (cond) {
_al_cond_init(&cond->cond);
}
return cond;
}
void al_destroy_cond(ALLEGRO_COND *cond)
{
if (cond) {
_al_cond_destroy(&cond->cond);
al_free(cond);
}
}
void al_wait_cond(ALLEGRO_COND *cond, ALLEGRO_MUTEX *mutex)
{
ASSERT(cond);
ASSERT(mutex);
_al_cond_wait(&cond->cond, &mutex->mutex);
}
int al_wait_cond_until(ALLEGRO_COND *cond, ALLEGRO_MUTEX *mutex,
const ALLEGRO_TIMEOUT *timeout)
{
ASSERT(cond);
ASSERT(mutex);
ASSERT(timeout);
return _al_cond_timedwait(&cond->cond, &mutex->mutex, timeout);
}
void al_broadcast_cond(ALLEGRO_COND *cond)
{
ASSERT(cond);
_al_cond_broadcast(&cond->cond);
}
void al_signal_cond(ALLEGRO_COND *cond)
{
ASSERT(cond);
_al_cond_signal(&cond->cond);
}
