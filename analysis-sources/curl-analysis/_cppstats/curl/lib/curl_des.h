#include "curl_setup.h"
#if defined(USE_NTLM) && !defined(USE_OPENSSL)
void Curl_des_set_odd_parity(unsigned char *bytes, size_t length);
#endif 
