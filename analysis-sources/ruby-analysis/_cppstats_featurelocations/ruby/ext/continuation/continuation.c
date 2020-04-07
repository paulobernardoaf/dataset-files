
#include "ruby/ruby.h"

void ruby_Init_Continuation_body(void);

void
Init_continuation(void)
{
#if !defined(RUBY_EXPORT)
rb_warn("callcc is obsolete; use Fiber instead");
#endif
ruby_Init_Continuation_body();
}
