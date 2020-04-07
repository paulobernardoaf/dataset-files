






































#if !defined(RNET_H)
#define RNET_H

#include <limits.h> 
#include <inttypes.h> 







#define NOGDICAPMASKS 
#define NOVIRTUALKEYCODES 
#define NOWINMESSAGES 
#define NOWINSTYLES 
#define NOSYSMETRICS 
#define NOMENUS 
#define NOICONS 
#define NOKEYSTATES 
#define NOSYSCOMMANDS 
#define NORASTEROPS 
#define NOSHOWWINDOW 
#define OEMRESOURCE 
#define NOATOM 
#define NOCLIPBOARD 
#define NOCOLOR 
#define NOCTLMGR 
#define NODRAWTEXT 
#define NOGDI 
#define NOKERNEL 
#define NOUSER 
#define NONLS 
#define NOMB 
#define NOMEMMGR 
#define NOMETAFILE 
#define NOMINMAX 
#define NOMSG 
#define NOOPENFILE 
#define NOSCROLL 
#define NOSERVICE 
#define NOSOUND 
#define NOTEXTMETRIC 
#define NOWH 
#define NOWINOFFSETS 
#define NOCOMM 
#define NOKANJI 
#define NOHELP 
#define NOPROFILER 
#define NODEFERWINDOWPOS 
#define NOMCX 
#define MMNOSOUND


#if !defined(RNET_MALLOC)
#define RNET_MALLOC(sz) malloc(sz)
#endif
#if !defined(RNET_CALLOC)
#define RNET_CALLOC(n,sz) calloc(n,sz)
#endif
#if !defined(RNET_FREE)
#define RNET_FREE(p) free(p)
#endif






#if defined(WIN32)
typedef int socklen_t;
#endif

#if !defined(RESULT_SUCCESS)
#define RESULT_SUCCESS 0
#endif 

#if !defined(RESULT_FAILURE)
#define RESULT_FAILURE 1
#endif 

#if !defined(htonll)
#if defined(_BIG_ENDIAN)
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ((((uint64) htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64) ntohl(x)) << 32) + ntohl(x >> 32))
#endif 
#endif 







#if defined(_WIN32) 
#define __USE_W32_SOCKETS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#define IPTOS_LOWDELAY 0x10
#else 
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET ~(0)
#endif

#if !defined(__USE_W32_SOCKETS)
#define closesocket close
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#if defined(__USE_W32_SOCKETS)
#if !defined(EINTR)
#define EINTR WSAEINTR
#endif
#endif






#define SOCKET_MAX_SET_SIZE 32 
#define SOCKET_MAX_QUEUE_SIZE 16 
#define SOCKET_MAX_SOCK_OPTS 4 
#define SOCKET_MAX_UDPCHANNELS 32 
#define SOCKET_MAX_UDPADDRESSES 4 


#define ADDRESS_IPV4_ADDRSTRLEN 22 
#define ADDRESS_IPV6_ADDRSTRLEN 65 
#define ADDRESS_TYPE_ANY 0 
#define ADDRESS_TYPE_IPV4 2 
#define ADDRESS_TYPE_IPV6 23 
#define ADDRESS_MAXHOST 1025 
#define ADDRESS_MAXSERV 32 


#define ADDRESS_ANY (unsigned long)0x00000000
#define ADDRESS_LOOPBACK 0x7f000001
#define ADDRESS_BROADCAST (unsigned long)0xffffffff
#define ADDRESS_NONE 0xffffffff


#define NAME_INFO_DEFAULT 0x00 
#define NAME_INFO_NOFQDN 0x01 
#define NAME_INFO_NUMERICHOST 0x02 
#define NAME_INFO_NAMEREQD 0x04 
#define NAME_INFO_NUMERICSERV 0x08 
#define NAME_INFO_DGRAM 0x10 


#if defined(_WIN32)
#define ADDRESS_INFO_PASSIVE (0x00000001) 
#define ADDRESS_INFO_CANONNAME (0x00000002) 
#define ADDRESS_INFO_NUMERICHOST (0x00000004) 
#define ADDRESS_INFO_NUMERICSERV (0x00000008) 
#define ADDRESS_INFO_DNS_ONLY (0x00000010) 
#define ADDRESS_INFO_ALL (0x00000100) 
#define ADDRESS_INFO_ADDRCONFIG (0x00000400) 
#define ADDRESS_INFO_V4MAPPED (0x00000800) 
#define ADDRESS_INFO_NON_AUTHORITATIVE (0x00004000) 
#define ADDRESS_INFO_SECURE (0x00008000) 
#define ADDRESS_INFO_RETURN_PREFERRED_NAMES (0x00010000) 
#define ADDRESS_INFO_FQDN (0x00020000) 
#define ADDRESS_INFO_FILESERVER (0x00040000) 
#define ADDRESS_INFO_DISABLE_IDN_ENCODING (0x00080000) 
#define ADDRESS_INFO_EXTENDED (0x80000000) 
#define ADDRESS_INFO_RESOLUTION_HANDLE (0x40000000) 
#endif






#if defined(__STDC__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool)
typedef enum { false, true } bool;
#endif

typedef enum {
SOCKET_TCP = 0, 
SOCKET_UDP = 1 
} SocketType;


typedef uint32_t SocketChannel;
typedef struct _AddressInformation *AddressInformation;
typedef struct _SocketAddress *SocketAddress;
typedef struct _SocketAddressIPv4 *SocketAddressIPv4;
typedef struct _SocketAddressIPv6 *SocketAddressIPv6;
typedef struct _SocketAddressStorage *SocketAddressStorage;


typedef struct IPAddress {
unsigned long host; 
unsigned short port; 
} IPAddress;

typedef struct UDPChannel {
int numbound; 
IPAddress address[SOCKET_MAX_UDPADDRESSES]; 
} UDPChannel;


typedef struct SocketOpt {
int id; 
int valueLen; 
void *value; 
} SocketOpt;

typedef struct Socket {
int ready; 
int status; 
bool isServer; 
SocketChannel channel; 
SocketType type; 

bool isIPv6; 
SocketAddressIPv4 addripv4; 
SocketAddressIPv6 addripv6; 

struct UDPChannel binding[SOCKET_MAX_UDPCHANNELS]; 
} Socket;


typedef struct SocketConfig {
SocketType type; 
char *host; 
char *port; 
bool server; 
bool nonblocking; 
int backlog_size; 
SocketOpt sockopts[SOCKET_MAX_SOCK_OPTS];
} SocketConfig;

typedef struct SocketDataPacket {
IPAddress address; 
int channel; 
int maxlen; 
int status; 
unsigned int len; 
unsigned char *data; 
} SocketDataPacket;


typedef struct SocketResult {
int status; 
Socket *socket; 
} SocketResult;

