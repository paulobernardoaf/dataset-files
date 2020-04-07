#include "curl_setup.h"
#if defined(USE_NSS)
#include "urldata.h"
CURLcode Curl_nss_force_init(struct Curl_easy *data);
extern const struct Curl_ssl Curl_ssl_nss;
#endif 
