#if !defined(HEADER_CURL_ESCAPE_H)
#define HEADER_CURL_ESCAPE_H
























bool Curl_isunreserved(unsigned char in);
CURLcode Curl_urldecode(struct Curl_easy *data,
const char *string, size_t length,
char **ostring, size_t *olen,
bool reject_crlf);

#endif 
