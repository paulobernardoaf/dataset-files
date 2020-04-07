






















#include <stdio.h>
#include <string.h>

#include "libavutil/cpu.h"
#include "libavutil/thread.h"

static void *thread_main(void *arg)
{
int *flags = arg;

*flags = av_get_cpu_flags();
return NULL;
}

int main(void)
{
int cpu_flags1;
int cpu_flags2;
int ret;
pthread_t thread1;
pthread_t thread2;

if ((ret = pthread_create(&thread1, NULL, thread_main, &cpu_flags1))) {
fprintf(stderr, "pthread_create failed: %s.\n", strerror(ret));
return 1;
}
if ((ret = pthread_create(&thread2, NULL, thread_main, &cpu_flags2))) {
fprintf(stderr, "pthread_create failed: %s.\n", strerror(ret));
return 1;
}
pthread_join(thread1, NULL);
pthread_join(thread2, NULL);

if (cpu_flags1 < 0)
return 2;
if (cpu_flags2 < 0)
return 2;
if (cpu_flags1 != cpu_flags2)
return 3;

return 0;
}
