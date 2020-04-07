



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdlib.h>

#if !defined(__ANDROID__)
float strtof (const char *str, char **end)
{
return strtod (str, end);
}
#endif
