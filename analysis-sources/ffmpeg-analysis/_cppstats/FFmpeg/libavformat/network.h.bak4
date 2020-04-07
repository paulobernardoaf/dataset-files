



















#if !defined(AVFORMAT_NETWORK_H)
#define AVFORMAT_NETWORK_H

#include <errno.h>
#include <stdint.h>

#include "config.h"
#include "libavutil/error.h"
#include "os_support.h"
#include "avio.h"
#include "url.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_WINSOCK2_H
#include <winsock2.h>
#include <ws2tcpip.h>

#if !defined(EPROTONOSUPPORT)
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#endif
#if !defined(ETIMEDOUT)
#define ETIMEDOUT WSAETIMEDOUT
#endif
#if !defined(ECONNREFUSED)
#define ECONNREFUSED WSAECONNREFUSED
#endif
#if !defined(EINPROGRESS)
#define EINPROGRESS WSAEINPROGRESS
#endif
#if !defined(ENOTCONN)
#define ENOTCONN WSAENOTCONN
#endif

#define getsockopt(a, b, c, d, e) getsockopt(a, b, c, (char*) d, e)
#define setsockopt(a, b, c, d, e) setsockopt(a, b, c, (const char*) d, e)

int ff_neterrno(void);
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define ff_neterrno() AVERROR(errno)
#endif 

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_POLL_H
#include <poll.h>
#endif

int ff_socket_nonblock(int socket, int enable);

int ff_network_init(void);
void ff_network_close(void);

int ff_tls_init(void);
void ff_tls_deinit(void);

int ff_network_wait_fd(int fd, int write);











int ff_network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb);








int ff_network_sleep_interruptible(int64_t timeout, AVIOInterruptCB *int_cb);

#if !HAVE_STRUCT_SOCKADDR_STORAGE
struct sockaddr_storage {
#if HAVE_STRUCT_SOCKADDR_SA_LEN
uint8_t ss_len;
uint8_t ss_family;
#else
uint16_t ss_family;
#endif 
char ss_pad1[6];
int64_t ss_align;
char ss_pad2[112];
};
#endif 

typedef union sockaddr_union {
struct sockaddr_storage storage;
struct sockaddr_in in;
#if HAVE_STRUCT_SOCKADDR_IN6
struct sockaddr_in6 in6;
#endif
} sockaddr_union;

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

#if !HAVE_STRUCT_ADDRINFO
struct addrinfo {
int ai_flags;
int ai_family;
int ai_socktype;
int ai_protocol;
int ai_addrlen;
struct sockaddr *ai_addr;
char *ai_canonname;
struct addrinfo *ai_next;
};
#endif 


#if !defined(EAI_AGAIN)
#define EAI_AGAIN 2
#endif
#if !defined(EAI_BADFLAGS)
#define EAI_BADFLAGS 3
#endif
#if !defined(EAI_FAIL)
#define EAI_FAIL 4
#endif
#if !defined(EAI_FAMILY)
#define EAI_FAMILY 5
#endif
#if !defined(EAI_MEMORY)
#define EAI_MEMORY 6
#endif
#if !defined(EAI_NODATA)
#define EAI_NODATA 7
#endif
#if !defined(EAI_NONAME)
#define EAI_NONAME 8
#endif
#if !defined(EAI_SERVICE)
#define EAI_SERVICE 9
#endif
#if !defined(EAI_SOCKTYPE)
#define EAI_SOCKTYPE 10
#endif

#if !defined(AI_PASSIVE)
#define AI_PASSIVE 1
#endif

#if !defined(AI_CANONNAME)
#define AI_CANONNAME 2
#endif

#if !defined(AI_NUMERICHOST)
#define AI_NUMERICHOST 4
#endif

#if !defined(NI_NOFQDN)
#define NI_NOFQDN 1
#endif

#if !defined(NI_NUMERICHOST)
#define NI_NUMERICHOST 2
#endif

#if !defined(NI_NAMERQD)
#define NI_NAMERQD 4
#endif

#if !defined(NI_NUMERICSERV)
#define NI_NUMERICSERV 8
#endif

#if !defined(NI_DGRAM)
#define NI_DGRAM 16
#endif

#if !HAVE_GETADDRINFO
int ff_getaddrinfo(const char *node, const char *service,
const struct addrinfo *hints, struct addrinfo **res);
void ff_freeaddrinfo(struct addrinfo *res);
int ff_getnameinfo(const struct sockaddr *sa, int salen,
char *host, int hostlen,
char *serv, int servlen, int flags);
#define getaddrinfo ff_getaddrinfo
#define freeaddrinfo ff_freeaddrinfo
#define getnameinfo ff_getnameinfo
#endif 

#if !HAVE_GETADDRINFO || HAVE_WINSOCK2_H
const char *ff_gai_strerror(int ecode);
#undef gai_strerror
#define gai_strerror ff_gai_strerror
#endif 

#if !defined(INADDR_LOOPBACK)
#define INADDR_LOOPBACK 0x7f000001
#endif

#if !defined(INET_ADDRSTRLEN)
#define INET_ADDRSTRLEN 16
#endif

#if !defined(INET6_ADDRSTRLEN)
#define INET6_ADDRSTRLEN INET_ADDRSTRLEN
#endif

#if !defined(IN_MULTICAST)
#define IN_MULTICAST(a) ((((uint32_t)(a)) & 0xf0000000) == 0xe0000000)
#endif
#if !defined(IN6_IS_ADDR_MULTICAST)
#define IN6_IS_ADDR_MULTICAST(a) (((uint8_t *) (a))[0] == 0xff)
#endif

int ff_is_multicast_address(struct sockaddr *addr);

#define POLLING_TIME 100 













int ff_listen_bind(int fd, const struct sockaddr *addr,
socklen_t addrlen, int timeout,
URLContext *h);








int ff_listen(int fd, const struct sockaddr *addr, socklen_t addrlen);










int ff_accept(int fd, int timeout, URLContext *h);
















int ff_listen_connect(int fd, const struct sockaddr *addr,
socklen_t addrlen, int timeout,
URLContext *h, int will_try_next);

int ff_http_match_no_proxy(const char *no_proxy, const char *hostname);

int ff_socket(int domain, int type, int protocol);

void ff_log_net_error(void *ctx, int level, const char* prefix);

























int ff_connect_parallel(struct addrinfo *addrs, int timeout_ms_per_address,
int parallel, URLContext *h, int *fd,
void (*customize_fd)(void *, int), void *customize_ctx);

#endif 
