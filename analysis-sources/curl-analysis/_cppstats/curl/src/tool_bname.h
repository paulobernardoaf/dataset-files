#include "tool_setup.h"
#if !defined(HAVE_BASENAME)
char *tool_basename(char *path);
#define basename(x) tool_basename((x))
#endif 