typedef struct SocketSet {
int numsockets; 
int maxsockets; 
struct Socket **sockets; 
} SocketSet;


typedef struct Packet {
uint32_t size; 
uint32_t offs; 
uint32_t maxs; 
uint8_t *data; 
} Packet;


#if defined(__cplusplus)
extern "C" { 
#endif











bool InitNetworkDevice(void);
void CloseNetworkDevice(void);


void ResolveIP(const char *ip, const char *service, int flags, char *outhost, char *outserv);
int ResolveHost(const char *address, const char *service, int addressType, int flags, AddressInformation *outAddr);
int GetAddressFamily(AddressInformation address);
int GetAddressSocketType(AddressInformation address);
int GetAddressProtocol(AddressInformation address);
char *GetAddressCanonName(AddressInformation address);
char *GetAddressHostAndPort(AddressInformation address, char *outhost, int *outport);


AddressInformation LoadAddress(void);
void UnloadAddress(AddressInformation *addressInfo);
AddressInformation *LoadAddressList(int size);


bool SocketCreate(SocketConfig *config, SocketResult *result);
bool SocketBind(SocketConfig *config, SocketResult *result);
bool SocketListen(SocketConfig *config, SocketResult *result);
bool SocketConnect(SocketConfig *config, SocketResult *result);
Socket *SocketAccept(Socket *server, SocketConfig *config);


int SocketSend(Socket *sock, const void *datap, int len);
int SocketReceive(Socket *sock, void *data, int maxlen);
SocketAddressStorage SocketGetPeerAddress(Socket *sock);
char *GetSocketAddressHost(SocketAddressStorage storage);
short GetSocketAddressPort(SocketAddressStorage storage);
void SocketClose(Socket *sock);


int SocketSetChannel(Socket *socket, int channel, const IPAddress *address);
void SocketUnsetChannel(Socket *socket, int channel);


SocketDataPacket *AllocPacket(int size);
int ResizePacket(SocketDataPacket *packet, int newsize);
void FreePacket(SocketDataPacket *packet);
SocketDataPacket **AllocPacketList(int count, int size);
void FreePacketList(SocketDataPacket **packets);


Socket *LoadSocket(void);
void UnloadSocket(Socket **sock);
SocketResult *LoadSocketResult(void);
void UnloadSocketResult(SocketResult **result);
SocketSet *LoadSocketSet(int max);
void UnloadSocketSet(SocketSet *sockset);


bool IsSocketReady(Socket *sock);
bool IsSocketConnected(Socket *sock);
int AddSocket(SocketSet *set, Socket *sock);
int RemoveSocket(SocketSet *set, Socket *sock);
int CheckSockets(SocketSet *set, unsigned int timeout);


void PacketSend(Packet *packet);
void PacketReceive(Packet *packet);
void PacketWrite8(Packet *packet, uint16_t value);
void PacketWrite16(Packet *packet, uint16_t value);
void PacketWrite32(Packet *packet, uint32_t value);
void PacketWrite64(Packet *packet, uint64_t value);
uint16_t PacketRead8(Packet *packet);
uint16_t PacketRead16(Packet *packet);
uint32_t PacketRead32(Packet *packet);
uint64_t PacketRead64(Packet *packet);

#if defined(__cplusplus)
}
#endif

#endif 







#if defined(RNET_IMPLEMENTATION)

#include <assert.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#define NET_DEBUG_ENABLED 1

#if defined(SUPPORT_TRACELOG)
#define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

#if defined(SUPPORT_TRACELOG_DEBUG)
#define TRACELOGD(...) TraceLog(LOG_DEBUG, __VA_ARGS__)
#else
#define TRACELOGD(...) (void)0
#endif
#else
#define TRACELOG(level, ...) (void)0
#define TRACELOGD(...) (void)0
#endif





typedef struct _SocketAddress
{
struct sockaddr address;
} _SocketAddress;

typedef struct _SocketAddressIPv4
{
struct sockaddr_in address;
} _SocketAddressIPv4;

typedef struct _SocketAddressIPv6
{
struct sockaddr_in6 address;
} _SocketAddressIPv6;

typedef struct _SocketAddressStorage
{
struct sockaddr_storage address;
} _SocketAddressStorage;

typedef struct _AddressInformation
{
struct addrinfo addr;
} _AddressInformation;




static void PrintSocket(struct sockaddr_storage *addr, const int family, const int socktype, const int protocol);
static const char *SocketAddressToString(struct sockaddr_storage *sockaddr);
static bool IsIPv4Address(const char *ip);
static bool IsIPv6Address(const char *ip);
static void *GetSocketPortPtr(struct sockaddr_storage *sa);
static void *GetSocketAddressPtr(struct sockaddr_storage *sa);
static bool IsSocketValid(Socket *sock);
static void SocketSetLastError(int err);
static int SocketGetLastError();
static char *SocketGetLastErrorString();
static char *SocketErrorCodeToString(int err);
static bool SocketSetDefaults(SocketConfig *config);
static bool InitSocket(Socket *sock, struct addrinfo *addr);
static bool CreateSocket(SocketConfig *config, SocketResult *outresult);
static bool SocketSetBlocking(Socket *sock);
static bool SocketSetNonBlocking(Socket *sock);
static bool SocketSetOptions(SocketConfig *config, Socket *sock);
static void SocketSetHints(SocketConfig *config, struct addrinfo *hints);





static void PrintSocket(struct sockaddr_storage *addr, const int family, const int socktype, const int protocol)
{
switch (family)
{
case AF_UNSPEC: TRACELOG(LOG_DEBUG, "\tFamily: Unspecified"); break;
case AF_INET:
{
TRACELOG(LOG_DEBUG, "\tFamily: AF_INET (IPv4)");
TRACELOG(LOG_INFO, "\t- IPv4 address %s", SocketAddressToString(addr));
} break;
case AF_INET6:
{
TRACELOG(LOG_DEBUG, "\tFamily: AF_INET6 (IPv6)");
TRACELOG(LOG_INFO, "\t- IPv6 address %s", SocketAddressToString(addr));
} break;
case AF_NETBIOS:
{
TRACELOG(LOG_DEBUG, "\tFamily: AF_NETBIOS (NetBIOS)");
} break;
default: TRACELOG(LOG_DEBUG, "\tFamily: Other %ld", family); break;
}

TRACELOG(LOG_DEBUG, "\tSocket type:");
switch (socktype)
{
case 0: TRACELOG(LOG_DEBUG, "\t- Unspecified"); break;
case SOCK_STREAM: TRACELOG(LOG_DEBUG, "\t- SOCK_STREAM (stream)"); break;
case SOCK_DGRAM: TRACELOG(LOG_DEBUG, "\t- SOCK_DGRAM (datagram)"); break;
case SOCK_RAW: TRACELOG(LOG_DEBUG, "\t- SOCK_RAW (raw)"); break;
case SOCK_RDM: TRACELOG(LOG_DEBUG, "\t- SOCK_RDM (reliable message datagram)"); break;
case SOCK_SEQPACKET: TRACELOG(LOG_DEBUG, "\t- SOCK_SEQPACKET (pseudo-stream packet)"); break;
default: TRACELOG(LOG_DEBUG, "\t- Other %ld", socktype); break;
}

TRACELOG(LOG_DEBUG, "\tProtocol:");
switch (protocol)
{
case 0: TRACELOG(LOG_DEBUG, "\t- Unspecified"); break;
case IPPROTO_TCP: TRACELOG(LOG_DEBUG, "\t- IPPROTO_TCP (TCP)"); break;
case IPPROTO_UDP: TRACELOG(LOG_DEBUG, "\t- IPPROTO_UDP (UDP)"); break;
default: TRACELOG(LOG_DEBUG, "\t- Other %ld", protocol); break;
}
}


