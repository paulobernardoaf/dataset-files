
























#if !defined(VLC_NETWORK_H)
#define VLC_NETWORK_H











#include <sys/types.h>
#include <unistd.h>

#if defined( _WIN32 )
#define _NO_OLDNAMES 1
#include <winsock2.h>
#include <ws2tcpip.h>
#define net_errno (WSAGetLastError())
#define net_Close(fd) ((void)closesocket((SOCKET)fd))
#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define net_errno errno
#define net_Close(fd) ((void)vlc_close(fd))
#endif
















VLC_API int vlc_socket(int pf, int type, int proto, bool nonblock) VLC_USED;
















VLC_API int vlc_socketpair(int pf, int type, int proto, int fds[2],
bool nonblock);

struct sockaddr;














VLC_API int vlc_accept(int lfd, struct sockaddr *addr, socklen_t *alen,
bool nonblock) VLC_USED;
















VLC_API ssize_t vlc_send(int fd, const void *buf, size_t buflen, int flags);





















VLC_API ssize_t vlc_sendto(int fd, const void *buf, size_t buflen, int flags,
const struct sockaddr *dst, socklen_t dstlen);













VLC_API ssize_t vlc_sendmsg(int fd, const struct msghdr *msg, int flags);

#if defined(__cplusplus)
extern "C" {
#endif


int net_Socket (vlc_object_t *obj, int family, int socktype, int proto);

VLC_API int net_Connect(vlc_object_t *p_this, const char *psz_host, int i_port, int socktype, int protocol);
#define net_Connect(a, b, c, d, e) net_Connect(VLC_OBJECT(a), b, c, d, e)

VLC_API int * net_Listen(vlc_object_t *p_this, const char *psz_host, unsigned i_port, int socktype, int protocol);

#define net_ListenTCP(a, b, c) net_Listen(VLC_OBJECT(a), b, c, SOCK_STREAM, IPPROTO_TCP)


VLC_API int net_ConnectTCP (vlc_object_t *obj, const char *host, int port);
#define net_ConnectTCP(a, b, c) net_ConnectTCP(VLC_OBJECT(a), b, c)

















VLC_API int net_Accept(vlc_object_t *obj, int *fds);
#define net_Accept(a, b) net_Accept(VLC_OBJECT(a), b)


VLC_API int net_ConnectDgram( vlc_object_t *p_this, const char *psz_host, unsigned i_port, int hlim, int proto );
#define net_ConnectDgram(a, b, c, d, e ) net_ConnectDgram(VLC_OBJECT(a), b, c, d, e)


static inline int net_ConnectUDP (vlc_object_t *obj, const char *host, unsigned port, int hlim)
{
return net_ConnectDgram (obj, host, port, hlim, IPPROTO_UDP);
}

VLC_API int net_OpenDgram( vlc_object_t *p_this, const char *psz_bind, unsigned i_bind, const char *psz_server, unsigned i_server, int proto );
#define net_OpenDgram( a, b, c, d, e, g ) net_OpenDgram(VLC_OBJECT(a), b, c, d, e, g)


static inline int net_ListenUDP1 (vlc_object_t *obj, const char *host, unsigned port)
{
return net_OpenDgram (obj, host, port, NULL, 0, IPPROTO_UDP);
}

VLC_API void net_ListenClose( int *fd );

VLC_API int net_SetCSCov( int fd, int sendcov, int recvcov );










VLC_API ssize_t net_Read( vlc_object_t *p_this, int fd, void *p_data, size_t i_data );
#define net_Read(a,b,c,d) net_Read(VLC_OBJECT(a),b,c,d)











VLC_API ssize_t net_Write( vlc_object_t *p_this, int fd, const void *p_data, size_t i_data );
#define net_Write(a,b,c,d) net_Write(VLC_OBJECT(a),b,c,d)

VLC_API int vlc_close(int);



#if defined(_WIN32)
static inline int vlc_getsockopt(int s, int level, int name,
void *val, socklen_t *len)
{
return getsockopt(s, level, name, (char *)val, len);
}
#define getsockopt vlc_getsockopt

static inline int vlc_setsockopt(int s, int level, int name,
const void *val, socklen_t len)
{
return setsockopt(s, level, name, (const char *)val, len);
}
#define setsockopt vlc_setsockopt
#endif



#define NI_MAXNUMERICHOST 64

#if !defined(AI_NUMERICSERV)
#define AI_NUMERICSERV 0
#endif
#if !defined(AI_IDN)
#define AI_IDN 0 
#endif

#if defined(_WIN32)
#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#undef gai_strerror
#define gai_strerror gai_strerrorA
#endif
#endif

VLC_API int vlc_getnameinfo( const struct sockaddr *, int, char *, int, int *, int );
VLC_API int vlc_getaddrinfo (const char *, unsigned,
const struct addrinfo *, struct addrinfo **);
VLC_API int vlc_getaddrinfo_i11e(const char *, unsigned,
const struct addrinfo *, struct addrinfo **);

static inline bool
net_SockAddrIsMulticast (const struct sockaddr *addr, socklen_t len)
{
switch (addr->sa_family)
{
#if defined(IN_MULTICAST)
case AF_INET:
{
const struct sockaddr_in *v4 = (const struct sockaddr_in *)addr;
if ((size_t)len < sizeof (*v4))
return false;
return IN_MULTICAST (ntohl (v4->sin_addr.s_addr)) != 0;
}
#endif

#if defined(IN6_IS_ADDR_MULTICAST)
case AF_INET6:
{
const struct sockaddr_in6 *v6 = (const struct sockaddr_in6 *)addr;
if ((size_t)len < sizeof (*v6))
return false;
return IN6_IS_ADDR_MULTICAST (&v6->sin6_addr) != 0;
}
#endif
}

return false;
}


static inline int net_GetSockAddress( int fd, char *address, int *port )
{
struct sockaddr_storage addr;
socklen_t addrlen = sizeof( addr );

return getsockname( fd, (struct sockaddr *)&addr, &addrlen )
|| vlc_getnameinfo( (struct sockaddr *)&addr, addrlen, address,
NI_MAXNUMERICHOST, port, NI_NUMERICHOST )
? VLC_EGENERIC : 0;
}

static inline int net_GetPeerAddress( int fd, char *address, int *port )
{
struct sockaddr_storage addr;
socklen_t addrlen = sizeof( addr );

return getpeername( fd, (struct sockaddr *)&addr, &addrlen )
|| vlc_getnameinfo( (struct sockaddr *)&addr, addrlen, address,
NI_MAXNUMERICHOST, port, NI_NUMERICHOST )
? VLC_EGENERIC : 0;
}

VLC_API char *vlc_getProxyUrl(const char *);

#if defined(__cplusplus)
}
#endif



#endif
