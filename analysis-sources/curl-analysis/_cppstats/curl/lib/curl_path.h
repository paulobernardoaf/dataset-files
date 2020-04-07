#include "curl_setup.h"
#include <curl/curl.h>
#include "urldata.h"
#if defined(WIN32)
#undef PATH_MAX
#define PATH_MAX MAX_PATH
#if !defined(R_OK)
#define R_OK 4
#endif
#endif
#if !defined(PATH_MAX)
#define PATH_MAX 1024 
#endif
CURLcode Curl_getworkingpath(struct connectdata *conn,
char *homedir,
char **path);
CURLcode Curl_get_pathname(const char **cpp, char **path, char *homedir);
