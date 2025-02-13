









#if !defined(RUBY_THREAD_PTHREAD_H)
#define RUBY_THREAD_PTHREAD_H

#if defined(HAVE_PTHREAD_NP_H)
#include <pthread_np.h>
#endif

#define RB_NATIVETHREAD_LOCK_INIT PTHREAD_MUTEX_INITIALIZER
#define RB_NATIVETHREAD_COND_INIT PTHREAD_COND_INITIALIZER

typedef pthread_cond_t rb_nativethread_cond_t;

typedef struct native_thread_data_struct {
union {
struct list_node ubf;
struct list_node gvl;
} node;
#if defined(__GLIBC__) || defined(__FreeBSD__)
union
#else




struct
#endif
{
rb_nativethread_cond_t intr; 
rb_nativethread_cond_t gvlq; 
} cond;
} native_thread_data_t;

#undef except
#undef try
#undef leave
#undef finally

typedef struct rb_global_vm_lock_struct {

const struct rb_thread_struct *owner;
rb_nativethread_lock_t lock; 












struct list_head waitq; 
const struct rb_thread_struct *timer;
int timer_err;


rb_nativethread_cond_t switch_cond;
rb_nativethread_cond_t switch_wait_cond;
int need_yield;
int wait_yield;
} rb_global_vm_lock_t;

#endif 