static const char *SocketAddressToString(struct sockaddr_storage *sockaddr)
{

assert(sockaddr != NULL);
assert(sockaddr->ss_family == AF_INET || sockaddr->ss_family == AF_INET6);

switch (sockaddr->ss_family)
{
case AF_INET:
{


}
break;
case AF_INET6:
{


}
break;
}

return NULL;
}


static bool IsIPv4Address(const char *ip)
{





return false;
}


static bool IsIPv6Address(const char *ip)
{





return false;
}


static void *GetSocketPortPtr(struct sockaddr_storage *sa)
{
if (sa->ss_family == AF_INET)
{
return &(((struct sockaddr_in *)sa)->sin_port);
}

return &(((struct sockaddr_in6 *)sa)->sin6_port);
}


static void *GetSocketAddressPtr(struct sockaddr_storage *sa)
{
if (sa->ss_family == AF_INET)
{
return &(((struct sockaddr_in *)sa)->sin_addr);
}

return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


static bool IsSocketValid(Socket *sock)
{
if (sock != NULL)
{
return (sock->channel != INVALID_SOCKET);
}

return false;
}


static void SocketSetLastError(int err)
{
#if defined(_WIN32)
WSASetLastError(err);
#else
errno = err;
#endif
}


static int SocketGetLastError(void)
{
#if defined(_WIN32)
return WSAGetLastError();
#else
return errno;
#endif
}


static char *SocketGetLastErrorString(void)
{
return SocketErrorCodeToString(SocketGetLastError());
}


static char *SocketErrorCodeToString(int err)
{
#if defined(_WIN32)
static char gaiStrErrorBuffer[GAI_STRERROR_BUFFER_SIZE];
TRACELOG(LOG_INFO, gaiStrErrorBuffer, "%s", gai_strerror(err));
return gaiStrErrorBuffer;
#else
return gai_strerror(err);
#endif
}


static bool SocketSetDefaults(SocketConfig *config)
{
if (config->backlog_size == 0) config->backlog_size = SOCKET_MAX_QUEUE_SIZE;

return true;
}


static bool InitSocket(Socket *sckt, struct addrinfo *address)
{
switch (sckt->type)
{
case SOCKET_TCP:
{
if (address->ai_family == AF_INET) sckt->channel = socket(AF_INET, SOCK_STREAM, 0);
else sckt->channel = socket(AF_INET6, SOCK_STREAM, 0);
} break;
case SOCKET_UDP:
{
if (address->ai_family == AF_INET) sckt->channel = socket(AF_INET, SOCK_DGRAM, 0);
else sckt->channel = socket(AF_INET6, SOCK_DGRAM, 0);
} break;
default: TRACELOG(LOG_WARNING, "Invalid socket type specified."); break;
}

return IsSocketValid(sckt);
}

















static bool CreateSocket(SocketConfig *config, SocketResult *outresult)
{
bool success = true;
int addrstatus;
struct addrinfo hints; 
struct addrinfo *res; 

outresult->socket->channel = INVALID_SOCKET;
outresult->status = RESULT_FAILURE;


outresult->socket->type = config->type;








SocketSetHints(config, &hints);


addrstatus = getaddrinfo(config->host, 
config->port, 
&hints, 
&res 
);


if (addrstatus != 0)
{
outresult->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(outresult->socket->status));
SocketSetLastError(0);
TRACELOG(LOG_WARNING, "Failed to get resolve host %s:%s: %s", config->host, config->port, SocketGetLastErrorString());

return (success = false);
}
else
{
char hoststr[NI_MAXHOST];
char portstr[NI_MAXSERV];


TRACELOG(LOG_INFO, "Successfully resolved host %s:%s", hoststr, portstr);
}


struct addrinfo *it;
for (it = res; it != NULL; it = it->ai_next)
{

if (!InitSocket(outresult->socket, it))
{
outresult->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(outresult->socket->status));
SocketSetLastError(0);
continue;
}


if (!SocketSetOptions(config, outresult->socket))
{
outresult->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(outresult->socket->status));
SocketSetLastError(0);
freeaddrinfo(res);

return (success = false);
}
}

if (!IsSocketValid(outresult->socket))
{
outresult->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(outresult->status));
SocketSetLastError(0);
freeaddrinfo(res);

return (success = false);
}

if (success)
{
outresult->status = RESULT_SUCCESS;
outresult->socket->ready = 0;
outresult->socket->status = 0;

if (!(config->type == SOCKET_UDP)) outresult->socket->isServer = config->server;

switch (res->ai_addr->sa_family)
{
case AF_INET:
{
outresult->socket->addripv4 = (struct _SocketAddressIPv4 *)RNET_MALLOC(sizeof(*outresult->socket->addripv4));

if (outresult->socket->addripv4 != NULL)
{
memset(outresult->socket->addripv4, 0, sizeof(*outresult->socket->addripv4));

if (outresult->socket->addripv4 != NULL)
{
memcpy(&outresult->socket->addripv4->address, (struct sockaddr_in *)res->ai_addr, sizeof(struct sockaddr_in));

outresult->socket->isIPv6 = false;
char hoststr[NI_MAXHOST];
char portstr[NI_MAXSERV];

socklen_t client_len = sizeof(struct sockaddr_storage);
getnameinfo((struct sockaddr *)&outresult->socket->addripv4->address, client_len, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);

TRACELOG(LOG_INFO, "Socket address set to %s:%s", hoststr, portstr);
}
}
} break;
case AF_INET6:
{
outresult->socket->addripv6 = (struct _SocketAddressIPv6 *)RNET_MALLOC(
sizeof(*outresult->socket->addripv6));
if (outresult->socket->addripv6 != NULL)
{
memset(outresult->socket->addripv6, 0,
sizeof(*outresult->socket->addripv6));
if (outresult->socket->addripv6 != NULL)
{
memcpy(&outresult->socket->addripv6->address,
(struct sockaddr_in6 *)res->ai_addr, sizeof(struct sockaddr_in6));
outresult->socket->isIPv6 = true;
char hoststr[NI_MAXHOST];
char portstr[NI_MAXSERV];
socklen_t client_len = sizeof(struct sockaddr_storage);
getnameinfo(
(struct sockaddr *)&outresult->socket->addripv6->address, client_len, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
TRACELOG(LOG_INFO, "Socket address set to %s:%s", hoststr, portstr);
}
}
} break;
default: break;
}
}

freeaddrinfo(res);
return success;
}


