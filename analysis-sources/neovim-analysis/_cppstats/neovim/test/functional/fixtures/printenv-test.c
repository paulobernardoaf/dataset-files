#include <stdio.h>
#if defined(WIN32)
#include <windows.h>
#else
#include <stdlib.h>
#endif
#if defined(WIN32)
int wmain(int argc, wchar_t **argv)
#else
int main(int argc, char **argv)
#endif
{
if (argc != 2) {
return 1;
}
#if defined(WIN32)
wchar_t *value = _wgetenv(argv[1]);
if (value == NULL) {
return 1;
}
int utf8_len = WideCharToMultiByte(CP_UTF8,
0,
value,
-1,
NULL,
0,
NULL,
NULL);
if (utf8_len == 0) {
return (int)GetLastError();
}
char *utf8_value = (char *)calloc((size_t)utf8_len, sizeof(char));
utf8_len = WideCharToMultiByte(CP_UTF8,
0,
value,
-1,
utf8_value,
utf8_len,
NULL,
NULL);
fprintf(stdout, "%s", utf8_value);
free(utf8_value);
#else
char *value = getenv(argv[1]);
if (value == NULL) {
fprintf(stderr, "env var not found: %s", argv[1]);
return 1;
}
fprintf(stdout, "%s", value);
#endif
fflush(stdout);
return 0;
}
