#include "test.h"
int
main(int argc, char * argv[])
{
struct timespec interval = {1L, 500000000L};
assert(pthread_delay_np(&interval) == 0);
return 0;
}
