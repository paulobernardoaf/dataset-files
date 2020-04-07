



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#if !defined (HAVE_POSIX_MEMALIGN)
#include <malloc.h>
#endif

void *aligned_alloc(size_t align, size_t size)
{


if ((align & (align - 1)) || (size & (align - 1)))
{
errno = EINVAL;
return NULL;
}

#if defined(HAVE_POSIX_MEMALIGN)
if (align < sizeof (void *)) 
align = sizeof (void *);

void *ptr;
int err = posix_memalign(&ptr, align, size);
if (err)
{
errno = err;
ptr = NULL;
}
return ptr;

#elif defined(HAVE_MEMALIGN)
return memalign(align, size);
#elif defined (_WIN32) && defined(__MINGW32__)
return __mingw_aligned_malloc(size, align);
#elif defined (_WIN32) && defined(_MSC_VER)
return _aligned_malloc(size, align);
#else

assert(align <= alignof (max_align_t));
return malloc(((void) align, size));
#endif
}
