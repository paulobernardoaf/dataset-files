















#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

long ff_atomic_inc_long(volatile long *val);
long ff_atomic_dec_long(volatile long *val);

#if defined(__cplusplus)
}
#endif
