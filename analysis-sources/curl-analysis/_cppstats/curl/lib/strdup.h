#include "curl_setup.h"
#if !defined(HAVE_STRDUP)
extern char *curlx_strdup(const char *str);
#endif
void *Curl_memdup(const void *src, size_t buffer_length);
void *Curl_saferealloc(void *ptr, size_t size);
