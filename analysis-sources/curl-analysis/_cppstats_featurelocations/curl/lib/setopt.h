#if !defined(HEADER_CURL_SETOPT_H)
#define HEADER_CURL_SETOPT_H






















CURLcode Curl_setstropt(char **charp, const char *s);
CURLcode Curl_vsetopt(struct Curl_easy *data, CURLoption option, va_list arg);

#endif 
