bool Curl_isunreserved(unsigned char in);
CURLcode Curl_urldecode(struct Curl_easy *data,
const char *string, size_t length,
char **ostring, size_t *olen,
bool reject_crlf);
