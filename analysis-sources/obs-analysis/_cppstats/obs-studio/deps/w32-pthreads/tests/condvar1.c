#include "test.h"
static pthread_cond_t cv = NULL;
int
main()
{
assert(cv == NULL);
assert(pthread_cond_init(&cv, NULL) == 0);
assert(cv != NULL);
assert(pthread_cond_destroy(&cv) == 0);
assert(cv == NULL);
return 0;
}
