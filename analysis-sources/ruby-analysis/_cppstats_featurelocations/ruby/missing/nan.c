#include "ruby/missing.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

double
nan(const char *spec)
{
#if 0



if (spec && spec[0]) {
double generated_nan;
int len = snprintf(NULL, 0, "NAN(%s)", spec);
char *buf = malloc(len + 1); 
sprintf(buf, "NAN(%s)", spec);
generated_nan = strtod(buf, NULL);
free(buf);
return generated_nan;
}
else
#endif
{
assert(!spec || !spec[0]);
return (double)NAN;
}
}
