












































































#include <stdlib.h>
#include "test.h"

enum {
NUM_CV = 5,
NUM_LOOPS = 5
};

static pthread_cond_t cv[NUM_CV];

int
main()
{
int i, j, k;
void* result = (void*)-1;
pthread_t t;

for (k = 0; k < NUM_LOOPS; k++)
{
for (i = 0; i < NUM_CV; i++)
{
assert(pthread_cond_init(&cv[i], NULL) == 0);
}

j = NUM_CV;
(void) srand((unsigned)time(NULL));


assert(pthread_create(&t, NULL, pthread_timechange_handler_np, NULL) == 0);

do
{
i = (NUM_CV - 1) * rand() / RAND_MAX;
if (cv[i] != NULL)
{
j--;
assert(pthread_cond_destroy(&cv[i]) == 0);
}
}
while (j > 0);

assert(pthread_join(t, &result) == 0);
assert ((int)(size_t)result == 0);
}

return 0;
}
