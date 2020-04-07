#include "pthread.h"
#include "implement.h"
#if defined(NEED_CALLOC)
void *
ptw32_calloc (size_t n, size_t s)
{
unsigned int m = n * s;
void *p;
p = malloc (m);
if (p == NULL)
return NULL;
memset (p, 0, m);
return p;
}
#endif
