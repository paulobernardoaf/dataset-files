#if !defined( SEMAPHORE_H )
#define SEMAPHORE_H
#undef PTW32_SEMAPHORE_LEVEL
#if defined(_POSIX_SOURCE)
#define PTW32_SEMAPHORE_LEVEL 0
#endif
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309
#undef PTW32_SEMAPHORE_LEVEL
#define PTW32_SEMAPHORE_LEVEL 1
#endif
#if defined(INCLUDE_NP)
#undef PTW32_SEMAPHORE_LEVEL
#define PTW32_SEMAPHORE_LEVEL 2
#endif
#define PTW32_SEMAPHORE_LEVEL_MAX 3
#if !defined(PTW32_SEMAPHORE_LEVEL)
#define PTW32_SEMAPHORE_LEVEL PTW32_SEMAPHORE_LEVEL_MAX
#endif
#if defined(__GNUC__) && ! defined (__declspec)
#error Please upgrade your GNU compiler to one that supports __declspec.
#endif
#if !defined(PTW32_STATIC_LIB)
#if defined(PTW32_BUILD)
#define PTW32_DLLPORT __declspec (dllexport)
#else
#define PTW32_DLLPORT
#endif
#else
#define PTW32_DLLPORT
#endif
#if !defined(PTW32_CONFIG_H)
#if defined(WINCE)
#define NEED_ERRNO
#define NEED_SEM
#endif
#if defined(__MINGW64__)
#define HAVE_STRUCT_TIMESPEC
#define HAVE_MODE_T
#elif defined(_UWIN) || defined(__MINGW32__)
#define HAVE_MODE_T
#endif
#endif
#if PTW32_SEMAPHORE_LEVEL >= PTW32_SEMAPHORE_LEVEL_MAX
#if defined(NEED_ERRNO)
#include "need_errno.h"
#else
#include <errno.h>
#endif
#endif 
#define _POSIX_SEMAPHORES
#if defined(__cplusplus)
extern "C"
{
#endif 
#if !defined(HAVE_MODE_T)
typedef unsigned int mode_t;
#endif
typedef struct sem_t_ * sem_t;
PTW32_DLLPORT int __cdecl sem_init (sem_t * sem,
int pshared,
unsigned int value);
PTW32_DLLPORT int __cdecl sem_destroy (sem_t * sem);
PTW32_DLLPORT int __cdecl sem_trywait (sem_t * sem);
PTW32_DLLPORT int __cdecl sem_wait (sem_t * sem);
PTW32_DLLPORT int __cdecl sem_timedwait (sem_t * sem,
const struct timespec * abstime);
PTW32_DLLPORT int __cdecl sem_post (sem_t * sem);
PTW32_DLLPORT int __cdecl sem_post_multiple (sem_t * sem,
int count);
PTW32_DLLPORT int __cdecl sem_open (const char * name,
int oflag,
mode_t mode,
unsigned int value);
PTW32_DLLPORT int __cdecl sem_close (sem_t * sem);
PTW32_DLLPORT int __cdecl sem_unlink (const char * name);
PTW32_DLLPORT int __cdecl sem_getvalue (sem_t * sem,
int * sval);
#if defined(__cplusplus)
} 
#endif 
#undef PTW32_SEMAPHORE_LEVEL
#undef PTW32_SEMAPHORE_LEVEL_MAX
#endif 
