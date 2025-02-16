#if !defined(__al_included_allegro5_aintern_thread_h)
#define __al_included_allegro5_aintern_thread_h

#include ALLEGRO_INTERNAL_THREAD_HEADER

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct _AL_THREAD _AL_THREAD;
typedef struct _AL_MUTEX _AL_MUTEX;
typedef struct _AL_COND _AL_COND;


void _al_thread_create(_AL_THREAD*, void (*proc)(_AL_THREAD*, void*), void *arg);
void _al_thread_create_with_stacksize(_AL_THREAD*, void (*proc)(_AL_THREAD*, void*), void *arg, size_t stacksize);
void _al_thread_set_should_stop(_AL_THREAD *);

void _al_thread_join(_AL_THREAD*);
void _al_thread_detach(_AL_THREAD*);


void _al_mutex_init(_AL_MUTEX*);
void _al_mutex_init_recursive(_AL_MUTEX*);
void _al_mutex_destroy(_AL_MUTEX*);







#if defined(ALLEGRO_WINDOWS)
void _al_cond_init(_AL_COND*);
void _al_cond_destroy(_AL_COND*);
void _al_cond_wait(_AL_COND*, _AL_MUTEX*);
void _al_cond_broadcast(_AL_COND*);
void _al_cond_signal(_AL_COND*);
#endif

int _al_cond_timedwait(_AL_COND*, _AL_MUTEX*, const ALLEGRO_TIMEOUT *timeout);


#if defined(__cplusplus)
}
#endif

#endif


