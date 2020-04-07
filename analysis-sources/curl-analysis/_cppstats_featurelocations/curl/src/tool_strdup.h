#if !defined(HEADER_TOOL_STRDUP_H)
#define HEADER_TOOL_STRDUP_H





















#include "tool_setup.h"

#if !defined(HAVE_STRDUP)
extern char *strdup(const char *str);
#endif

#endif 
