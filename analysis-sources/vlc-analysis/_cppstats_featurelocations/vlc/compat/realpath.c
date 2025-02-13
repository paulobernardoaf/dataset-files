



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>
#if defined(_WIN32)
#include <windows.h>
#endif

char *realpath(const char * restrict relpath, char * restrict resolved_path)
{
if (relpath == NULL)
{
errno = EINVAL;
return NULL;
}

#if defined(_WIN32)
size_t len = MultiByteToWideChar( CP_UTF8, 0, relpath, -1, NULL, 0 );
if (len == 0)
return NULL;

wchar_t *wrelpath = malloc(len * sizeof (wchar_t));
if (wrelpath == NULL)
return NULL;

MultiByteToWideChar( CP_UTF8, 0, relpath, -1, wrelpath, len );

wchar_t *wfullpath = _wfullpath( NULL, wrelpath, _MAX_PATH );
free(wrelpath);
if (wfullpath != NULL)
{
len = WideCharToMultiByte( CP_UTF8, 0, wfullpath, -1, NULL, 0, NULL, NULL );
if (len != 0)
{
if (resolved_path != NULL)
len = len >= _MAX_PATH ? _MAX_PATH : len;
else
resolved_path = (char *)malloc(len);

if (resolved_path != NULL)
WideCharToMultiByte( CP_UTF8, 0, wfullpath, -1, resolved_path, len, NULL, NULL );
free(wfullpath);
return resolved_path;
}
free(wfullpath);
}
#else
(void)resolved_path;
#endif
errno = EACCES;
return NULL;
}
