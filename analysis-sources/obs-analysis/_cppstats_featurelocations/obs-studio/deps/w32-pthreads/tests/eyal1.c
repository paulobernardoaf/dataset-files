












































































#include "test.h"

#include <stdlib.h>
#include <math.h>

struct thread_control {
int id;
pthread_t thread; 
pthread_mutex_t mutex_start;
pthread_mutex_t mutex_started;
pthread_mutex_t mutex_end;
pthread_mutex_t mutex_ended;
long work; 
int stat; 
};

typedef struct thread_control TC;

static TC *tcs = NULL;
static int nthreads = 10;
static int nwork = 100;
static int quiet = 0;

static int todo = -1;

static pthread_mutex_t mutex_todo = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_stdout = PTHREAD_MUTEX_INITIALIZER;


static void
die (int ret)
{
if (NULL != tcs)
{
free (tcs);
tcs = NULL;
}

if (ret)
exit (ret);
}


static double
waste_time (int n)
{
int i;
double f, g, h, s;

s = 0.0;




for (i = n*100; i > 0; --i)
{
f = rand ();
g = rand ();
h = rand ();
s += 2.0 * f * g / (h != 0.0 ? (h * h) : 1.0);
}
return s;
}

static int
do_work_unit (int who, int n)
{
int i;
static int nchars = 0;
double f = 0.0;

if (quiet)
i = 0;
else {



assert(pthread_mutex_lock (&mutex_stdout) == 0);




i = printf ("%c", "0123456789abcdefghijklmnopqrstuvwxyz"[who]);

if (!(++nchars % 50))
printf ("\n");

fflush (stdout);




assert(pthread_mutex_unlock (&mutex_stdout) == 0);
}

n = rand () % 10000; 
f = waste_time (n);


if (f > 0.0)
return(n);

return (n);
}

static int
print_server (void *ptr)
{
int mywork;
int n;
TC *tc = (TC *)ptr;

assert(pthread_mutex_lock (&tc->mutex_started) == 0);

for (;;)
{
assert(pthread_mutex_lock (&tc->mutex_start) == 0);
assert(pthread_mutex_unlock (&tc->mutex_start) == 0);
assert(pthread_mutex_lock (&tc->mutex_ended) == 0);
assert(pthread_mutex_unlock (&tc->mutex_started) == 0);

for (;;)
{




assert(pthread_mutex_lock (&mutex_todo) == 0);

mywork = todo;
if (todo >= 0)
{
++todo;
if (todo >= nwork)
todo = -1;
}
assert(pthread_mutex_unlock (&mutex_todo) == 0);

if (mywork < 0)
break;

assert((n = do_work_unit (tc->id, mywork)) >= 0);
tc->work += n;
}

assert(pthread_mutex_lock (&tc->mutex_end) == 0);
assert(pthread_mutex_unlock (&tc->mutex_end) == 0);
assert(pthread_mutex_lock (&tc->mutex_started) == 0);
assert(pthread_mutex_unlock (&tc->mutex_ended) == 0);

if (-2 == mywork)
break;
}

assert(pthread_mutex_unlock (&tc->mutex_started) == 0);

return (0);
}

static void
dosync (void)
{
int i;

for (i = 0; i < nthreads; ++i)
{
assert(pthread_mutex_lock (&tcs[i].mutex_end) == 0);
assert(pthread_mutex_unlock (&tcs[i].mutex_start) == 0);
assert(pthread_mutex_lock (&tcs[i].mutex_started) == 0);
assert(pthread_mutex_unlock (&tcs[i].mutex_started) == 0);
}




for (i = 0; i < nthreads; ++i)
{
assert(pthread_mutex_lock (&tcs[i].mutex_start) == 0);
assert(pthread_mutex_unlock (&tcs[i].mutex_end) == 0);
assert(pthread_mutex_lock (&tcs[i].mutex_ended) == 0);
assert(pthread_mutex_unlock (&tcs[i].mutex_ended) == 0);
}
}

static void
dowork (void)
{
todo = 0;
dosync();

todo = 0;
dosync();
}

int
main (int argc, char *argv[])
{
int i;

assert(NULL != (tcs = (TC *) calloc (nthreads, sizeof (*tcs))));




for (i = 0; i < nthreads; ++i)
{
tcs[i].id = i;

assert(pthread_mutex_init (&tcs[i].mutex_start, NULL) == 0);
assert(pthread_mutex_init (&tcs[i].mutex_started, NULL) == 0);
assert(pthread_mutex_init (&tcs[i].mutex_end, NULL) == 0);
assert(pthread_mutex_init (&tcs[i].mutex_ended, NULL) == 0);

tcs[i].work = 0; 

assert(pthread_mutex_lock (&tcs[i].mutex_start) == 0);
assert((tcs[i].stat = 
pthread_create (&tcs[i].thread,
NULL,
(void *(*)(void *))print_server,
(void *) &tcs[i])
) == 0);




{
int trylock = 0;

while (trylock == 0)
{
trylock = pthread_mutex_trylock(&tcs[i].mutex_started);
assert(trylock == 0 || trylock == EBUSY);

if (trylock == 0)
{
assert(pthread_mutex_unlock (&tcs[i].mutex_started) == 0);
}
}
}
}

dowork ();




todo = -2; 
dosync();

for (i = 0; i < nthreads; ++i)
{
if (0 == tcs[i].stat)
assert(pthread_join (tcs[i].thread, NULL) == 0);
}




assert(pthread_mutex_destroy (&mutex_stdout) == 0);
assert(pthread_mutex_destroy (&mutex_todo) == 0);




printf ("\n");




for (i = 0; i < nthreads; ++i)
{
printf ("%2d ", i);
if (0 == tcs[i].stat)
printf ("%10ld\n", tcs[i].work);
else
printf ("failed %d\n", tcs[i].stat);

assert(pthread_mutex_unlock(&tcs[i].mutex_start) == 0);

assert(pthread_mutex_destroy (&tcs[i].mutex_start) == 0);
assert(pthread_mutex_destroy (&tcs[i].mutex_started) == 0);
assert(pthread_mutex_destroy (&tcs[i].mutex_end) == 0);
assert(pthread_mutex_destroy (&tcs[i].mutex_ended) == 0);
}

die (0);

return (0);
}
