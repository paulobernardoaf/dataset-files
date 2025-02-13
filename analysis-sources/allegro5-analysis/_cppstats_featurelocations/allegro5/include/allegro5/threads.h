














#if !defined(__al_included_allegro5_threads_h)
#define __al_included_allegro5_threads_h

#include "allegro5/altime.h"

#if defined(__cplusplus)
extern "C" {
#endif



typedef struct ALLEGRO_THREAD ALLEGRO_THREAD;



typedef struct ALLEGRO_MUTEX ALLEGRO_MUTEX;



typedef struct ALLEGRO_COND ALLEGRO_COND;


AL_FUNC(ALLEGRO_THREAD *, al_create_thread,
(void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg));
#if defined(ALLEGRO_UNSTABLE) || defined(ALLEGRO_INTERNAL_UNSTABLE) || defined(ALLEGRO_SRC)
AL_FUNC(ALLEGRO_THREAD *, al_create_thread_with_stacksize,
(void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg, size_t stacksize));
#endif
AL_FUNC(void, al_start_thread, (ALLEGRO_THREAD *outer));
AL_FUNC(void, al_join_thread, (ALLEGRO_THREAD *outer, void **ret_value));
AL_FUNC(void, al_set_thread_should_stop, (ALLEGRO_THREAD *outer));
AL_FUNC(bool, al_get_thread_should_stop, (ALLEGRO_THREAD *outer));
AL_FUNC(void, al_destroy_thread, (ALLEGRO_THREAD *thread));
AL_FUNC(void, al_run_detached_thread, (void *(*proc)(void *arg), void *arg));

AL_FUNC(ALLEGRO_MUTEX *, al_create_mutex, (void));
AL_FUNC(ALLEGRO_MUTEX *, al_create_mutex_recursive, (void));
AL_FUNC(void, al_lock_mutex, (ALLEGRO_MUTEX *mutex));
AL_FUNC(void, al_unlock_mutex, (ALLEGRO_MUTEX *mutex));
AL_FUNC(void, al_destroy_mutex, (ALLEGRO_MUTEX *mutex));

AL_FUNC(ALLEGRO_COND *, al_create_cond, (void));
AL_FUNC(void, al_destroy_cond, (ALLEGRO_COND *cond));
AL_FUNC(void, al_wait_cond, (ALLEGRO_COND *cond, ALLEGRO_MUTEX *mutex));
AL_FUNC(int, al_wait_cond_until, (ALLEGRO_COND *cond, ALLEGRO_MUTEX *mutex,
const ALLEGRO_TIMEOUT *timeout));
AL_FUNC(void, al_broadcast_cond, (ALLEGRO_COND *cond));
AL_FUNC(void, al_signal_cond, (ALLEGRO_COND *cond));

#if defined(__cplusplus)
}
#endif

#endif


