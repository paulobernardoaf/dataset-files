























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_block.h>

#include <vlc_network.h>
#include <vlc_sout.h>
#include <vlc_fs.h>
#include "rtp.h"

#include <assert.h>

#if !defined(SOL_IP)
#define SOL_IP IPPROTO_IP
#endif















struct rtcp_sender_t
{
size_t length; 
uint8_t payload[28 + 8 + (2 * 257) + 8];
int handle; 

uint32_t packets; 
uint32_t bytes; 
unsigned counter; 
};


rtcp_sender_t *OpenRTCP (vlc_object_t *obj, int rtp_fd, int proto,
bool mux)
{
rtcp_sender_t *rtcp;
uint8_t *ptr;
int fd;
char src[NI_MAXNUMERICHOST];
int sport;

if (net_GetSockAddress (rtp_fd, src, &sport))
return NULL;

if (mux)
{

#if !defined(_WIN32)
fd = vlc_dup (rtp_fd);
#else
WSAPROTOCOL_INFO info;
WSADuplicateSocket (rtp_fd, GetCurrentProcessId (), &info);
fd = WSASocket (info.iAddressFamily, info.iSocketType, info.iProtocol,
&info, 0, 0);
#endif
}
else
{

char dst[NI_MAXNUMERICHOST];
int dport;

if (net_GetPeerAddress (rtp_fd, dst, &dport))
return NULL;

sport++;
dport++;

fd = net_OpenDgram (obj, src, sport, dst, dport, proto);
if (fd != -1)
{

int ttl;
socklen_t len = sizeof (ttl);

if (!getsockopt (rtp_fd, SOL_IP, IP_MULTICAST_TTL, &ttl, &len))
setsockopt (fd, SOL_IP, IP_MULTICAST_TTL, &ttl, len);


setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &(int){ 0 }, sizeof (int));
}
}

if (fd == -1)
return NULL;

rtcp = malloc (sizeof (*rtcp));
if (rtcp == NULL)
{
net_Close (fd);
return NULL;
}

rtcp->handle = fd;
rtcp->bytes = rtcp->packets = rtcp->counter = 0;

ptr = (uint8_t *)strchr (src, '%');
if (ptr != NULL)
*ptr = '\0'; 

ptr = rtcp->payload;


ptr[0] = 2 << 6; 
ptr[1] = 200; 
SetWBE (ptr + 2, 6); 
memset (ptr + 4, 0, 4); 
SetQWBE (ptr + 8, NTPtime64 ());
memset (ptr + 16, 0, 12); 
ptr += 28;


uint8_t *sdes = ptr;
ptr[0] = (2 << 6) | 1; 
ptr[1] = 202; 
uint8_t *lenptr = ptr + 2;
memset (ptr + 4, 0, 4); 
ptr += 8;

ptr[0] = 1; 
assert (NI_MAXNUMERICHOST <= 256);
ptr[1] = strlen (src);
memcpy (ptr + 2, src, ptr[1]);
ptr += ptr[1] + 2;

static const char tool[] = PACKAGE_STRING;
ptr[0] = 6; 
ptr[1] = (sizeof (tool) > 256) ? 255 : (sizeof (tool) - 1);
memcpy (ptr + 2, tool, ptr[1]);
ptr += ptr[1] + 2;

while ((ptr - sdes) & 3) 
*ptr++ = 0;
SetWBE (lenptr, (ptr - sdes - 1) >> 2);

rtcp->length = ptr - rtcp->payload;
return rtcp;
}


void CloseRTCP (rtcp_sender_t *rtcp)
{
if (rtcp == NULL)
return;

uint8_t *ptr = rtcp->payload;
uint64_t now64 = NTPtime64 ();
SetQWBE (ptr + 8, now64); 


ptr += rtcp->length;
ptr[0] = (2 << 6) | 1; 
ptr[1] = 203; 
SetWBE (ptr + 2, 1);
memcpy (ptr + 4, rtcp->payload + 4, 4); 
rtcp->length += 8;



send (rtcp->handle, rtcp->payload, rtcp->length, 0);
net_Close (rtcp->handle);
free (rtcp);
}


void SendRTCP (rtcp_sender_t *restrict rtcp, const block_t *rtp)
{
if ((rtcp == NULL) 
|| (rtp->i_buffer < 12)) 
return;


rtcp->packets++;
rtcp->bytes += rtp->i_buffer;
rtcp->counter += rtp->i_buffer;


if ((rtcp->counter / 80) < rtcp->length)
return;

uint8_t *ptr = rtcp->payload;
uint32_t last = GetDWBE (ptr + 8); 
uint64_t now64 = NTPtime64 ();
if ((now64 >> 32) < (last + 5))
return; 

memcpy (ptr + 4, rtp->p_buffer + 8, 4); 
SetQWBE (ptr + 8, now64);
memcpy (ptr + 16, rtp->p_buffer + 4, 4); 
SetDWBE (ptr + 20, rtcp->packets);
SetDWBE (ptr + 24, rtcp->bytes);
memcpy (ptr + 28 + 4, rtp->p_buffer + 8, 4); 

if (send (rtcp->handle, ptr, rtcp->length, 0) == (ssize_t)rtcp->length)
rtcp->counter = 0;
}
