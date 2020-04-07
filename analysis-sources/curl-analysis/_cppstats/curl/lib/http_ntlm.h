#include "curl_setup.h"
#if !defined(CURL_DISABLE_HTTP) && defined(USE_NTLM)
CURLcode Curl_input_ntlm(struct connectdata *conn, bool proxy,
const char *header);
CURLcode Curl_output_ntlm(struct connectdata *conn, bool proxy);
void Curl_http_auth_cleanup_ntlm(struct connectdata *conn);
#endif 