static bool SocketSetBlocking(Socket *sock)
{
bool ret = true;
#if defined(_WIN32)
unsigned long mode = 0;
ret = ioctlsocket(sock->channel, FIONBIO, &mode);
#else
const int flags = fcntl(sock->channel, F_GETFL, 0);
if (!(flags & O_NONBLOCK))
{
TRACELOG(LOG_DEBUG, "Socket was already in blocking mode");
return ret;
}

ret = (0 == fcntl(sock->channel, F_SETFL, (flags ^ O_NONBLOCK)));
#endif
return ret;
}


static bool SocketSetNonBlocking(Socket *sock)
{
bool ret = true;
#if defined(_WIN32)
unsigned long mode = 1;
ret = ioctlsocket(sock->channel, FIONBIO, &mode);
#else
const int flags = fcntl(sock->channel, F_GETFL, 0);

if ((flags & O_NONBLOCK))
{
TRACELOG(LOG_DEBUG, "Socket was already in non-blocking mode");
return ret;
}

ret = (0 == fcntl(sock->channel, F_SETFL, (flags | O_NONBLOCK)));
#endif
return ret;
}


static bool SocketSetOptions(SocketConfig *config, Socket *sock)
{
for (int i = 0; i < SOCKET_MAX_SOCK_OPTS; i++)
{
SocketOpt *opt = &config->sockopts[i];

if (opt->id == 0) break;

if (setsockopt(sock->channel, SOL_SOCKET, opt->id, opt->value, opt->valueLen) < 0) return false;
}

return true;
}


static void SocketSetHints(SocketConfig *config, struct addrinfo *hints)
{
if (config == NULL || hints == NULL) return;

memset(hints, 0, sizeof(*hints));


if (IsIPv4Address(config->host))
{
hints->ai_family = AF_INET;
hints->ai_flags |= AI_NUMERICHOST;
}
else
{
if (IsIPv6Address(config->host))
{
hints->ai_family = AF_INET6;
hints->ai_flags |= AI_NUMERICHOST;
}
else hints->ai_family = AF_UNSPEC;
}

if (config->type == SOCKET_UDP) hints->ai_socktype = SOCK_DGRAM;
else hints->ai_socktype = SOCK_STREAM;



if (!(config->type == SOCKET_UDP) || config->server) hints->ai_flags = AI_PASSIVE;
}






bool InitNetworkDevice(void)
{
#if defined(_WIN32)
WORD wVersionRequested;
WSADATA wsaData;

wVersionRequested = MAKEWORD(2, 2);
int err = WSAStartup(wVersionRequested, &wsaData);

if (err != 0)
{
TRACELOG(LOG_WARNING, "WinSock failed to initialise.");
return false;
}
else TRACELOG(LOG_INFO, "WinSock initialised.");

if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
{
TRACELOG(LOG_WARNING, "WinSock failed to initialise.");
WSACleanup();
return false;
}

return true;
#else
return true;
#endif
}


void CloseNetworkDevice(void)
{
#if defined(_WIN32)
WSACleanup();
#endif
}














void ResolveIP(const char *ip, const char *port, int flags, char *host, char *serv)
{

int status; 
struct addrinfo hints; 
struct addrinfo *res; 


memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; 
hints.ai_protocol = 0; 


status = getaddrinfo(ip, 
port, 
&hints, 
&res 
);


if (status != 0) TRACELOG(LOG_WARNING, "Failed to get resolve host %s:%s: %s", ip, port, gai_strerror(errno));
else TRACELOG(LOG_DEBUG, "Resolving... %s::%s", ip, port);


switch (res->ai_family)
{
case AF_INET:
{
status = getnameinfo(&*((struct sockaddr *)res->ai_addr),
sizeof(*((struct sockaddr_in *)res->ai_addr)),
host, NI_MAXHOST, serv, NI_MAXSERV, flags);
} break;
case AF_INET6:
{





} break;
default: break;
}

if (status != 0) TRACELOG(LOG_WARNING, "Failed to resolve ip %s: %s", ip, SocketGetLastErrorString());
else TRACELOG(LOG_DEBUG, "Successfully resolved %s::%s to %s", ip, port, host);


freeaddrinfo(res);
}














int ResolveHost(const char *address, const char *service, int addressType, int flags, AddressInformation *outAddr)
{

int status; 
struct addrinfo hints; 
struct addrinfo *res; 
struct addrinfo *iterator;
assert(((address != NULL || address != 0) || (service != NULL || service != 0)));
assert(((addressType == AF_INET) || (addressType == AF_INET6) || (addressType == AF_UNSPEC)));


memset(&hints, 0, sizeof hints);
hints.ai_family = addressType; 
hints.ai_protocol = 0; 
hints.ai_flags = flags;
assert((hints.ai_addrlen == 0) || (hints.ai_addrlen == 0));
assert((hints.ai_canonname == 0) || (hints.ai_canonname == 0));
assert((hints.ai_addr == 0) || (hints.ai_addr == 0));
assert((hints.ai_next == 0) || (hints.ai_next == 0));


if (address == NULL)
{
if ((hints.ai_flags & AI_PASSIVE) == 0) hints.ai_flags |= AI_PASSIVE;
}

TRACELOG(LOG_INFO, "Resolving host...");


status = getaddrinfo(address, 
service, 
&hints, 
&res 
);


if (status != 0)
{
int error = SocketGetLastError();
SocketSetLastError(0);
TRACELOG(LOG_WARNING, "Failed to get resolve host: %s", SocketErrorCodeToString(error));
return -1;
}
else TRACELOG(LOG_INFO, "Successfully resolved host %s:%s", address, service);


int size = 0;
for (iterator = res; iterator != NULL; iterator = iterator->ai_next) size++;


if (size <= 0)
{
TRACELOG(LOG_WARNING, "Error, no addresses found.");
return -1;
}


if (outAddr == NULL) outAddr = (AddressInformation *)RNET_MALLOC(size * sizeof(AddressInformation));


if (outAddr != NULL)
{
int i;
for (i = 0; i < size; ++i)
{
outAddr[i] = LoadAddress();
if (outAddr[i] == NULL)
{
break;
}
}

outAddr[i] = NULL;
if (i != size) outAddr = NULL;
}
else
{
TRACELOG(LOG_WARNING, "Error, failed to dynamically allocate memory for the address list");
return -1;
}


int i = 0;
for (iterator = res; iterator != NULL; iterator = iterator->ai_next)
{
if (i < size)
{
outAddr[i]->addr.ai_flags = iterator->ai_flags;
outAddr[i]->addr.ai_family = iterator->ai_family;
outAddr[i]->addr.ai_socktype = iterator->ai_socktype;
outAddr[i]->addr.ai_protocol = iterator->ai_protocol;
outAddr[i]->addr.ai_addrlen = iterator->ai_addrlen;
*outAddr[i]->addr.ai_addr = *iterator->ai_addr;
#if NET_DEBUG_ENABLED
TRACELOG(LOG_DEBUG, "GetAddressInformation");
TRACELOG(LOG_DEBUG, "\tFlags: 0x%x", iterator->ai_flags);

TRACELOG(LOG_DEBUG, "Length of this sockaddr: %d", outAddr[i]->addr.ai_addrlen);
TRACELOG(LOG_DEBUG, "Canonical name: %s", iterator->ai_canonname);
#endif
i++;
}
}


freeaddrinfo(res);


return size;
}















