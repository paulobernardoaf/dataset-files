


















#undef RUBY_EXPORT
#include "ruby.h"
#include "vm_debug.h"
#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif
#if RUBY_DEVEL && !defined RUBY_DEBUG_ENV
#define RUBY_DEBUG_ENV 1
#endif
#if defined RUBY_DEBUG_ENV && !RUBY_DEBUG_ENV
#undef RUBY_DEBUG_ENV
#endif
#if defined(RUBY_DEBUG_ENV)
#include <stdlib.h>
#endif

int
main(int argc, char **argv)
{
#if defined(RUBY_DEBUG_ENV)
ruby_set_debug_option(getenv("RUBY_DEBUG"));
#endif
#if defined(HAVE_LOCALE_H)
setlocale(LC_CTYPE, "");
#endif

ruby_sysinit(&argc, &argv);
{
RUBY_INIT_STACK;
ruby_init();
return ruby_run_node(ruby_options(argc, argv));
}
}
