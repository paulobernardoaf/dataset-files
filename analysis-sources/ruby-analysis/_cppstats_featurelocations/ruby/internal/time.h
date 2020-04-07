#if !defined(INTERNAL_TIME_H)
#define INTERNAL_TIME_H









#include "ruby/config.h" 
#include "internal/bits.h" 
#include "ruby/ruby.h" 

#if SIGNEDNESS_OF_TIME_T < 0 
#define TIMET_MAX SIGNED_INTEGER_MAX(time_t)
#define TIMET_MIN SIGNED_INTEGER_MIN(time_t)
#elif SIGNEDNESS_OF_TIME_T > 0 
#define TIMET_MAX UNSIGNED_INTEGER_MAX(time_t)
#define TIMET_MIN ((time_t)0)
#endif

struct timeval; 


struct timeval rb_time_timeval(VALUE);

RUBY_SYMBOL_EXPORT_BEGIN

void ruby_reset_leap_second_info(void);
void ruby_reset_timezone(void);
RUBY_SYMBOL_EXPORT_END

#endif 
