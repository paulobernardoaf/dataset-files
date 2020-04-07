#include "pthread.h"
#include "implement.h"
int ptw32_processInitialized = PTW32_FALSE;
ptw32_thread_t * ptw32_threadReuseTop = PTW32_THREAD_REUSE_EMPTY;
ptw32_thread_t * ptw32_threadReuseBottom = PTW32_THREAD_REUSE_EMPTY;
pthread_key_t ptw32_selfThreadKey = NULL;
pthread_key_t ptw32_cleanupKey = NULL;
pthread_cond_t ptw32_cond_list_head = NULL;
pthread_cond_t ptw32_cond_list_tail = NULL;
int ptw32_concurrency = 0;
int ptw32_features = 0;
unsigned __int64 ptw32_threadSeqNumber = 0;
DWORD (*ptw32_register_cancelation) (PAPCFUNC, HANDLE, DWORD) = NULL;
ptw32_mcs_lock_t ptw32_thread_reuse_lock = 0;
ptw32_mcs_lock_t ptw32_mutex_test_init_lock = 0;
ptw32_mcs_lock_t ptw32_cond_test_init_lock = 0;
ptw32_mcs_lock_t ptw32_rwlock_test_init_lock = 0;
ptw32_mcs_lock_t ptw32_spinlock_test_init_lock = 0;
ptw32_mcs_lock_t ptw32_cond_list_lock = 0;
#if defined(_UWIN)
int pthread_count = 0;
#endif
