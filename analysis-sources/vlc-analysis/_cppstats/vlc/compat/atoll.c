#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdlib.h>
long long atoll (const char *str)
{
return strtoll (str, NULL, 10);
}
