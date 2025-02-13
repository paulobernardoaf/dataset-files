#include "test.h"
static int wasHere = 0;
static pthread_spinlock_t spin;
void * unlocker(void * arg)
{
int expectedResult = (int)(size_t)arg;
wasHere++;
assert(pthread_spin_unlock(&spin) == expectedResult);
wasHere++;
return NULL;
}
int
main()
{
pthread_t t;
wasHere = 0;
assert(pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE) == 0);
assert(pthread_spin_lock(&spin) == 0);
assert(pthread_create(&t, NULL, unlocker, (void*)0) == 0);
assert(pthread_join(t, NULL) == 0);
assert(pthread_spin_unlock(&spin) == 0);
assert(pthread_spin_destroy(&spin) == 0);
assert(wasHere == 2);
return 0;
}
