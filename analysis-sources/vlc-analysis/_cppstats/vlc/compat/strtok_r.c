#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <string.h>
char *strtok_r(char *s, const char *delim, char **save_ptr)
{
char *token;
if (s == NULL)
s = *save_ptr;
s += strspn (s, delim);
if (*s == '\0')
return NULL;
token = s;
s = strpbrk (token, delim);
if (s == NULL)
*save_ptr = strchr (token, '\0');
else
{
*s = '\0';
*save_ptr = s + 1;
}
return token;
}
