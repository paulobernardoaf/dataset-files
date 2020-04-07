



















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include "vlc_charset.h"

#undef NDEBUG
#include <assert.h>

int main (void)
{
const char dot9[] = "999999.999999";
const char comma9[] = "999999,999999";
const char sharp9[] = "999999#999999";
char *end;

assert (us_atof("0") == 0.);
assert (us_atof("1") == 1.);
assert (us_atof("1.") == 1.);
assert (us_atof("1,") == 1.);
assert (us_atof("1#") == 1.);
assert (us_atof(dot9) == 999999.999999);
assert (us_atof(comma9) == 999999.);
assert (us_atof(sharp9) == 999999.);
assert (us_atof("invalid") == 0.);

assert ((us_strtod(dot9, &end ) == 999999.999999)
&& (*end == '\0'));
assert ((us_strtod(comma9, &end ) == 999999.)
&& (*end == ','));
assert ((us_strtod(sharp9, &end ) == 999999.)
&& (*end == '#'));

return 0;
}
