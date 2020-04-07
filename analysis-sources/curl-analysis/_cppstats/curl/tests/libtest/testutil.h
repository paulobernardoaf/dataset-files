#include "curl_setup.h"
struct timeval tutil_tvnow(void);
long tutil_tvdiff(struct timeval t1, struct timeval t2);
double tutil_tvdiff_secs(struct timeval t1, struct timeval t2);
