#include "internal/stdbool.h" 
#include "ruby/ruby.h" 
#if USE_MJIT
extern bool mjit_enabled;
VALUE mjit_pause(bool wait_p);
VALUE mjit_resume(void);
void mjit_finish(bool close_handle_p);
#else
#define mjit_enabled 0
static inline VALUE mjit_pause(bool wait_p){ return Qnil; } 
static inline VALUE mjit_resume(void){ return Qnil; } 
static inline void mjit_finish(bool close_handle_p){}
#endif
