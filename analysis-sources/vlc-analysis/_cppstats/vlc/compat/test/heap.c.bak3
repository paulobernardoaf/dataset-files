



















#include "config.h"
#undef NDEBUG
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

struct big_align_struct {
long long ll;
double d;
};


static const size_t alignv[] = {
alignof (char),
alignof (short),
alignof (int),
alignof (long),
alignof (long long),
alignof (float),
alignof (double),
alignof (struct big_align_struct),
alignof (void *),
alignof (max_align_t),
};

static const size_t alignc = sizeof (alignv) / sizeof (alignv[0]);

static void test_posix_memalign(size_t align, size_t size)
{
void *p;
int val = posix_memalign(&p, align, size);

if (align >= sizeof (void *)) {
if (val == 0) {
assert(((uintptr_t)p & (align - 1)) == 0);
free(p);
}
} else
assert(val != 0);
}

int main(void)
{
void *p;



for (size_t i = 0; i < alignc; i++) {
size_t align = alignv[i];

assert((align & (align - 1)) == 0); 

p = aligned_alloc(alignv[i], 0);
free(p); 

for (size_t j = 0; j < alignc; j++) {
size_t size = alignv[j];

if (size < align)
continue; 

p = aligned_alloc(align, size);
assert(p != NULL); 
assert(((uintptr_t)p & (align - 1)) == 0);
free(p);
}
}



for (size_t i = 0; i < 21; i++) {
size_t align = (size_t)1 << i;

test_posix_memalign(align, 0);
test_posix_memalign(align, 1);
test_posix_memalign(align, align - 1);
test_posix_memalign(align, align);
test_posix_memalign(align, align * 2);
}

return 0;
}
