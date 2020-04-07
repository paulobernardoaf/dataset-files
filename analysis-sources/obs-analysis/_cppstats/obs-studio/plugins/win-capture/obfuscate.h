#pragma once
#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif
extern void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val);
#if defined(__cplusplus)
}
#endif
