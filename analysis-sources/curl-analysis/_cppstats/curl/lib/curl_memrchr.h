#include "curl_setup.h"
#if defined(HAVE_MEMRCHR)
#if defined(HAVE_STRING_H)
#include <string.h>
#endif
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#else 
void *Curl_memrchr(const void *s, int c, size_t n);
#define memrchr(x,y,z) Curl_memrchr((x),(y),(z))
#endif 
