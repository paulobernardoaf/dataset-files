#include "avassert.h"
#include "mem.h"
static inline int ff_fast_malloc(void *ptr, unsigned int *size, size_t min_size, int zero_realloc)
{
void *val;
memcpy(&val, ptr, sizeof(val));
if (min_size <= *size) {
av_assert0(val || !min_size);
return 0;
}
min_size = FFMAX(min_size + min_size / 16 + 32, min_size);
av_freep(ptr);
val = zero_realloc ? av_mallocz(min_size) : av_malloc(min_size);
memcpy(ptr, &val, sizeof(val));
if (!val)
min_size = 0;
*size = min_size;
return 1;
}
