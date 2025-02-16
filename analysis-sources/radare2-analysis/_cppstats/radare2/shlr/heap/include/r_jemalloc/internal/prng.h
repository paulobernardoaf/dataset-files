#if defined(JEMALLOC_H_TYPES)
#define PRNG_A_32 UINT32_C(1103515241)
#define PRNG_C_32 UINT32_C(12347)
#define PRNG_A_64 UINT64_C(6364136223846793005)
#define PRNG_C_64 UINT64_C(1442695040888963407)
#endif 
#if defined(JEMALLOC_H_STRUCTS)
#endif 
#if defined(JEMALLOC_H_EXTERNS)
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
uint32_t prng_state_next_u32(uint32_t state);
uint64_t prng_state_next_u64(uint64_t state);
size_t prng_state_next_zu(size_t state);
uint32_t prng_lg_range_u32(uint32_t *state, unsigned lg_range,
bool atomic);
uint64_t prng_lg_range_u64(uint64_t *state, unsigned lg_range);
size_t prng_lg_range_zu(size_t *state, unsigned lg_range, bool atomic);
uint32_t prng_range_u32(uint32_t *state, uint32_t range, bool atomic);
uint64_t prng_range_u64(uint64_t *state, uint64_t range);
size_t prng_range_zu(size_t *state, size_t range, bool atomic);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_PRNG_C_))
JEMALLOC_ALWAYS_INLINE uint32_t
prng_state_next_u32(uint32_t state)
{
return ((state * PRNG_A_32) + PRNG_C_32);
}
JEMALLOC_ALWAYS_INLINE uint64_t
prng_state_next_u64(uint64_t state)
{
return ((state * PRNG_A_64) + PRNG_C_64);
}
JEMALLOC_ALWAYS_INLINE size_t
prng_state_next_zu(size_t state)
{
#if LG_SIZEOF_PTR == 2
return ((state * PRNG_A_32) + PRNG_C_32);
#elif LG_SIZEOF_PTR == 3
return ((state * PRNG_A_64) + PRNG_C_64);
#else
#error Unsupported pointer size
#endif
}
JEMALLOC_ALWAYS_INLINE uint32_t
prng_lg_range_u32(uint32_t *state, unsigned lg_range, bool atomic)
{
uint32_t ret, state1;
if (lg_range <1 || lg_range >32) {
return 0;
}
#if 0
assert(lg_range > 0);
assert(lg_range <= 32);
#endif
if (atomic) {
uint32_t state0;
do {
state0 = atomic_read_uint32(state);
state1 = prng_state_next_u32(state0);
} while (atomic_cas_uint32(state, state0, state1));
} else {
state1 = prng_state_next_u32(*state);
*state = state1;
}
ret = state1 >> (32 - lg_range);
return (ret);
}
JEMALLOC_ALWAYS_INLINE uint64_t
prng_lg_range_u64(uint64_t *state, unsigned lg_range)
{
uint64_t ret, state1;
if (lg_range <1 || lg_range > 64) {
return 0;
}
#if 0
assert(lg_range > 0);
assert(lg_range <= 64);
#endif
state1 = prng_state_next_u64(*state);
*state = state1;
ret = state1 >> (64 - lg_range);
return (ret);
}
JEMALLOC_ALWAYS_INLINE size_t
prng_lg_range_zu(size_t *state, unsigned lg_range, bool atomic)
{
size_t ret, state1;
#if 0
assert(lg_range > 0);
assert(lg_range <= ZU(1) << (3 + LG_SIZEOF_PTR));
#endif
if (atomic) {
size_t state0;
do {
state0 = atomic_read_z(state);
state1 = prng_state_next_zu(state0);
} while (atomic_cas_z(state, state0, state1));
} else {
state1 = prng_state_next_zu(*state);
*state = state1;
}
ret = state1 >> ((ZU(1) << (3 + LG_SIZEOF_PTR)) - lg_range);
return (ret);
}
JEMALLOC_ALWAYS_INLINE uint32_t
prng_range_u32(uint32_t *state, uint32_t range, bool atomic)
{
uint32_t ret;
unsigned lg_range;
#if 0
assert(range > 1);
#endif
if (range <2) {
return 0;
}
lg_range = ffs_u32(pow2_ceil_u32(range)) - 1;
do {
ret = prng_lg_range_u32(state, lg_range, atomic);
} while (ret >= range);
return (ret);
}
JEMALLOC_ALWAYS_INLINE uint64_t
prng_range_u64(uint64_t *state, uint64_t range)
{
uint64_t ret;
unsigned lg_range;
lg_range = ffs_u64(pow2_ceil_u64(range)) - 1;
do {
ret = prng_lg_range_u64(state, lg_range);
} while (ret >= range);
return (ret);
}
JEMALLOC_ALWAYS_INLINE size_t
prng_range_zu(size_t *state, size_t range, bool atomic)
{
size_t ret;
unsigned lg_range;
assert(range > 1);
lg_range = ffs_u64(pow2_ceil_u64(range)) - 1;
do {
ret = prng_lg_range_zu(state, lg_range, atomic);
} while (ret >= range);
return (ret);
}
#endif
#endif 
