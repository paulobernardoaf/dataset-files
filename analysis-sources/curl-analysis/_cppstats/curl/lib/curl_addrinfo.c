#include "curl_setup.h"
#include <curl/curl.h>
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_NETINET_IN6_H)
#include <netinet/in6.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_SYS_UN_H)
#include <sys/un.h>
#endif
#if defined(__VMS)
#include <in.h>
#include <inet.h>
#endif
#if defined(NETWARE) && defined(__NOVELL_LIBC__)
#undef in_addr_t
#define in_addr_t unsigned long
#endif
#include <stddef.h>
#include "curl_addrinfo.h"
#include "inet_pton.h"
#include "warnless.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"
#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER == 910) && defined(__OPTIMIZE__) && defined(__unix__) && defined(__i386__)
#define vqualifier volatile
#else
#define vqualifier
#endif
void
Curl_freeaddrinfo(Curl_addrinfo *cahead)
{
Curl_addrinfo *vqualifier canext;
Curl_addrinfo *ca;
for(ca = cahead; ca != NULL; ca = canext) {
free(ca->ai_addr);
free(ca->ai_canonname);
canext = ca->ai_next;
free(ca);
}
}
#if defined(HAVE_GETADDRINFO)
int
Curl_getaddrinfo_ex(const char *nodename,
const char *servname,
const struct addrinfo *hints,
Curl_addrinfo **result)
{
const struct addrinfo *ai;
struct addrinfo *aihead;
Curl_addrinfo *cafirst = NULL;
Curl_addrinfo *calast = NULL;
Curl_addrinfo *ca;
size_t ss_size;
int error;
*result = NULL; 
error = getaddrinfo(nodename, servname, hints, &aihead);
if(error)
return error;
for(ai = aihead; ai != NULL; ai = ai->ai_next) {
if(ai->ai_family == AF_INET)
ss_size = sizeof(struct sockaddr_in);
#if defined(ENABLE_IPV6)
else if(ai->ai_family == AF_INET6)
ss_size = sizeof(struct sockaddr_in6);
#endif
else
continue;
if((ai->ai_addr == NULL) || !(ai->ai_addrlen > 0))
continue;
if((size_t)ai->ai_addrlen < ss_size)
continue;
ca = malloc(sizeof(Curl_addrinfo));
if(!ca) {
error = EAI_MEMORY;
break;
}
ca->ai_flags = ai->ai_flags;
ca->ai_family = ai->ai_family;
ca->ai_socktype = ai->ai_socktype;
ca->ai_protocol = ai->ai_protocol;
ca->ai_addrlen = (curl_socklen_t)ss_size;
ca->ai_addr = NULL;
ca->ai_canonname = NULL;
ca->ai_next = NULL;
ca->ai_addr = malloc(ss_size);
if(!ca->ai_addr) {
error = EAI_MEMORY;
free(ca);
break;
}
memcpy(ca->ai_addr, ai->ai_addr, ss_size);
if(ai->ai_canonname != NULL) {
ca->ai_canonname = strdup(ai->ai_canonname);
if(!ca->ai_canonname) {
error = EAI_MEMORY;
free(ca->ai_addr);
free(ca);
break;
}
}
if(!cafirst)
cafirst = ca;
if(calast)
calast->ai_next = ca;
calast = ca;
}
if(aihead)
freeaddrinfo(aihead);
if(error) {
Curl_freeaddrinfo(cafirst);
cafirst = NULL;
}
else if(!cafirst) {
#if defined(EAI_NONAME)
error = EAI_NONAME;
#else
error = EAI_NODATA;
#endif
#if defined(USE_WINSOCK)
SET_SOCKERRNO(error);
#endif
}
*result = cafirst;
return error;
}
#endif 
Curl_addrinfo *
Curl_he2ai(const struct hostent *he, int port)
{
Curl_addrinfo *ai;
Curl_addrinfo *prevai = NULL;
Curl_addrinfo *firstai = NULL;
struct sockaddr_in *addr;
#if defined(ENABLE_IPV6)
struct sockaddr_in6 *addr6;
#endif
CURLcode result = CURLE_OK;
int i;
char *curr;
if(!he)
return NULL;
DEBUGASSERT((he->h_name != NULL) && (he->h_addr_list != NULL));
for(i = 0; (curr = he->h_addr_list[i]) != NULL; i++) {
size_t ss_size;
#if defined(ENABLE_IPV6)
if(he->h_addrtype == AF_INET6)
ss_size = sizeof(struct sockaddr_in6);
else
#endif
ss_size = sizeof(struct sockaddr_in);
ai = calloc(1, sizeof(Curl_addrinfo));
if(!ai) {
result = CURLE_OUT_OF_MEMORY;
break;
}
ai->ai_canonname = strdup(he->h_name);
if(!ai->ai_canonname) {
result = CURLE_OUT_OF_MEMORY;
free(ai);
break;
}
ai->ai_addr = calloc(1, ss_size);
if(!ai->ai_addr) {
result = CURLE_OUT_OF_MEMORY;
free(ai->ai_canonname);
free(ai);
break;
}
if(!firstai)
firstai = ai;
if(prevai)
prevai->ai_next = ai;
ai->ai_family = he->h_addrtype;
ai->ai_socktype = SOCK_STREAM;
ai->ai_addrlen = (curl_socklen_t)ss_size;
switch(ai->ai_family) {
case AF_INET:
addr = (void *)ai->ai_addr; 
memcpy(&addr->sin_addr, curr, sizeof(struct in_addr));
addr->sin_family = (CURL_SA_FAMILY_T)(he->h_addrtype);
addr->sin_port = htons((unsigned short)port);
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
addr6 = (void *)ai->ai_addr; 
memcpy(&addr6->sin6_addr, curr, sizeof(struct in6_addr));
addr6->sin6_family = (CURL_SA_FAMILY_T)(he->h_addrtype);
addr6->sin6_port = htons((unsigned short)port);
break;
#endif
}
prevai = ai;
}
if(result) {
Curl_freeaddrinfo(firstai);
firstai = NULL;
}
return firstai;
}
struct namebuff {
struct hostent hostentry;
union {
struct in_addr ina4;
#if defined(ENABLE_IPV6)
struct in6_addr ina6;
#endif
} addrentry;
char *h_addr_list[2];
};
Curl_addrinfo *
Curl_ip2addr(int af, const void *inaddr, const char *hostname, int port)
{
Curl_addrinfo *ai;
#if defined(__VMS) && defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
#pragma pointer_size save
#pragma pointer_size short
#pragma message disable PTRMISMATCH
#endif
struct hostent *h;
struct namebuff *buf;
char *addrentry;
char *hoststr;
size_t addrsize;
DEBUGASSERT(inaddr && hostname);
buf = malloc(sizeof(struct namebuff));
if(!buf)
return NULL;
hoststr = strdup(hostname);
if(!hoststr) {
free(buf);
return NULL;
}
switch(af) {
case AF_INET:
addrsize = sizeof(struct in_addr);
addrentry = (void *)&buf->addrentry.ina4;
memcpy(addrentry, inaddr, sizeof(struct in_addr));
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
addrsize = sizeof(struct in6_addr);
addrentry = (void *)&buf->addrentry.ina6;
memcpy(addrentry, inaddr, sizeof(struct in6_addr));
break;
#endif
default:
free(hoststr);
free(buf);
return NULL;
}
h = &buf->hostentry;
h->h_name = hoststr;
h->h_aliases = NULL;
h->h_addrtype = (short)af;
h->h_length = (short)addrsize;
h->h_addr_list = &buf->h_addr_list[0];
h->h_addr_list[0] = addrentry;
h->h_addr_list[1] = NULL; 
#if defined(__VMS) && defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
#pragma pointer_size restore
#pragma message enable PTRMISMATCH
#endif
ai = Curl_he2ai(h, port);
free(hoststr);
free(buf);
return ai;
}
Curl_addrinfo *Curl_str2addr(char *address, int port)
{
struct in_addr in;
if(Curl_inet_pton(AF_INET, address, &in) > 0)
return Curl_ip2addr(AF_INET, &in, address, port);
#if defined(ENABLE_IPV6)
{
struct in6_addr in6;
if(Curl_inet_pton(AF_INET6, address, &in6) > 0)
return Curl_ip2addr(AF_INET6, &in6, address, port);
}
#endif
return NULL; 
}
#if defined(USE_UNIX_SOCKETS)
Curl_addrinfo *Curl_unix2addr(const char *path, bool *longpath, bool abstract)
{
Curl_addrinfo *ai;
struct sockaddr_un *sa_un;
size_t path_len;
*longpath = FALSE;
ai = calloc(1, sizeof(Curl_addrinfo));
if(!ai)
return NULL;
ai->ai_addr = calloc(1, sizeof(struct sockaddr_un));
if(!ai->ai_addr) {
free(ai);
return NULL;
}
sa_un = (void *) ai->ai_addr;
sa_un->sun_family = AF_UNIX;
path_len = strlen(path) + 1;
if(path_len > sizeof(sa_un->sun_path)) {
free(ai->ai_addr);
free(ai);
*longpath = TRUE;
return NULL;
}
ai->ai_family = AF_UNIX;
ai->ai_socktype = SOCK_STREAM; 
ai->ai_addrlen = (curl_socklen_t)
((offsetof(struct sockaddr_un, sun_path) + path_len) & 0x7FFFFFFF);
if(abstract)
memcpy(sa_un->sun_path + 1, path, path_len - 1);
else
memcpy(sa_un->sun_path, path, path_len); 
return ai;
}
#endif
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO) && defined(HAVE_FREEADDRINFO)
void
curl_dbg_freeaddrinfo(struct addrinfo *freethis,
int line, const char *source)
{
curl_dbg_log("ADDR %s:%d freeaddrinfo(%p)\n",
source, line, (void *)freethis);
#if defined(USE_LWIPSOCK)
lwip_freeaddrinfo(freethis);
#else
(freeaddrinfo)(freethis);
#endif
}
#endif 
#if defined(CURLDEBUG) && defined(HAVE_GETADDRINFO)
int
curl_dbg_getaddrinfo(const char *hostname,
const char *service,
const struct addrinfo *hints,
struct addrinfo **result,
int line, const char *source)
{
#if defined(USE_LWIPSOCK)
int res = lwip_getaddrinfo(hostname, service, hints, result);
#else
int res = (getaddrinfo)(hostname, service, hints, result);
#endif
if(0 == res)
curl_dbg_log("ADDR %s:%d getaddrinfo() = %p\n",
source, line, (void *)*result);
else
curl_dbg_log("ADDR %s:%d getaddrinfo() failed\n",
source, line);
return res;
}
#endif 
#if defined(HAVE_GETADDRINFO) && defined(USE_RESOLVE_ON_IPS)
void Curl_addrinfo_set_port(Curl_addrinfo *addrinfo, int port)
{
Curl_addrinfo *ca;
struct sockaddr_in *addr;
#if defined(ENABLE_IPV6)
struct sockaddr_in6 *addr6;
#endif
for(ca = addrinfo; ca != NULL; ca = ca->ai_next) {
switch(ca->ai_family) {
case AF_INET:
addr = (void *)ca->ai_addr; 
addr->sin_port = htons((unsigned short)port);
break;
#if defined(ENABLE_IPV6)
case AF_INET6:
addr6 = (void *)ca->ai_addr; 
addr6->sin6_port = htons((unsigned short)port);
break;
#endif
}
}
}
#endif
