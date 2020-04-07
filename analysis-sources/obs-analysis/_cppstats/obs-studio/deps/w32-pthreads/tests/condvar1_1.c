#include <stdlib.h>
#include "test.h"
enum {
NUM_CV = 100
};
static pthread_cond_t cv[NUM_CV];
int
main()
{
int i, j;
for (i = 0; i < NUM_CV; i++)
{
assert(pthread_timechange_handler_np(NULL) == (void *) 0);
assert(pthread_cond_init(&cv[i], NULL) == 0);
}
j = NUM_CV;
(void) srand((unsigned)time(NULL));
do
{
i = (NUM_CV - 1) * rand() / RAND_MAX;
if (cv[i] != NULL)
{
j--;
assert(pthread_cond_destroy(&cv[i]) == 0);
assert(pthread_timechange_handler_np(NULL) == (void *) 0);
}
}
while (j > 0);
return 0;
}
