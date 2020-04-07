#pragma once
#if defined (__cplusplus)
extern "C" {
#endif
unsigned int XXH32 (const void* input, size_t len, unsigned int seed);
void* XXH32_init (unsigned int seed);
int XXH32_feed (void* state, const void* input, int len);
unsigned int XXH32_result (void* state);
unsigned int XXH32_getIntermediateResult (void* state);
#if defined (__cplusplus)
}
#endif