bool SocketCreate(SocketConfig *config, SocketResult *result)
{

bool success = true;


if (config == NULL || result == NULL) return (success = false);


if (!SocketSetDefaults(config))
{
TRACELOG(LOG_WARNING, "Configuration Error.");
success = false;
}
else
{

if (CreateSocket(config, result))
{
if (config->nonblocking) SocketSetNonBlocking(result->socket);
else SocketSetBlocking(result->socket);
}
else success = false;
}

return success;
}



bool SocketBind(SocketConfig *config, SocketResult *result)
{
bool success = false;
result->status = RESULT_FAILURE;
struct sockaddr_storage *sock_addr = NULL;


if (!IsSocketValid(result->socket) || !config->server)
{
TRACELOG(LOG_WARNING, Cannot bind to socket marked as \"Client\" in SocketConfig.");
success = false;
}
else
{
if (result->socket->isIPv6) sock_addr = (struct sockaddr_storage *)&result->socket->addripv6->address;
else sock_addr = (struct sockaddr_storage *)&result->socket->addripv4->address;

if (sock_addr != NULL)
{
if (bind(result->socket->channel, (struct sockaddr *)sock_addr, sizeof(*sock_addr)) != SOCKET_ERROR)
{
TRACELOG(LOG_INFO, "Successfully bound socket.");
success = true;
}
else
{
result->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(result->socket->status));
SocketSetLastError(0);
success = false;
}
}
}


if (success)
{
result->status = RESULT_SUCCESS;
result->socket->ready = 0;
result->socket->status = 0;
socklen_t sock_len = sizeof(*sock_addr);

if (getsockname(result->socket->channel, (struct sockaddr *)sock_addr, &sock_len) < 0)
{
TRACELOG(LOG_WARNING, "Couldn't get socket address");
}
else
{
struct sockaddr_in *s = (struct sockaddr_in *)sock_addr;



result->socket->addripv4 = (struct _SocketAddressIPv4 *)RNET_MALLOC(sizeof(*result->socket->addripv4));

if (result->socket->addripv4 != NULL) memset(result->socket->addripv4, 0, sizeof(*result->socket->addripv4));

memcpy(&result->socket->addripv4->address, (struct sockaddr_in *)&s->sin_addr, sizeof(struct sockaddr_in));
}
}
return success;
}


bool SocketListen(SocketConfig *config, SocketResult *result)
{
bool success = false;
result->status = RESULT_FAILURE;


if (!IsSocketValid(result->socket) || !config->server)
{
TRACELOG(LOG_WARNING, "Cannot listen on socket marked as \"Client\" in SocketConfig.");
success = false;
}
else
{

if (!(config->type == SOCKET_UDP))
{
if (listen(result->socket->channel, config->backlog_size) != SOCKET_ERROR)
{
TRACELOG(LOG_INFO, "Started listening on socket...");
success = true;
}
else
{
success = false;
result->socket->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(result->socket->status));
SocketSetLastError(0);
}
}
else
{
TRACELOG(LOG_WARNING, "Cannot listen on socket marked as \"UDP\" (datagram) in SocketConfig.");
success = false;
}
}


if (success)
{
result->status = RESULT_SUCCESS;
result->socket->ready = 0;
result->socket->status = 0;
}

return success;
}


bool SocketConnect(SocketConfig *config, SocketResult *result)
{
bool success = true;
result->status = RESULT_FAILURE;


if (config->server)
{
TRACELOG(LOG_WARNING, "Cannot connect to socket marked as \"Server\" in SocketConfig.");
success = false;
}
else
{
if (IsIPv4Address(config->host))
{
struct sockaddr_in ip4addr;
ip4addr.sin_family = AF_INET;
unsigned long hport;
hport = strtoul(config->port, NULL, 0);
ip4addr.sin_port = htons(hport);




int connect_result = connect(result->socket->channel, (struct sockaddr *)&ip4addr, sizeof(ip4addr));

if (connect_result == SOCKET_ERROR)
{
result->socket->status = SocketGetLastError();
SocketSetLastError(0);

switch (result->socket->status)
{
case WSAEWOULDBLOCK: success = true; break;
default:
{
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(result->socket->status));
success = false;
} break;
}
}
else
{
TRACELOG(LOG_INFO, "Successfully connected to socket.");
success = true;
}
}
else
{
if (IsIPv6Address(config->host))
{
struct sockaddr_in6 ip6addr;
ip6addr.sin6_family = AF_INET6;
unsigned long hport;
hport = strtoul(config->port, NULL, 0);
ip6addr.sin6_port = htons(hport);

int connect_result = connect(result->socket->channel, (struct sockaddr *)&ip6addr, sizeof(ip6addr));

if (connect_result == SOCKET_ERROR)
{
result->socket->status = SocketGetLastError();
SocketSetLastError(0);

switch (result->socket->status)
{
case WSAEWOULDBLOCK: success = true; break;
default:
{
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(result->socket->status));
success = false;
} break;
}
}
else
{
TRACELOG(LOG_INFO, "Successfully connected to socket.");
success = true;
}
}
}
}

if (success)
{
result->status = RESULT_SUCCESS;
result->socket->ready = 0;
result->socket->status = 0;
}

return success;
}




void SocketClose(Socket *sock)
{
if (sock != NULL)
{
if (sock->channel != INVALID_SOCKET) closesocket(sock->channel);
}
}


