#include "pthread.h"
#include "implement.h"
#if defined(NEED_FTIME)
#define PTW32_TIMESPEC_TO_FILETIME_OFFSET ( ((int64_t) 27111902 << 32) + (int64_t) 3577643008 )
INLINE void
ptw32_timespec_to_filetime (const struct timespec *ts, FILETIME * ft)
{
*(int64_t *) ft = ts->tv_sec * 10000000
+ (ts->tv_nsec + 50) / 100 + PTW32_TIMESPEC_TO_FILETIME_OFFSET;
}
INLINE void
ptw32_filetime_to_timespec (const FILETIME * ft, struct timespec *ts)
{
ts->tv_sec =
(int) ((*(int64_t *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
ts->tv_nsec =
(int) ((*(int64_t *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET -
((int64_t) ts->tv_sec * (int64_t) 10000000)) * 100);
}
#endif 
