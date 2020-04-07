















#include "ff-threading.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

long ff_atomic_inc_long(volatile long *val)
{
return InterlockedIncrement(val);
}

long ff_atomic_dec_long(volatile long *val)
{
return InterlockedDecrement(val);
}