SocketAddressStorage SocketGetPeerAddress(Socket *sock)
{







return NULL;
}


char *GetSocketAddressHost(SocketAddressStorage storage)
{
assert(storage->address.ss_family == AF_INET || storage->address.ss_family == AF_INET6);
return SocketAddressToString((struct sockaddr_storage *)storage);
}


short GetSocketAddressPort(SocketAddressStorage storage)
{


return 0;
}

















Socket *SocketAccept(Socket *server, SocketConfig *config)
{
if (!server->isServer || server->type == SOCKET_UDP) return NULL;

struct sockaddr_storage sock_addr;
socklen_t sock_alen;
Socket *sock = LoadSocket();
server->ready = 0;
sock_alen = sizeof(sock_addr);
sock->channel = accept(server->channel, (struct sockaddr *)&sock_addr, &sock_alen);

if (sock->channel == INVALID_SOCKET)
{
sock->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(sock->status));
SocketSetLastError(0);
SocketClose(sock);

return NULL;
}

(config->nonblocking) ? SocketSetNonBlocking(sock) : SocketSetBlocking(sock);
sock->isServer = false;
sock->ready = 0;
sock->type = server->type;

switch (sock_addr.ss_family)
{
case AF_INET:
{
struct sockaddr_in *s = ((struct sockaddr_in *)&sock_addr);
sock->addripv4 = (struct _SocketAddressIPv4 *)RNET_MALLOC(sizeof(*sock->addripv4));

if (sock->addripv4 != NULL)
{
memset(sock->addripv4, 0, sizeof(*sock->addripv4));
memcpy(&sock->addripv4->address, (struct sockaddr_in *)&s->sin_addr, sizeof(struct sockaddr_in));
TRACELOG(LOG_INFO, "Server: Got connection from %s::%hu", SocketAddressToString((struct sockaddr_storage *)s), ntohs(sock->addripv4->address.sin_port));
}
} break;
case AF_INET6:
{
struct sockaddr_in6 *s = ((struct sockaddr_in6 *)&sock_addr);
sock->addripv6 = (struct _SocketAddressIPv6 *)RNET_MALLOC(sizeof(*sock->addripv6));

if (sock->addripv6 != NULL)
{
memset(sock->addripv6, 0, sizeof(*sock->addripv6));
memcpy(&sock->addripv6->address, (struct sockaddr_in6 *)&s->sin6_addr, sizeof(struct sockaddr_in6));
TRACELOG(LOG_INFO, "Server: Got connection from %s::%hu", SocketAddressToString((struct sockaddr_storage *)s), ntohs(sock->addripv6->address.sin6_port));
}
} break;
}

return sock;
}


static int ValidChannel(int channel)
{
if ((channel < 0) || (channel >= SOCKET_MAX_UDPCHANNELS))
{
TRACELOG(LOG_WARNING, "Invalid channel");
return 0;
}

return 1;
}


int SocketSetChannel(Socket *socket, int channel, const IPAddress *address)
{
struct UDPChannel *binding;

if (socket == NULL)
{
TRACELOG(LOG_WARNING, "Passed a NULL socket");
return (-1);
}

if (channel == -1)
{
for (channel = 0; channel < SOCKET_MAX_UDPCHANNELS; ++channel)
{
binding = &socket->binding[channel];
if (binding->numbound < SOCKET_MAX_UDPADDRESSES) break;
}
}
else
{
if (!ValidChannel(channel)) return (-1);

binding = &socket->binding[channel];
}

if (binding->numbound == SOCKET_MAX_UDPADDRESSES)
{
TRACELOG(LOG_WARNING, "No room for new addresses");
return (-1);
}

binding->address[binding->numbound++] = *address;

return (channel);
}


void SocketUnsetChannel(Socket *socket, int channel)
{
if ((channel >= 0) && (channel < SOCKET_MAX_UDPCHANNELS)) socket->binding[channel].numbound = 0;
}




SocketDataPacket *AllocPacket(int size)
{
SocketDataPacket *packet = (SocketDataPacket *)RNET_MALLOC(sizeof(*packet));
int error = 1;

if (packet != NULL)
{
packet->maxlen = size;
packet->data = (uint8_t *)RNET_MALLOC(size);
if (packet->data != NULL)
{
error = 0;
}
}

if (error)
{
FreePacket(packet);
packet = NULL;
}

return (packet);
}

int ResizePacket(SocketDataPacket *packet, int newsize)
{
uint8_t *newdata = (uint8_t *)RNET_MALLOC(newsize);

if (newdata != NULL)
{
RNET_FREE(packet->data);
packet->data = newdata;
packet->maxlen = newsize;
}

return (packet->maxlen);
}

void FreePacket(SocketDataPacket *packet)
{
if (packet)
{
RNET_FREE(packet->data);
RNET_FREE(packet);
}
}



SocketDataPacket **AllocPacketList(int howmany, int size)
{
SocketDataPacket **packetV = (SocketDataPacket **)RNET_MALLOC((howmany + 1) * sizeof(*packetV));

if (packetV != NULL)
{
int i;
for (i = 0; i < howmany; ++i)
{
packetV[i] = AllocPacket(size);
if (packetV[i] == NULL)
{
break;
}
}
packetV[i] = NULL;

if (i != howmany)
{
FreePacketList(packetV);
packetV = NULL;
}
}

return (packetV);
}

void FreePacketList(SocketDataPacket **packetV)
{
if (packetV)
{
for (int i = 0; packetV[i]; ++i) FreePacket(packetV[i]);
RNET_FREE(packetV);
}
}


int SocketSend(Socket *sock, const void *datap, int length)
{
int sent = 0;
int left = length;
int status = -1;
int numsent = 0;
const unsigned char *data = (const unsigned char *)datap;


if (sock->isServer)
{
TRACELOG(LOG_WARNING, "Cannot send information on a server socket");
return -1;
}


switch (sock->type)
{
case SOCKET_TCP:
{
SocketSetLastError(0);
do
{
length = send(sock->channel, (const char *)data, left, 0);
if (length > 0)
{
sent += length;
left -= length;
data += length;
}
} while ((left > 0) && 
((length > 0) || 
(SocketGetLastError() == WSAEINTR)) 
);

if (length == SOCKET_ERROR)
{
sock->status = SocketGetLastError();
TRACELOG(LOG_DEBUG, "Socket Error: %s", SocketErrorCodeToString(sock->status));
SocketSetLastError(0);
}
else TRACELOG(LOG_DEBUG, "Successfully sent \"%s\" (%d bytes)", datap, sent);

return sent;
} break;
case SOCKET_UDP:
{
SocketSetLastError(0);

if (sock->isIPv6) status = sendto(sock->channel, (const char *)data, left, 0, (struct sockaddr *)&sock->addripv6->address, sizeof(sock->addripv6->address));
else status = sendto(sock->channel, (const char *)data, left, 0, (struct sockaddr *)&sock->addripv4->address, sizeof(sock->addripv4->address));

if (sent >= 0)
{
sock->status = 0;
++numsent;
TRACELOG(LOG_DEBUG, "Successfully sent \"%s\" (%d bytes)", datap, status);
}
else
{
sock->status = SocketGetLastError();
TRACELOG(LOG_DEBUG, "Socket Error: %s", SocketGetLastErrorString(sock->status));
SocketSetLastError(0);
return 0;
}

return numsent;
} break;
default: break;
}

return -1;
}






