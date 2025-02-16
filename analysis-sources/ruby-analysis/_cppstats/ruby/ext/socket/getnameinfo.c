#include "ruby/config.h"
#if defined(RUBY_EXTCONF_H)
#include RUBY_EXTCONF_H
#endif
#include <stdio.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#if defined(HAVE_ARPA_NAMESER_H)
#include <arpa/nameser.h>
#endif
#include <netdb.h>
#if defined(HAVE_RESOLV_H)
#include <resolv.h>
#endif
#endif
#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER <= 1200
#include <windows.h>
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#define snprintf _snprintf
#endif
#include <string.h>
#include <stddef.h>
#if defined(SOCKS5)
#include <socks.h>
#endif
#if !defined(HAVE_TYPE_SOCKLEN_T)
typedef int socklen_t;
#endif
#include "addrinfo.h"
#include "sockport.h"
#include "rubysocket.h"
#define SUCCESS 0
#define ANY 0
#define YES 1
#define NO 0
struct sockinet {
u_char si_len;
u_char si_family;
u_short si_port;
};
static struct afd {
int a_af;
int a_addrlen;
int a_socklen;
int a_off;
} afdl [] = {
#if defined(INET6)
#define N_INET6 0
{PF_INET6, sizeof(struct in6_addr),
sizeof(struct sockaddr_in6),
offsetof(struct sockaddr_in6, sin6_addr)},
#define N_INET 1
#else
#define N_INET 0
#endif
{PF_INET, sizeof(struct in_addr),
sizeof(struct sockaddr_in),
offsetof(struct sockaddr_in, sin_addr)},
{0, 0, 0, 0},
};
#define ENI_NOSOCKET 0
#define ENI_NOSERVNAME 1
#define ENI_NOHOSTNAME 2
#define ENI_MEMORY 3
#define ENI_SYSTEM 4
#define ENI_FAMILY 5
#define ENI_SALEN 6
int
getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
struct afd *afd;
struct hostent *hp;
u_short port;
int family, len, i;
char *addr, *p;
u_long v4a;
#if defined(INET6)
u_char pfx;
#endif
int h_error;
char numserv[512];
char numaddr[512];
if (sa == NULL)
return ENI_NOSOCKET;
if (!VALIDATE_SOCKLEN(sa, salen)) return ENI_SALEN;
len = salen;
family = sa->sa_family;
for (i = 0; afdl[i].a_af; i++)
if (afdl[i].a_af == family) {
afd = &afdl[i];
goto found;
}
return ENI_FAMILY;
found:
if (len != afd->a_socklen) return ENI_SALEN;
port = ((struct sockinet *)sa)->si_port; 
addr = (char *)sa + afd->a_off;
if (serv == NULL || servlen == 0) {
} else if (flags & NI_NUMERICSERV) {
snprintf(numserv, sizeof(numserv), "%d", ntohs(port));
if (strlen(numserv) + 1 > servlen)
return ENI_MEMORY;
strcpy(serv, numserv);
} else {
#if defined(HAVE_GETSERVBYPORT)
struct servent *sp = getservbyport(port, (flags & NI_DGRAM) ? "udp" : "tcp");
if (sp) {
if (strlen(sp->s_name) + 1 > servlen)
return ENI_MEMORY;
strcpy(serv, sp->s_name);
} else
return ENI_NOSERVNAME;
#else
return ENI_NOSERVNAME;
#endif
}
switch (sa->sa_family) {
case AF_INET:
v4a = ntohl(((struct sockaddr_in *)sa)->sin_addr.s_addr);
if (IN_MULTICAST(v4a) || IN_EXPERIMENTAL(v4a))
flags |= NI_NUMERICHOST;
v4a >>= IN_CLASSA_NSHIFT;
if (v4a == 0)
flags |= NI_NUMERICHOST;
break;
#if defined(INET6)
case AF_INET6:
#if defined(HAVE_ADDR8)
pfx = ((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr8[0];
#else
pfx = ((struct sockaddr_in6 *)sa)->sin6_addr.s6_addr[0];
#endif
if (pfx == 0 || pfx == 0xfe || pfx == 0xff)
flags |= NI_NUMERICHOST;
break;
#endif
}
if (host == NULL || hostlen == 0) {
} else if (flags & NI_NUMERICHOST) {
if (inet_ntop(afd->a_af, addr, numaddr, sizeof(numaddr))
== NULL)
return ENI_SYSTEM;
if (strlen(numaddr) > hostlen)
return ENI_MEMORY;
strcpy(host, numaddr);
} else {
#if defined(INET6)
hp = getipnodebyaddr(addr, afd->a_addrlen, afd->a_af, &h_error);
#else
hp = gethostbyaddr(addr, afd->a_addrlen, afd->a_af);
h_error = h_errno;
#endif
if (hp) {
if (flags & NI_NOFQDN) {
p = strchr(hp->h_name, '.');
if (p) *p = '\0';
}
if (strlen(hp->h_name) + 1 > hostlen) {
#if defined(INET6)
freehostent(hp);
#endif
return ENI_MEMORY;
}
strcpy(host, hp->h_name);
#if defined(INET6)
freehostent(hp);
#endif
} else {
if (flags & NI_NAMEREQD)
return ENI_NOHOSTNAME;
if (inet_ntop(afd->a_af, addr, numaddr, sizeof(numaddr))
== NULL)
return ENI_NOHOSTNAME;
if (strlen(numaddr) > hostlen)
return ENI_MEMORY;
strcpy(host, numaddr);
}
}
return SUCCESS;
}
