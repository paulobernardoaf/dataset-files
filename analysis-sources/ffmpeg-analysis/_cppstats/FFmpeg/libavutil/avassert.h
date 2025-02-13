#include <stdlib.h>
#include "avutil.h"
#include "log.h"
#define av_assert0(cond) do { if (!(cond)) { av_log(NULL, AV_LOG_PANIC, "Assertion %s failed at %s:%d\n", AV_STRINGIFY(cond), __FILE__, __LINE__); abort(); } } while (0)
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 0
#define av_assert1(cond) av_assert0(cond)
#else
#define av_assert1(cond) ((void)0)
#endif
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
#define av_assert2(cond) av_assert0(cond)
#define av_assert2_fpu() av_assert0_fpu()
#else
#define av_assert2(cond) ((void)0)
#define av_assert2_fpu() ((void)0)
#endif
void av_assert0_fpu(void);