int SocketReceive(Socket *sock, void *data, int maxlen)
{
int len = 0;
int numrecv = 0;
int status = 0;
socklen_t sock_len;
struct sockaddr_storage sock_addr;



if (sock->isServer && (sock->type == SOCKET_TCP))
{
sock->status = SocketGetLastError();
TRACELOG(LOG_DEBUG, "Socket Error: %s", "Server sockets cannot be used to receive data");
SocketSetLastError(0);
return 0;
}


switch (sock->type)
{
case SOCKET_TCP:
{
SocketSetLastError(0);
do
{
len = recv(sock->channel, (char *)data, maxlen, 0);
} while (SocketGetLastError() == WSAEINTR);

if (len > 0)
{

if (sock->type == SOCKET_UDP)
{

}

((unsigned char *)data)[len] = '\0'; 
TRACELOG(LOG_DEBUG, "Received \"%s\" (%d bytes)", data, len);
}

sock->ready = 0;
return len;
} break;
case SOCKET_UDP:
{
SocketSetLastError(0);
sock_len = sizeof(sock_addr);
status = recvfrom(sock->channel, 
data, 
maxlen, 
0, 
(struct sockaddr *)&sock_addr, 
&sock_len 
);

if (status >= 0) ++numrecv;
else
{
sock->status = SocketGetLastError();

switch (sock->status)
{
case WSAEWOULDBLOCK: break;
default: TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(sock->status)); break;
}

SocketSetLastError(0);
return 0;
}

sock->ready = 0;
return numrecv;
} break;
default: break;
}

return -1;
}


bool IsSocketReady(Socket *sock)
{
return (sock != NULL) && (sock->ready);
}


bool IsSocketConnected(Socket *sock)
{
#if defined(_WIN32)
FD_SET writefds;
FD_ZERO(&writefds);
FD_SET(sock->channel, &writefds);
struct timeval timeout;
timeout.tv_sec = 1;
timeout.tv_usec = 1000000000UL;
int total = select(0, NULL, &writefds, NULL, &timeout);

if (total == -1)
{ 
sock->status = SocketGetLastError();
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(sock->status));
SocketSetLastError(0);
}
else if (total == 0) return false; 
else if (FD_ISSET(sock->channel, &writefds)) return true;

return false;
#else
return true;
#endif
}


SocketResult *LoadSocketResult(void)
{
struct SocketResult *res = (struct SocketResult *)RNET_MALLOC(sizeof(*res));

if (res != NULL)
{
memset(res, 0, sizeof(*res));
if ((res->socket = LoadSocket()) == NULL)
{
RNET_FREE(res);
res = NULL;
}
}

return res;
}


void UnloadSocketResult(SocketResult **result)
{
if (*result != NULL)
{
if ((*result)->socket != NULL) UnloadSocket(&((*result)->socket));

RNET_FREE(*result);
*result = NULL;
}
}


Socket *LoadSocket(void)
{
struct Socket *sock;
sock = (Socket *)RNET_MALLOC(sizeof(*sock));

if (sock != NULL) memset(sock, 0, sizeof(*sock));
else
{
TRACELOG(LOG_WARNING, "Ran out of memory attempting to allocate a socket");
SocketClose(sock);
RNET_FREE(sock);
sock = NULL;
}

return sock;
}


void UnloadSocket(Socket **sock)
{
if (*sock != NULL)
{
RNET_FREE(*sock);
*sock = NULL;
}
}


SocketSet *LoadSocketSet(int max)
{
struct SocketSet *set = (struct SocketSet *)RNET_MALLOC(sizeof(*set));

if (set != NULL)
{
set->numsockets = 0;
set->maxsockets = max;
set->sockets = (struct Socket **)RNET_MALLOC(max * sizeof(*set->sockets));
if (set->sockets != NULL)
{
for (int i = 0; i < max; ++i) set->sockets[i] = NULL;
}
else
{
RNET_FREE(set);
set = NULL;
}
}

return (set);
}


void UnloadSocketSet(SocketSet *set)
{
if (set)
{
RNET_FREE(set->sockets);
RNET_FREE(set);
}
}


int AddSocket(SocketSet *set, Socket *sock)
{
if (sock != NULL)
{
if (set->numsockets == set->maxsockets)
{
TRACELOG(LOG_DEBUG, "Socket Error: %s", "SocketSet is full");
SocketSetLastError(0);
return (-1);
}
set->sockets[set->numsockets++] = (struct Socket *)sock;
}
else
{
TRACELOG(LOG_DEBUG, "Socket Error: %s", "Socket was null");
SocketSetLastError(0);
return (-1);
}

return (set->numsockets);
}


int RemoveSocket(SocketSet *set, Socket *sock)
{
if (sock != NULL)
{
int i = 0;
for (i = 0; i < set->numsockets; ++i)
{
if (set->sockets[i] == (struct Socket *)sock) break;
}

if (i == set->numsockets)
{
TRACELOG(LOG_DEBUG, "Socket Error: %s", "Socket not found");
SocketSetLastError(0);
return (-1);
}

--set->numsockets;
for (; i < set->numsockets; ++i) set->sockets[i] = set->sockets[i + 1];
}

return (set->numsockets);
}


int CheckSockets(SocketSet *set, unsigned int timeout)
{
int i;
SOCKET maxfd;
int retval;
struct timeval tv;
fd_set mask;


maxfd = 0;
for (i = set->numsockets - 1; i >= 0; --i)
{
if (set->sockets[i]->channel > maxfd)
{
maxfd = set->sockets[i]->channel;
}
}


do
{
SocketSetLastError(0);


FD_ZERO(&mask);
for (i = set->numsockets - 1; i >= 0; --i)
{
FD_SET(set->sockets[i]->channel, &mask);
} 

tv.tv_sec = timeout / 1000;
tv.tv_usec = (timeout % 1000) * 1000;


retval = select(maxfd + 1, &mask, NULL, NULL, &tv);
} while (SocketGetLastError() == WSAEINTR);


if (retval > 0)
{
for (i = set->numsockets - 1; i >= 0; --i)
{
if (FD_ISSET(set->sockets[i]->channel, &mask)) set->sockets[i]->ready = 1;
}
}

return retval;
}


