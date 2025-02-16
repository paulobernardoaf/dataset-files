






#include "test.h"
#include <sys/timeb.h>

#if defined(__GNUC__)
#include <stdlib.h>
#endif

#define THREADS 5
#define DATASIZE 7
#define ITERATIONS 1000000




typedef struct thread_tag {
int thread_num;
pthread_t thread_id;
int updates;
int reads;
int changed;
int seed;
} thread_t;




typedef struct data_tag {
pthread_rwlock_t lock;
int data;
int updates;
} data_t;

static thread_t threads[THREADS];
static data_t data[DATASIZE];




void *thread_routine (void *arg)
{
thread_t *self = (thread_t*)arg;
int iteration;
int element = 0;
int seed = self->seed;
int interval = 1 + rand_r (&seed) % 71;

self->changed = 0;

for (iteration = 0; iteration < ITERATIONS; iteration++)
{
if (iteration % (ITERATIONS / 10) == 0)
{
putchar('.');
fflush(stdout);
}





if ((iteration % interval) == 0)
{
assert(pthread_rwlock_wrlock (&data[element].lock) == 0);
data[element].data = self->thread_num;
data[element].updates++;
self->updates++;
interval = 1 + rand_r (&seed) % 71;
assert(pthread_rwlock_unlock (&data[element].lock) == 0);
} else {





assert(pthread_rwlock_rdlock (&data[element].lock) == 0);

self->reads++;

if (data[element].data != self->thread_num)
{
self->changed++;
interval = 1 + self->changed % 71;
}

assert(pthread_rwlock_unlock (&data[element].lock) == 0);
}

element = (element + 1) % DATASIZE;

}

return NULL;
}

int
main (int argc, char *argv[])
{
int count;
int data_count;
int thread_updates = 0;
int data_updates = 0;
int seed = 1;

PTW32_STRUCT_TIMEB currSysTime1;
PTW32_STRUCT_TIMEB currSysTime2;




for (data_count = 0; data_count < DATASIZE; data_count++)
{
data[data_count].data = 0;
data[data_count].updates = 0;

assert(pthread_rwlock_init (&data[data_count].lock, NULL) == 0);
}

PTW32_FTIME(&currSysTime1);




for (count = 0; count < THREADS; count++)
{
threads[count].thread_num = count;
threads[count].updates = 0;
threads[count].reads = 0;
threads[count].seed = 1 + rand_r (&seed) % 71;

assert(pthread_create (&threads[count].thread_id,
NULL, thread_routine, (void*)(size_t)&threads[count]) == 0);
}





for (count = 0; count < THREADS; count++)
{
assert(pthread_join (threads[count].thread_id, NULL) == 0);
}

putchar('\n');
fflush(stdout);

for (count = 0; count < THREADS; count++)
{
if (threads[count].changed > 0)
{
printf ("Thread %d found changed elements %d times\n",
count, threads[count].changed);
}
}

putchar('\n');
fflush(stdout);

for (count = 0; count < THREADS; count++)
{
thread_updates += threads[count].updates;
printf ("%02d: seed %d, updates %d, reads %d\n",
count, threads[count].seed,
threads[count].updates, threads[count].reads);
}

putchar('\n');
fflush(stdout);




for (data_count = 0; data_count < DATASIZE; data_count++)
{
data_updates += data[data_count].updates;
printf ("data %02d: value %d, %d updates\n",
data_count, data[data_count].data, data[data_count].updates);
assert(pthread_rwlock_destroy (&data[data_count].lock) == 0);
}

printf ("%d thread updates, %d data updates\n",
thread_updates, data_updates);

PTW32_FTIME(&currSysTime2);

printf( "\nstart: %ld/%d, stop: %ld/%d, duration:%ld\n",
(long)currSysTime1.time,currSysTime1.millitm,
(long)currSysTime2.time,currSysTime2.millitm,
((long)((currSysTime2.time*1000+currSysTime2.millitm) -
(currSysTime1.time*1000+currSysTime1.millitm))));

return 0;
}
