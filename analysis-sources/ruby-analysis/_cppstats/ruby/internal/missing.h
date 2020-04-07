#include "ruby/config.h" 
#if !defined(HAVE_SETPROCTITLE)
extern void ruby_init_setproctitle(int argc, char *argv[]);
#endif
