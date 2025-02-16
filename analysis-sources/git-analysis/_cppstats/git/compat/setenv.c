#include "../git-compat-util.h"
int gitsetenv(const char *name, const char *value, int replace)
{
int out;
size_t namelen, valuelen;
char *envstr;
if (!name || strchr(name, '=') || !value) {
errno = EINVAL;
return -1;
}
if (!replace) {
char *oldval = NULL;
oldval = getenv(name);
if (oldval) return 0;
}
namelen = strlen(name);
valuelen = strlen(value);
envstr = malloc(st_add3(namelen, valuelen, 2));
if (!envstr) {
errno = ENOMEM;
return -1;
}
memcpy(envstr, name, namelen);
envstr[namelen] = '=';
memcpy(envstr + namelen + 1, value, valuelen);
envstr[namelen + valuelen + 1] = 0;
out = putenv(envstr);
return out;
}
