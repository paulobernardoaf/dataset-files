#include <curl/curl.h>
#define strcasecompare(a,b) Curl_strcasecompare(a,b)
#define strncasecompare(a,b,c) Curl_strncasecompare(a,b,c)
int Curl_strcasecompare(const char *first, const char *second);
int Curl_safe_strcasecompare(const char *first, const char *second);
int Curl_strncasecompare(const char *first, const char *second, size_t max);
char Curl_raw_toupper(char in);
#define checkprefix(a,b) curl_strnequal(a,b,strlen(a))
void Curl_strntoupper(char *dest, const char *src, size_t n);
void Curl_strntolower(char *dest, const char *src, size_t n);