AddressInformation LoadAddress(void)
{
AddressInformation addressInfo = NULL;
addressInfo = (AddressInformation)RNET_CALLOC(1, sizeof(*addressInfo));

if (addressInfo != NULL)
{
addressInfo->addr.ai_addr = (struct sockaddr *)RNET_CALLOC(1, sizeof(struct sockaddr));
if (addressInfo->addr.ai_addr == NULL) TRACELOG(LOG_WARNING, "Failed to allocate memory for \"struct sockaddr\"");
}
else TRACELOG(LOG_WARNING, "Failed to allocate memory for \"struct AddressInformation\"");

return addressInfo;
}


void UnloadAddress(AddressInformation *addressInfo)
{
if (*addressInfo != NULL)
{
if ((*addressInfo)->addr.ai_addr != NULL)
{
RNET_FREE((*addressInfo)->addr.ai_addr);
(*addressInfo)->addr.ai_addr = NULL;
}

RNET_FREE(*addressInfo);
*addressInfo = NULL;
}
}


AddressInformation *LoadAddressList(int size)
{
AddressInformation *addr;
addr = (AddressInformation *)RNET_MALLOC(size * sizeof(AddressInformation));
return addr;
}


int GetAddressFamily(AddressInformation address)
{
return address->addr.ai_family;
}


int GetAddressSocketType(AddressInformation address)
{
return address->addr.ai_socktype;
}


int GetAddressProtocol(AddressInformation address)
{
return address->addr.ai_protocol;
}


char *GetAddressCanonName(AddressInformation address)
{
return address->addr.ai_canonname;
}


char *GetAddressHostAndPort(AddressInformation address, char *outhost, int *outport)
{

char *result = NULL;
struct sockaddr_storage *storage = (struct sockaddr_storage *)address->addr.ai_addr;

switch (storage->ss_family)
{
case AF_INET:
{
struct sockaddr_in *s = ((struct sockaddr_in *)address->addr.ai_addr);

*outport = ntohs(s->sin_port);
} break;
case AF_INET6:
{
struct sockaddr_in6 *s = ((struct sockaddr_in6 *)address->addr.ai_addr);

*outport = ntohs(s->sin6_port);
} break;
default: break;
}

if (result == NULL)
{
TRACELOG(LOG_WARNING, "Socket Error: %s", SocketErrorCodeToString(SocketGetLastError()));
SocketSetLastError(0);
}
else
{
strcpy(outhost, result);
}
return result;
}


void PacketSend(Packet *packet)
{
TRACELOG(LOG_INFO, "Sending packet (%s) with size %d\n", packet->data, packet->size);
}


void PacketReceive(Packet *packet)
{
TRACELOG(LOG_INFO, "Receiving packet (%s) with size %d\n", packet->data, packet->size);
}


void PacketWrite16(Packet *packet, uint16_t value)
{
TRACELOG(LOG_INFO, "Original: 0x%04" PRIX16 " - %" PRIu16 "\n", value, value);
uint8_t *data = packet->data + packet->offs;
*data++ = (uint8_t)(value >> 8);
*data++ = (uint8_t)(value);
packet->size += sizeof(uint16_t);
packet->offs += sizeof(uint16_t);
TRACELOG(LOG_INFO, "Network: 0x%04" PRIX16 " - %" PRIu16 "\n", (uint16_t) *data, (uint16_t) *data);
}


void PacketWrite32(Packet *packet, uint32_t value)
{
TRACELOG(LOG_INFO, "Original: 0x%08" PRIX32 " - %" PRIu32 "\n", value, value);
uint8_t *data = packet->data + packet->offs;
*data++ = (uint8_t)(value >> 24);
*data++ = (uint8_t)(value >> 16);
*data++ = (uint8_t)(value >> 8);
*data++ = (uint8_t)(value);
packet->size += sizeof(uint32_t);
packet->offs += sizeof(uint32_t);

TRACELOG(LOG_INFO, "Network: 0x%08" PRIX32 " - %" PRIu32 "\n",
(uint32_t)(((intptr_t) packet->data) - packet->offs),
(uint32_t)(((intptr_t) packet->data) - packet->offs));
}


void PacketWrite64(Packet *packet, uint64_t value)
{
TRACELOG(LOG_INFO, "Original: 0x%016" PRIX64 " - %" PRIu64 "\n", value, value);

uint8_t *data = packet->data + packet->offs;
*data++ = (uint8_t)(value >> 56);
*data++ = (uint8_t)(value >> 48);
*data++ = (uint8_t)(value >> 40);
*data++ = (uint8_t)(value >> 32);
*data++ = (uint8_t)(value >> 24);
*data++ = (uint8_t)(value >> 16);
*data++ = (uint8_t)(value >> 8);
*data++ = (uint8_t)(value);
packet->size += sizeof(uint64_t);
packet->offs += sizeof(uint64_t);

TRACELOG(LOG_INFO, "Network: 0x%016" PRIX64 " - %" PRIu64 "\n", (uint64_t)(packet->data - packet->offs), (uint64_t)(packet->data - packet->offs));
}


uint16_t PacketRead16(Packet *packet)
{
uint8_t *data = packet->data + packet->offs;
packet->size += sizeof(uint16_t);
packet->offs += sizeof(uint16_t);
uint16_t value = ((uint16_t) data[0] << 8) | data[1];
TRACELOG(LOG_INFO, "Original: 0x%04" PRIX16 " - %" PRIu16 "\n", value, value);

return value;
}


uint32_t PacketRead32(Packet *packet)
{
uint8_t *data = packet->data + packet->offs;
packet->size += sizeof(uint32_t);
packet->offs += sizeof(uint32_t);
uint32_t value = ((uint32_t) data[0] << 24) | ((uint32_t) data[1] << 16) | ((uint32_t) data[2] << 8) | data[3];
TRACELOG(LOG_INFO, "Original: 0x%08" PRIX32 " - %" PRIu32 "\n", value, value);

return value;
}


uint64_t PacketRead64(Packet *packet)
{
uint8_t *data = packet->data + packet->offs;
packet->size += sizeof(uint64_t);
packet->offs += sizeof(uint64_t);
uint64_t value = ((uint64_t) data[0] << 56) | ((uint64_t) data[1] << 48) | ((uint64_t) data[2] << 40) | ((uint64_t) data[3] << 32) | ((uint64_t) data[4] << 24) | ((uint64_t) data[5] << 16) | ((uint64_t) data[6] << 8) | data[7];
TRACELOG(LOG_INFO, "Original: 0x%016" PRIX64 " - %" PRIu64 "\n", value, value);

return value;
}

#endif