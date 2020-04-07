#include "ruby/assert.h"
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
#include <assert.h>
#else
#undef assert
#define assert RUBY_ASSERT
#endif
#if defined(NDEBUG)
#undef RUBY_NDEBUG
#define RUBY_NDEBUG 1
#endif
