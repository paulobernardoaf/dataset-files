#include <stdint.h>
static inline uint32_t bitmask_for_index(uint16_t id) {
return (1u << (31 - id));
}
#if defined _WIN32 && !defined __GNUC__
#include <intrin.h>
static inline uint32_t count_leading_zeros(uint32_t x) {
if (x == 0) return 32;
uint32_t result;
_BitScanReverse(&result, x);
return 31 - result;
}
#else
static inline uint32_t count_leading_zeros(uint32_t x) {
if (x == 0) return 32;
return __builtin_clz(x);
}
#endif
