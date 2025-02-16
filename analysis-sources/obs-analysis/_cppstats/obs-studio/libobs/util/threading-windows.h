#pragma once
#include <intrin.h>
static inline long os_atomic_inc_long(volatile long *val)
{
return _InterlockedIncrement(val);
}
static inline long os_atomic_dec_long(volatile long *val)
{
return _InterlockedDecrement(val);
}
static inline long os_atomic_set_long(volatile long *ptr, long val)
{
return (long)_InterlockedExchange((volatile long *)ptr, (long)val);
}
static inline long os_atomic_load_long(const volatile long *ptr)
{
return (long)_InterlockedOr((volatile long *)ptr, 0);
}
static inline bool os_atomic_compare_swap_long(volatile long *val, long old_val,
long new_val)
{
return _InterlockedCompareExchange(val, new_val, old_val) == old_val;
}
static inline bool os_atomic_set_bool(volatile bool *ptr, bool val)
{
return !!_InterlockedExchange8((volatile char *)ptr, (char)val);
}
static inline bool os_atomic_load_bool(const volatile bool *ptr)
{
return !!_InterlockedOr8((volatile char *)ptr, 0);
}
