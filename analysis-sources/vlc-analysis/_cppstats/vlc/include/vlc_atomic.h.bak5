



















#if defined(__cplusplus)
#error Not implemented in C++.
#endif

#if !defined(VLC_ATOMIC_H)
#define VLC_ATOMIC_H






#include <assert.h>
#include <stdatomic.h>
#include <vlc_common.h>

typedef struct vlc_atomic_rc_t {
atomic_uintptr_t refs;
} vlc_atomic_rc_t;


static inline void vlc_atomic_rc_init(vlc_atomic_rc_t *rc)
{
atomic_init(&rc->refs, 1);
}


static inline void vlc_atomic_rc_inc(vlc_atomic_rc_t *rc)
{
uintptr_t prev = atomic_fetch_add_explicit(&rc->refs, 1, memory_order_relaxed);
vlc_assert(prev);
VLC_UNUSED(prev);
}


static inline bool vlc_atomic_rc_dec(vlc_atomic_rc_t *rc)
{
uintptr_t prev = atomic_fetch_sub_explicit(&rc->refs, 1, memory_order_acq_rel);
vlc_assert(prev);
return prev == 1;
}

#endif
