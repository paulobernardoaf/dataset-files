#include "urldata.h"
#define STRERROR_LEN 256 
const char *Curl_strerror(int err, char *buf, size_t buflen);
#if defined(WIN32) || defined(_WIN32_WCE)
const char *Curl_winapi_strerror(DWORD err, char *buf, size_t buflen);
#endif
#if defined(USE_WINDOWS_SSPI)
const char *Curl_sspi_strerror(int err, char *buf, size_t buflen);
#endif
