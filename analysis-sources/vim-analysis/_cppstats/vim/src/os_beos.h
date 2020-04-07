#undef USE_SYSTEM
#define USE_THREAD_FOR_INPUT_WITH_TIMEOUT 1
#define USE_TERM_CONSOLE
#define HAVE_DROP_FILE
#undef BEOS_DR8
#define BEOS_PR_OR_BETTER
#if !defined(PROTO)
#include <net/socket.h> 
#endif
