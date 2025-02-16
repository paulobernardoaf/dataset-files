#include <stdint.h>
typedef uint64_t TSDuration;
#if defined(_WIN32)
#include <windows.h>
typedef uint64_t TSClock;
static inline TSDuration duration_from_micros(uint64_t micros) {
LARGE_INTEGER frequency;
QueryPerformanceFrequency(&frequency);
return micros * (uint64_t)frequency.QuadPart / 1000000;
}
static inline uint64_t duration_to_micros(TSDuration self) {
LARGE_INTEGER frequency;
QueryPerformanceFrequency(&frequency);
return self * 1000000 / (uint64_t)frequency.QuadPart;
}
static inline TSClock clock_null(void) {
return 0;
}
static inline TSClock clock_now(void) {
LARGE_INTEGER result;
QueryPerformanceCounter(&result);
return (uint64_t)result.QuadPart;
}
static inline TSClock clock_after(TSClock base, TSDuration duration) {
return base + duration;
}
static inline bool clock_is_null(TSClock self) {
return !self;
}
static inline bool clock_is_gt(TSClock self, TSClock other) {
return self > other;
}
#elif defined(CLOCK_MONOTONIC) && !defined(__APPLE__)
#include <time.h>
typedef struct timespec TSClock;
static inline TSDuration duration_from_micros(uint64_t micros) {
return micros;
}
static inline uint64_t duration_to_micros(TSDuration self) {
return self;
}
static inline TSClock clock_now(void) {
TSClock result;
clock_gettime(CLOCK_MONOTONIC, &result);
return result;
}
static inline TSClock clock_null(void) {
return (TSClock) {0, 0};
}
static inline TSClock clock_after(TSClock base, TSDuration duration) {
TSClock result = base;
result.tv_sec += duration / 1000000;
result.tv_nsec += (duration % 1000000) * 1000;
return result;
}
static inline bool clock_is_null(TSClock self) {
return !self.tv_sec;
}
static inline bool clock_is_gt(TSClock self, TSClock other) {
if (self.tv_sec > other.tv_sec) return true;
if (self.tv_sec < other.tv_sec) return false;
return self.tv_nsec > other.tv_nsec;
}
#else
#include <time.h>
typedef uint64_t TSClock;
static inline TSDuration duration_from_micros(uint64_t micros) {
return micros * (uint64_t)CLOCKS_PER_SEC / 1000000;
}
static inline uint64_t duration_to_micros(TSDuration self) {
return self * 1000000 / (uint64_t)CLOCKS_PER_SEC;
}
static inline TSClock clock_null(void) {
return 0;
}
static inline TSClock clock_now(void) {
return (uint64_t)clock();
}
static inline TSClock clock_after(TSClock base, TSDuration duration) {
return base + duration;
}
static inline bool clock_is_null(TSClock self) {
return !self;
}
static inline bool clock_is_gt(TSClock self, TSClock other) {
return self > other;
}
#endif
