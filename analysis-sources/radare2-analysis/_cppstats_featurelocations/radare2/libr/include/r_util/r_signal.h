#if !defined(R_SIGNAL_H)
#define R_SIGNAL_H

#if __UNIX__
#include <signal.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif


R_API int r_signal_from_string (const char *str);


R_API const char* r_signal_to_string (int code);


#if __UNIX__
R_API void r_signal_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask);
#endif

#if defined(__cplusplus)
}
#endif

#endif 
