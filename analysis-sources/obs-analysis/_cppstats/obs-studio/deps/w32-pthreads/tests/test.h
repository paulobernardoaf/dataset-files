#include "pthread.h"
#include "sched.h"
#include "semaphore.h"
#include <windows.h>
#include <stdio.h>
#define PTW32_THREAD_NULL_ID {NULL,0}
#define rand_r( _seed ) ( _seed == _seed? rand() : rand() )
#if defined(__MINGW32__)
#include <stdint.h>
#elif defined(__BORLANDC__)
#define int64_t ULONGLONG
#else
#define int64_t _int64
#endif
#if defined(_MSC_VER) && _MSC_VER >= 1400
#define PTW32_FTIME(x) _ftime64_s(x)
#define PTW32_STRUCT_TIMEB struct __timeb64
#elif ( defined(_MSC_VER) && _MSC_VER >= 1300 ) || ( defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601 )
#define PTW32_FTIME(x) _ftime64(x)
#define PTW32_STRUCT_TIMEB struct __timeb64
#else
#define PTW32_FTIME(x) _ftime(x)
#define PTW32_STRUCT_TIMEB struct _timeb
#endif
const char * error_string[] = {
"ZERO_or_EOK",
"EPERM",
"ENOFILE_or_ENOENT",
"ESRCH",
"EINTR",
"EIO",
"ENXIO",
"E2BIG",
"ENOEXEC",
"EBADF",
"ECHILD",
"EAGAIN",
"ENOMEM",
"EACCES",
"EFAULT",
"UNKNOWN_15",
"EBUSY",
"EEXIST",
"EXDEV",
"ENODEV",
"ENOTDIR",
"EISDIR",
"EINVAL",
"ENFILE",
"EMFILE",
"ENOTTY",
"UNKNOWN_26",
"EFBIG",
"ENOSPC",
"ESPIPE",
"EROFS",
"EMLINK",
"EPIPE",
"EDOM",
"ERANGE",
"UNKNOWN_35",
"EDEADLOCK_or_EDEADLK",
"UNKNOWN_37",
"ENAMETOOLONG",
"ENOLCK",
"ENOSYS",
"ENOTEMPTY",
"EILSEQ",
"EOWNERDEAD",
"ENOTRECOVERABLE"
};
#if defined(assert)
#undef assert
#endif
#if !defined(ASSERT_TRACE)
#define ASSERT_TRACE 0
#else
#undef ASSERT_TRACE
#define ASSERT_TRACE 1
#endif
#define assert(e) ((e) ? ((ASSERT_TRACE) ? fprintf(stderr, "Assertion succeeded: (%s), file %s, line %d\n", #e, __FILE__, (int) __LINE__), fflush(stderr) : 0) : (fprintf(stderr, "Assertion failed: (%s), file %s, line %d\n", #e, __FILE__, (int) __LINE__), exit(1), 0))
int assertE;
#define assert_e(e, o, r) (((assertE = e) o (r)) ? ((ASSERT_TRACE) ? fprintf(stderr, "Assertion succeeded: (%s), file %s, line %d\n", #e, __FILE__, (int) __LINE__), fflush(stderr) : 0) : (fprintf(stderr, "Assertion failed: (%s %s %s), file %s, line %d, error %s\n", #e,#o,#r, __FILE__, (int) __LINE__, error_string[assertE]), exit(1), 0))
#define BEGIN_MUTEX_STALLED_ROBUST(mxAttr) for(;;) { static int _i=0; static int _robust; pthread_mutexattr_getrobust(&(mxAttr), &_robust);
#define END_MUTEX_STALLED_ROBUST(mxAttr) printf("Pass %s\n", _robust==PTHREAD_MUTEX_ROBUST?"Robust":"Non-robust"); if (++_i > 1) break; else { pthread_mutexattr_t *pma, *pmaEnd; for(pma = &(mxAttr), pmaEnd = pma + sizeof(mxAttr)/sizeof(pthread_mutexattr_t); pma < pmaEnd; pthread_mutexattr_setrobust(pma++, PTHREAD_MUTEX_ROBUST)); } }
#define IS_ROBUST (_robust==PTHREAD_MUTEX_ROBUST)
