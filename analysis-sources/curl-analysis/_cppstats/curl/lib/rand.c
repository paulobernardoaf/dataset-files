#include "curl_setup.h"
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <curl/curl.h>
#include "vtls/vtls.h"
#include "sendf.h"
#include "rand.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
static CURLcode randit(struct Curl_easy *data, unsigned int *rnd)
{
unsigned int r;
CURLcode result = CURLE_OK;
static unsigned int randseed;
static bool seeded = FALSE;
#if defined(CURLDEBUG)
char *force_entropy = getenv("CURL_ENTROPY");
if(force_entropy) {
if(!seeded) {
unsigned int seed = 0;
size_t elen = strlen(force_entropy);
size_t clen = sizeof(seed);
size_t min = elen < clen ? elen : clen;
memcpy((char *)&seed, force_entropy, min);
randseed = ntohl(seed);
seeded = TRUE;
}
else
randseed++;
*rnd = randseed;
return CURLE_OK;
}
#endif
result = Curl_ssl_random(data, (unsigned char *)rnd, sizeof(*rnd));
if(result != CURLE_NOT_BUILT_IN)
return result;
#if defined(RANDOM_FILE)
if(!seeded) {
int fd = open(RANDOM_FILE, O_RDONLY);
if(fd > -1) {
ssize_t nread = read(fd, &randseed, sizeof(randseed));
if(nread == sizeof(randseed))
seeded = TRUE;
close(fd);
}
}
#endif
if(!seeded) {
struct curltime now = Curl_now();
infof(data, "WARNING: Using weak random seed\n");
randseed += (unsigned int)now.tv_usec + (unsigned int)now.tv_sec;
randseed = randseed * 1103515245 + 12345;
randseed = randseed * 1103515245 + 12345;
randseed = randseed * 1103515245 + 12345;
seeded = TRUE;
}
r = randseed = randseed * 1103515245 + 12345;
*rnd = (r << 16) | ((r >> 16) & 0xFFFF);
return CURLE_OK;
}
CURLcode Curl_rand(struct Curl_easy *data, unsigned char *rnd, size_t num)
{
CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
DEBUGASSERT(num > 0);
while(num) {
unsigned int r;
size_t left = num < sizeof(unsigned int) ? num : sizeof(unsigned int);
result = randit(data, &r);
if(result)
return result;
while(left) {
*rnd++ = (unsigned char)(r & 0xFF);
r >>= 8;
--num;
--left;
}
}
return result;
}
CURLcode Curl_rand_hex(struct Curl_easy *data, unsigned char *rnd,
size_t num)
{
CURLcode result = CURLE_BAD_FUNCTION_ARGUMENT;
const char *hex = "0123456789abcdef";
unsigned char buffer[128];
unsigned char *bufp = buffer;
DEBUGASSERT(num > 1);
#if defined(__clang_analyzer__)
memset(buffer, 0, sizeof(buffer));
#endif
if((num/2 >= sizeof(buffer)) || !(num&1))
return CURLE_BAD_FUNCTION_ARGUMENT;
num--; 
result = Curl_rand(data, buffer, num/2);
if(result)
return result;
while(num) {
*rnd++ = hex[(*bufp & 0xF0)>>4];
*rnd++ = hex[*bufp & 0x0F];
bufp++;
num -= 2;
}
*rnd = 0;
return result;
}
