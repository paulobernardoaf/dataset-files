#include "pthread.h"
#include "implement.h"
int
ptw32_is_attr (const pthread_attr_t * attr)
{
return (attr == NULL ||
*attr == NULL || (*attr)->valid != PTW32_ATTR_VALID);
}
