#include "server_setup.h"
#if defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(_XOPEN_SOURCE_EXTENDED)
#include <arpa/inet.h>
#endif
#if defined(HAVE_NETDB_H)
#include <netdb.h>
#endif
#define ENABLE_CURLX_PRINTF
#include "curlx.h" 
#include "util.h"
#include "memdebug.h"
static bool use_ipv6 = FALSE;
static const char *ipv_inuse = "IPv4";
const char *serverlogfile = ""; 
int main(int argc, char *argv[])
{
int arg = 1;
const char *host = NULL;
int rc = 0;
while(argc>arg) {
if(!strcmp("--version", argv[arg])) {
printf("resolve IPv4%s\n",
#if defined(CURLRES_IPV6)
"/IPv6"
#else
""
#endif
);
return 0;
}
else if(!strcmp("--ipv6", argv[arg])) {
ipv_inuse = "IPv6";
use_ipv6 = TRUE;
arg++;
}
else if(!strcmp("--ipv4", argv[arg])) {
ipv_inuse = "IPv4";
use_ipv6 = FALSE;
arg++;
}
else {
host = argv[arg++];
}
}
if(!host) {
puts("Usage: resolve [option] <host>\n"
" --version\n"
" --ipv4"
#if defined(CURLRES_IPV6)
"\n --ipv6"
#endif
);
return 1;
}
#if defined(WIN32)
win32_init();
atexit(win32_cleanup);
#endif
#if defined(CURLRES_IPV6)
if(use_ipv6) {
curl_socket_t s = socket(PF_INET6, SOCK_DGRAM, 0);
if(s == CURL_SOCKET_BAD)
rc = -1;
else {
sclose(s);
}
}
if(rc == 0) {
struct addrinfo *ai;
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_family = use_ipv6 ? PF_INET6 : PF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_CANONNAME;
rc = (getaddrinfo)(host, "80", &hints, &ai);
if(rc == 0)
(freeaddrinfo)(ai);
}
#else
if(use_ipv6) {
puts("IPv6 support has been disabled in this program");
return 1;
}
else {
struct hostent *he;
he = gethostbyname(host);
rc = !he;
}
#endif
if(rc)
printf("Resolving %s '%s' didn't work\n", ipv_inuse, host);
return !!rc;
}
