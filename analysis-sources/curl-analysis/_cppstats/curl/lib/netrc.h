#include "curl_setup.h"
#if !defined(CURL_DISABLE_NETRC)
int Curl_parsenetrc(const char *host,
char **loginp,
char **passwordp,
bool *login_changed,
bool *password_changed,
char *filename);
#else
#define Curl_parsenetrc(a,b,c,d,e,f) 1
#endif
