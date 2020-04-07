#include <pthread.h>
#include <stdint.h>
#include "stdatomic.h"
static pthread_mutex_t atomic_lock = PTHREAD_MUTEX_INITIALIZER;
void avpriv_atomic_lock(void)
{
pthread_mutex_lock(&atomic_lock);
}
void avpriv_atomic_unlock(void)
{
pthread_mutex_unlock(&atomic_lock);
}
