#include "../config.h"
enum {
OLD_WIN32CS,
OLD_WIN32MUTEX
};
extern int old_mutex_use;
struct old_mutex_t_ {
HANDLE mutex;
CRITICAL_SECTION cs;
};
typedef struct old_mutex_t_ * old_mutex_t;
struct old_mutexattr_t_ {
int pshared;
};
typedef struct old_mutexattr_t_ * old_mutexattr_t;
extern BOOL (WINAPI *ptw32_try_enter_critical_section)(LPCRITICAL_SECTION);
extern HINSTANCE ptw32_h_kernel32;
#define PTW32_OBJECT_AUTO_INIT ((void *) -1)
void dummy_call(int * a);
void interlocked_inc_with_conditionals(int *a);
void interlocked_dec_with_conditionals(int *a);
int old_mutex_init(old_mutex_t *mutex, const old_mutexattr_t *attr);
int old_mutex_lock(old_mutex_t *mutex);
int old_mutex_unlock(old_mutex_t *mutex);
int old_mutex_trylock(old_mutex_t *mutex);
int old_mutex_destroy(old_mutex_t *mutex);
