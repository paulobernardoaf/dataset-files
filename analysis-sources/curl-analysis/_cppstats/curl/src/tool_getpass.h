#include "tool_setup.h"
#if !defined(HAVE_GETPASS_R)
char *getpass_r(const char *prompt, char *buffer, size_t buflen);
#endif
