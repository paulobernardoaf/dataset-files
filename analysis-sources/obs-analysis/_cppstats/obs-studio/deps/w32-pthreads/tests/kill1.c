#include "test.h"
int
main()
{
assert(pthread_kill(pthread_self(), 1) == EINVAL);
return 0;
}
