






































#include "test.h"

static pthread_mutexattr_t mxAttr;


int _optimiseFoil;
#define FOIL(x) (_optimiseFoil = x)

int
main()
{
int mxType = -1;

assert(FOIL(PTHREAD_MUTEX_DEFAULT) == PTHREAD_MUTEX_NORMAL);
assert(FOIL(PTHREAD_MUTEX_DEFAULT) != PTHREAD_MUTEX_ERRORCHECK);
assert(FOIL(PTHREAD_MUTEX_DEFAULT) != PTHREAD_MUTEX_RECURSIVE);
assert(FOIL(PTHREAD_MUTEX_RECURSIVE) != PTHREAD_MUTEX_ERRORCHECK);

assert(FOIL(PTHREAD_MUTEX_NORMAL) == PTHREAD_MUTEX_FAST_NP);
assert(FOIL(PTHREAD_MUTEX_RECURSIVE) == PTHREAD_MUTEX_RECURSIVE_NP);
assert(FOIL(PTHREAD_MUTEX_ERRORCHECK) == PTHREAD_MUTEX_ERRORCHECK_NP);

assert(pthread_mutexattr_init(&mxAttr) == 0);
assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
assert(mxType == PTHREAD_MUTEX_NORMAL);

return 0;
}
