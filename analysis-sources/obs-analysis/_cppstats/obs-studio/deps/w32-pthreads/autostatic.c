#if defined(PTW32_STATIC_LIB)
#if defined(__MINGW64__) || defined(__MINGW32__) || defined(_MSC_VER)
#include "pthread.h"
#include "implement.h"
static void on_process_init(void)
{
pthread_win32_process_attach_np ();
}
static void on_process_exit(void)
{
pthread_win32_thread_detach_np ();
pthread_win32_process_detach_np ();
}
#if defined(__MINGW64__) || defined(__MINGW32__)
#define attribute_section(a) __attribute__((section(a)))
#elif defined(_MSC_VER)
#define attribute_section(a) __pragma(section(a,long,read)); __declspec(allocate(a))
#endif
attribute_section(".ctors") void *gcc_ctor = on_process_init;
attribute_section(".dtors") void *gcc_dtor = on_process_exit;
attribute_section(".CRT$XCU") void *msc_ctor = on_process_init;
attribute_section(".CRT$XPU") void *msc_dtor = on_process_exit;
#endif 
#endif 
