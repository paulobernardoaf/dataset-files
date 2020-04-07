#include <vlc_network.h>
typedef struct vlc_tls
{
const struct vlc_tls_operations *ops;
struct vlc_tls *p;
} vlc_tls_t;
struct vlc_tls_operations
{
int (*get_fd)(struct vlc_tls *, short *events);
ssize_t (*readv)(struct vlc_tls *, struct iovec *iov, unsigned len);
ssize_t (*writev)(struct vlc_tls *, const struct iovec *iov, unsigned len);
int (*shutdown)(struct vlc_tls *, bool duplex);
void (*close)(struct vlc_tls *);
};
typedef struct vlc_tls_client
{
struct vlc_object_t obj;
const struct vlc_tls_client_operations *ops;
void *sys;
} vlc_tls_client_t;
struct vlc_tls_client_operations
{
vlc_tls_t *(*open)(struct vlc_tls_client *, vlc_tls_t *sock,
const char *host, const char *const *alpn);
int (*handshake)(vlc_tls_t *session,
const char *hostname, const char *service,
char ** alp);
void (*destroy)(struct vlc_tls_client *);
};
VLC_API vlc_tls_client_t *vlc_tls_ClientCreate(vlc_object_t *);
VLC_API void vlc_tls_ClientDelete(vlc_tls_client_t *);
VLC_API vlc_tls_t *vlc_tls_ClientSessionCreate(vlc_tls_client_t *creds,
vlc_tls_t *sock,
const char *host,
const char *service,
const char *const *alpn,
char **alp);
typedef struct vlc_tls_server
{
struct vlc_object_t obj;
const struct vlc_tls_server_operations *ops;
void *sys;
} vlc_tls_server_t;
struct vlc_tls_server_operations
{
vlc_tls_t *(*open)(struct vlc_tls_server *, vlc_tls_t *sock,
const char *const *alpn);
int (*handshake)(vlc_tls_t *session, char ** alp);
void (*destroy)(struct vlc_tls_server *);
};
VLC_API vlc_tls_server_t *vlc_tls_ServerCreate(vlc_object_t *,
const char *cert,
const char *key);
static inline int vlc_tls_SessionHandshake(vlc_tls_server_t *crd,
vlc_tls_t *tls)
{
return crd->ops->handshake(tls, NULL);
}
VLC_API vlc_tls_t *vlc_tls_ServerSessionCreate(vlc_tls_server_t *creds,
vlc_tls_t *sock,
const char *const *alpn);
VLC_API void vlc_tls_ServerDelete(vlc_tls_server_t *);
VLC_API void vlc_tls_SessionDelete (vlc_tls_t *);
static inline int vlc_tls_GetPollFD(vlc_tls_t *tls, short *events)
{
return tls->ops->get_fd(tls, events);
}
static inline int vlc_tls_GetFD(vlc_tls_t *tls)
{
short events = 0;
return vlc_tls_GetPollFD(tls, &events);
}
VLC_API ssize_t vlc_tls_Read(vlc_tls_t *, void *buf, size_t len, bool waitall);
VLC_API char *vlc_tls_GetLine(vlc_tls_t *);
VLC_API ssize_t vlc_tls_Write(vlc_tls_t *, const void *buf, size_t len);
static inline int vlc_tls_Shutdown(vlc_tls_t *tls, bool duplex)
{
return tls->ops->shutdown(tls, duplex);
}
static inline void vlc_tls_Close(vlc_tls_t *session)
{
do
{
vlc_tls_t *p = session->p;
vlc_tls_SessionDelete(session);
session = p;
}
while (session != NULL);
}
VLC_API vlc_tls_t *vlc_tls_SocketOpen(int fd);
VLC_API int vlc_tls_SocketPair(int family, int protocol, vlc_tls_t *[2]);
struct addrinfo;
VLC_API vlc_tls_t *vlc_tls_SocketOpenAddrInfo(const struct addrinfo *ai,
bool defer_connect);
VLC_API vlc_tls_t *vlc_tls_SocketOpenTCP(vlc_object_t *obj,
const char *hostname, unsigned port);
VLC_API vlc_tls_t *vlc_tls_SocketOpenTLS(vlc_tls_client_t *crd,
const char *hostname, unsigned port,
const char *service,
const char *const *alpn, char **alp);
