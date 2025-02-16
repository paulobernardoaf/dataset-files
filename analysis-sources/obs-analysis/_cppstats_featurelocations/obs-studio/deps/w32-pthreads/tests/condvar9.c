









































































#include "test.h"
#include <sys/timeb.h>




enum {
NUMTHREADS = 9
};

typedef struct bag_t_ bag_t;
struct bag_t_ {
int threadnum;
int started;
int finished;

};

static bag_t threadbag[NUMTHREADS + 1];

typedef struct cvthing_t_ cvthing_t;

struct cvthing_t_ {
pthread_cond_t notbusy;
pthread_mutex_t lock;
int shared;
};

static cvthing_t cvthing = {
PTHREAD_COND_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
0
};

static pthread_mutex_t start_flag = PTHREAD_MUTEX_INITIALIZER;

static struct timespec abstime = { 0, 0 };

static int awoken;

static void *
mythread(void * arg)
{
bag_t * bag = (bag_t *) arg;

assert(bag == &threadbag[bag->threadnum]);
assert(bag->started == 0);
bag->started = 1;


assert(pthread_mutex_lock(&start_flag) == 0);
assert(pthread_mutex_unlock(&start_flag) == 0);

assert(pthread_mutex_lock(&cvthing.lock) == 0);





#if defined(_MSC_VER)
#pragma inline_depth(0)
#endif
pthread_cleanup_push(pthread_mutex_unlock, (void *) &cvthing.lock);

while (! (cvthing.shared > 0))
assert(pthread_cond_timedwait(&cvthing.notbusy, &cvthing.lock, &abstime) == 0);

pthread_cleanup_pop(0);
#if defined(_MSC_VER)
#pragma inline_depth()
#endif

assert(cvthing.shared > 0);

awoken++;
bag->finished = 1;

assert(pthread_mutex_unlock(&cvthing.lock) == 0);

return (void *) 0;
}

int
main()
{
int failed = 0;
int i;
int first, last;
int canceledThreads = 0;
pthread_t t[NUMTHREADS + 1];

PTW32_STRUCT_TIMEB currSysTime;
const DWORD NANOSEC_PER_MILLISEC = 1000000;

assert((t[0] = pthread_self()).p != NULL);

assert(cvthing.notbusy == PTHREAD_COND_INITIALIZER);

assert(cvthing.lock == PTHREAD_MUTEX_INITIALIZER);

PTW32_FTIME(&currSysTime);

abstime.tv_sec = (long)currSysTime.time;
abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

abstime.tv_sec += 5;

assert((t[0] = pthread_self()).p != NULL);

awoken = 0;

for (first = 1, last = NUMTHREADS / 2;
first < NUMTHREADS;
first = last + 1, last = NUMTHREADS)
{
int ct;

assert(pthread_mutex_lock(&start_flag) == 0);

for (i = first; i <= last; i++)
{
threadbag[i].started = threadbag[i].finished = 0;
threadbag[i].threadnum = i;
assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
}




cvthing.shared = 0;

assert(pthread_mutex_unlock(&start_flag) == 0);




Sleep(1000);

ct = (first + last) / 2;
assert(pthread_cancel(t[ct]) == 0);
canceledThreads++;
assert(pthread_join(t[ct], NULL) == 0);

assert(pthread_mutex_lock(&cvthing.lock) == 0);
cvthing.shared++;
assert(pthread_mutex_unlock(&cvthing.lock) == 0);

assert(pthread_cond_broadcast(&cvthing.notbusy) == 0);




for (i = first; i <= last; i++)
{ 
failed = !threadbag[i].started;

if (failed)
{
fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
}
else
{
assert(pthread_join(t[i], NULL) == 0 || threadbag[i].finished == 0);

}
}
}





assert(pthread_mutex_destroy(&cvthing.lock) == 0);

assert(cvthing.lock == NULL);

assert_e(pthread_cond_destroy(&cvthing.notbusy), ==, 0);

assert(cvthing.notbusy == NULL);

assert(!failed);





assert(awoken == NUMTHREADS - canceledThreads);




return 0;
}
