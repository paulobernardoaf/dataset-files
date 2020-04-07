#pragma once
static inline long os_atomic_inc_long(volatile long *val)
{
return __sync_add_and_fetch(val, 1);
}
static inline long os_atomic_dec_long(volatile long *val)
{
return __sync_sub_and_fetch(val, 1);
}
static inline long os_atomic_set_long(volatile long *ptr, long val)
{
return __sync_lock_test_and_set(ptr, val);
}
static inline long os_atomic_load_long(const volatile long *ptr)
{
return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}
static inline bool os_atomic_compare_swap_long(volatile long *val, long old_val,
long new_val)
{
return __sync_bool_compare_and_swap(val, old_val, new_val);
}
static inline bool os_atomic_set_bool(volatile bool *ptr, bool val)
{
return __sync_lock_test_and_set(ptr, val);
}
static inline bool os_atomic_load_bool(const volatile bool *ptr)
{
return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}
