#include "r_util.h"
typedef struct timeval tv;
static int timeval_subtract(tv *result, tv *end, tv *begin) {
if (end->tv_usec < begin->tv_usec) {
int nsec = (begin->tv_usec - end->tv_usec) / 1000000 + 1;
begin->tv_usec -= 1000000 * nsec;
begin->tv_sec += nsec;
}
if (end->tv_usec - begin->tv_usec > 1000000) {
int nsec = (end->tv_usec - begin->tv_usec) / 1000000;
begin->tv_usec += 1000000 * nsec;
begin->tv_sec -= nsec;
}
result->tv_sec = end->tv_sec - begin->tv_sec;
result->tv_usec = end->tv_usec - begin->tv_usec;
return end->tv_sec < begin->tv_sec;
}
R_API void r_prof_start(struct r_prof_t *p) {
tv *begin = &p->begin;
p->result = 0.0;
gettimeofday(begin, NULL);
}
R_API double r_prof_end(struct r_prof_t *p) {
tv end, diff, *begin = &p->begin;
int sign;
gettimeofday (&end, NULL);
sign = timeval_subtract (&diff, begin, &end);
p->result = R_ABS (((double)(diff.tv_sec)
+ ((double)diff.tv_usec / 1000000.)));
return R_ABS (sign);
}
