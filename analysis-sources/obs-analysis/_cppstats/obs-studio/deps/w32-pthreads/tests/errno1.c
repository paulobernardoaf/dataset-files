#include "test.h"
enum {
NUMTHREADS = 3
};
typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
};
static bag_t threadbag[NUMTHREADS + 1];
pthread_mutex_t stop_here = PTHREAD_MUTEX_INITIALIZER;
void *
mythread(void * arg)
{
bag_t * bag = (bag_t *) arg;
assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;
errno = bag->threadnum;
Sleep(1000);
pthread_mutex_lock(&stop_here);
assert(errno == bag->threadnum);
pthread_mutex_unlock(&stop_here);
Sleep(1000);
return 0;
}
int
main()
{
int failed = 0;
int i;
pthread_t t[NUMTHREADS + 1];
pthread_mutex_lock(&stop_here);
errno = 0;
assert((t[0] = pthread_self()).p != NULL);
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
}
Sleep(2000);
pthread_mutex_unlock(&stop_here);
Sleep(NUMTHREADS * 1000);
for (i = 1; i <= NUMTHREADS; i++)
{ 
failed = !threadbag[i].started;
if (failed)
{
fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
}
}
assert(!failed);
for (i = 1; i <= NUMTHREADS; i++)
{ 
}
assert(!failed);
return 0;
}
