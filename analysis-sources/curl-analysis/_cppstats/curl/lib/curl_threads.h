#include "curl_setup.h"
#if defined(USE_THREADS_POSIX)
#define CURL_STDCALL
#define curl_mutex_t pthread_mutex_t
#define curl_thread_t pthread_t *
#define curl_thread_t_null (pthread_t *)0
#define Curl_mutex_init(m) pthread_mutex_init(m, NULL)
#define Curl_mutex_acquire(m) pthread_mutex_lock(m)
#define Curl_mutex_release(m) pthread_mutex_unlock(m)
#define Curl_mutex_destroy(m) pthread_mutex_destroy(m)
#elif defined(USE_THREADS_WIN32)
#define CURL_STDCALL __stdcall
#define curl_mutex_t CRITICAL_SECTION
#define curl_thread_t HANDLE
#define curl_thread_t_null (HANDLE)0
#if !defined(_WIN32_WINNT) || !defined(_WIN32_WINNT_VISTA) || (_WIN32_WINNT < _WIN32_WINNT_VISTA) || (defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR))
#define Curl_mutex_init(m) InitializeCriticalSection(m)
#else
#define Curl_mutex_init(m) InitializeCriticalSectionEx(m, 0, 1)
#endif
#define Curl_mutex_acquire(m) EnterCriticalSection(m)
#define Curl_mutex_release(m) LeaveCriticalSection(m)
#define Curl_mutex_destroy(m) DeleteCriticalSection(m)
#endif
#if defined(USE_THREADS_POSIX) || defined(USE_THREADS_WIN32)
curl_thread_t Curl_thread_create(unsigned int (CURL_STDCALL *func) (void *),
void *arg);
void Curl_thread_destroy(curl_thread_t hnd);
int Curl_thread_join(curl_thread_t *hnd);
#endif 
