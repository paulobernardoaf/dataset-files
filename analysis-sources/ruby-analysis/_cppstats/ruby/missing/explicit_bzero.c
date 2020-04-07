#if !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#include "ruby/missing.h"
#include <string.h>
#if defined(HAVE_MEMSET_S)
#include <string.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif
#if !defined(FUNC_UNOPTIMIZED)
#define FUNC_UNOPTIMIZED(x) x
#endif
#undef explicit_bzero
#if !defined(HAVE_EXPLICIT_BZERO)
#if defined(HAVE_EXPLICIT_MEMSET)
void
explicit_bzero(void *b, size_t len)
{
(void)explicit_memset(b, 0, len);
}
#elif defined HAVE_MEMSET_S
void
explicit_bzero(void *b, size_t len)
{
memset_s(b, len, 0, len);
}
#elif defined SecureZeroMemory
void
explicit_bzero(void *b, size_t len)
{
SecureZeroMemory(b, len);
}
#elif defined HAVE_FUNC_WEAK
WEAK(void ruby_explicit_bzero_hook_unused(void *buf, size_t len));
void
ruby_explicit_bzero_hook_unused(void *buf, size_t len)
{
}
void
explicit_bzero(void *b, size_t len)
{
memset(b, 0, len);
ruby_explicit_bzero_hook_unused(b, len);
}
#else 
FUNC_UNOPTIMIZED(void explicit_bzero(void *b, size_t len));
#undef explicit_bzero
void
explicit_bzero(void *b, size_t len)
{
volatile char* p = (volatile char*)b;
while(len) {
*p = 0;
p++;
len--;
}
}
#endif
#endif 
