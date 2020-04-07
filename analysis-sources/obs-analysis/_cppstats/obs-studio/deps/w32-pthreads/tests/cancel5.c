#include "test.h"
enum
{
NUMTHREADS = 4
};
typedef struct bag_t_ bag_t;
struct bag_t_
{
int threadnum;
int started;
int count;
};
static bag_t threadbag[NUMTHREADS + 1];
void *
mythread (void *arg)
{
void* result = (void*)((int)(size_t)PTHREAD_CANCELED + 1);
bag_t *bag = (bag_t *) arg;
assert (bag == &threadbag[bag->threadnum]);
assert (bag->started == 0);
bag->started = 1;
assert (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL) == 0);
assert (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
for (bag->count = 0; bag->count < 100; bag->count++)
Sleep (100);
return result;
}
int
main ()
{
int failed = 0;
int i;
pthread_t t[NUMTHREADS + 1];
for (i = 1; i <= NUMTHREADS; i++)
{
threadbag[i].started = 0;
threadbag[i].threadnum = i;
assert (pthread_create (&t[i], NULL, mythread, (void *) &threadbag[i])
== 0);
}
Sleep (500);
for (i = 1; i <= NUMTHREADS; i++)
{
assert (pthread_cancel (t[i]) == 0);
}
Sleep (NUMTHREADS * 100);
for (i = 1; i <= NUMTHREADS; i++)
{
if (!threadbag[i].started)
{
failed |= !threadbag[i].started;
fprintf (stderr, "Thread %d: started %d\n", i,
threadbag[i].started);
}
}
assert (!failed);
failed = 0;
for (i = 1; i <= NUMTHREADS; i++)
{
int fail = 0;
void* result = (void*)((int)(size_t)PTHREAD_CANCELED + 1);
assert (pthread_join (t[i], &result) == 0);
fail = (result != PTHREAD_CANCELED);
if (fail)
{
fprintf (stderr, "Thread %d: started %d: count %d\n",
i, threadbag[i].started, threadbag[i].count);
}
failed = (failed || fail);
}
assert (!failed);
return 0;
}
